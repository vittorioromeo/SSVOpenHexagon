// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <string_view>

namespace hg::Steam
{

////////////////////////////////////////////////////////////////////////////////
// Workshop API additions for the new in-game browser. See
// `docs/UI_REWRITE_DESIGN.md` §5.1 for design rationale.

// One workshop item as returned by `query_workshop_items`. POD; copies are
// fine.
struct WorkshopItem
{
    sf::base::U64    publishedFileId{};
    sf::base::String title;
    sf::base::String author;
    sf::base::String description;
    sf::base::U64    sizeBytes{};
    bool             isSubscribed{};
    bool             isInstalled{};

    // Workshop "child" items declared as required dependencies on the
    // store page. Populated by `query_workshop_items` (we ask Steam to
    // return children with each query). Subscribing to this item should
    // also subscribe to anything in here that isn't already subscribed.
    sf::base::Vector<sf::base::U64> dependencies;

    // Primary preview image URL (the "main" workshop screenshot). Pulled
    // from `ISteamUGC::GetQueryUGCPreviewURL` during query results. The UI
    // may display this as a thumbnail / carousel slide. Empty when Steam
    // didn't return a URL (e.g. the item has no preview).
    sf::base::String previewUrl;
};

// Sort/filter mode passed to `query_workshop_items`.
enum class WorkshopQueryMode : sf::base::U8
{
    MostPopular = 0,
    Newest,
    Trending,
    All,
};

// Async event surfaced by `poll_workshop_event()`. Tagged union of small
// structs; the `kind` discriminator picks the active member.
struct WorkshopEvent
{
    enum class Kind : sf::base::U8
    {
        QueryComplete = 0, //!< paged "browse" query finished
        DetailsComplete,   //!< on-demand details query (e.g. dep names)
        ItemSubscribed,    //!< user subscribed; download is in flight
        ItemUnsubscribed,  //!< user unsubscribed; folder may still exist
        ItemInstalled,     //!< pack is on disk at `installFolder`
        DownloadProgress,  //!< periodic progress update
        PreviewDownloaded, //!< HTTP fetch of `previewUrl` finished; bytes in `previewBytes`
    };

    Kind          kind{Kind::QueryComplete};
    sf::base::U64 publishedFileId{}; //!< for ItemSubscribed/Unsubscribed/Installed/DownloadProgress/PreviewDownloaded
    sf::base::U64 bytesDone{};       //!< for DownloadProgress
    sf::base::U64 bytesTotal{};      //!< for DownloadProgress
    sf::base::String               installFolder;   //!< for ItemInstalled
    sf::base::Vector<WorkshopItem> queryResults;    //!< for QueryComplete
    sf::base::U32                  totalMatching{}; //!< for QueryComplete: ISteamUGC's `m_unTotalMatchingResults`
    sf::base::Vector<sf::base::U8> previewBytes; //!< for PreviewDownloaded: raw image bytes fetched from `previewUrl`
};

class steam_manager
{
private:
    class steam_manager_impl;

    sf::base::UniquePtr<steam_manager_impl> _impl;

    [[nodiscard]] const steam_manager_impl& impl() const noexcept;
    [[nodiscard]] steam_manager_impl&       impl() noexcept;

public:
    explicit steam_manager();
    ~steam_manager();

    steam_manager(const steam_manager&)            = delete;
    steam_manager& operator=(const steam_manager&) = delete;

    steam_manager(steam_manager&&)            = delete;
    steam_manager& operator=(steam_manager&&) = delete;

    [[nodiscard]] bool is_initialized() const noexcept;

    bool request_stats_and_achievements();

    bool run_callbacks();

    bool store_stats();
    bool unlock_achievement(std::string_view name);

    bool set_rich_presence_in_menu();
    bool set_rich_presence_in_game(std::string_view level_name_format,
                                   std::string_view difficulty_mult_format,
                                   std::string_view time_format);

    bool               set_and_store_stat(std::string_view name, int data);
    [[nodiscard]] bool get_achievement(bool* out, std::string_view name);
    [[nodiscard]] bool get_stat(int* out, std::string_view name);

    bool update_hardcoded_achievements();

    void for_workshop_pack_folders(sf::base::FixedFunction<void(const sf::base::String&), 64> f) const;

    // Iterates every workshop item that was subscribed *and* installed
    // when the game booted, exposing both the published-file-id and
    // the on-disk folder. Lets the host (`MenuGame`) populate the
    // file-id <-> pack-id bridge for items that already existed at
    // startup -- without it, deleting one of those packs at runtime
    // can't be matched back to an `HGAssets` pack id (since
    // `EK::ItemInstalled` only fires for *new* downloads in the
    // current session).
    void for_workshop_subscribed_items(sf::base::FixedFunction<void(sf::base::U64, const sf::base::String&), 64> f) const;

    bool request_encrypted_app_ticket();

    [[nodiscard]] bool got_encrypted_app_ticket_response() const noexcept;

    [[nodiscard]] bool got_encrypted_app_ticket() const noexcept;

    [[nodiscard]] sf::base::Optional<sf::base::U64> get_ticket_steam_id() const noexcept;

    // Workshop API additions (see `WorkshopItem` / `WorkshopEvent` above).
    // Each call is a no-op if Steam is not initialized.

    // Kick off an async query. Results arrive via `poll_workshop_event()` as
    // a `QueryComplete` event. Page is 1-based per Steam UGC convention.
    void query_workshop_items(WorkshopQueryMode mode, int page);

    // Kick off an async query for specific item IDs (used to resolve
    // dependency titles). Results arrive via `poll_workshop_event()` as a
    // `QueryComplete` event, same as `query_workshop_items`.
    void query_workshop_details(const sf::base::Vector<sf::base::U64>& ids);

    // Subscribe / unsubscribe -- `ItemInstalled` will fire after Steam
    // finishes downloading.
    void subscribe_workshop_item(sf::base::U64 publishedFileId);
    void unsubscribe_workshop_item(sf::base::U64 publishedFileId);

    // Returns true if Steam currently reports the item as subscribed.
    // Cheap synchronous lookup against the local item-state cache; safe
    // to use as a guard before calling `subscribe_workshop_item`.
    [[nodiscard]] bool is_workshop_item_subscribed(sf::base::U64 publishedFileId) const noexcept;

    // Drains one event from the queue. Caller polls until empty.
    [[nodiscard]] sf::base::Optional<WorkshopEvent> poll_workshop_event();

    // Kick off an async HTTP fetch of a workshop item's preview image.
    // Caller passes the `previewUrl` Steam returned from `query_workshop_items`.
    // On success a `PreviewDownloaded` event arrives via `poll_workshop_event()`
    // with `publishedFileId` and `previewBytes` (the raw encoded image).
    // No-op if Steam is uninitialized, the URL is empty, or there's already
    // a pending request for the same `publishedFileId`.
    void request_workshop_preview(sf::base::U64 publishedFileId, const sf::base::String& url);

    // Polls in-flight HTTP requests started by `request_workshop_preview`
    // and emits `PreviewDownloaded` events for completed ones. Cheap when
    // nothing is pending -- call once per frame from the host's event pump.
    void pump_workshop_http();
};

} // namespace hg::Steam
