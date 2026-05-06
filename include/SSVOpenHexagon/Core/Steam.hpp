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
        QueryComplete = 0,    //!< query finished, results delivered
        ItemSubscribed,       //!< user subscribed; download is in flight
        ItemUnsubscribed,     //!< user unsubscribed; folder may still exist
        ItemInstalled,        //!< pack is on disk at `installFolder`
        DownloadProgress,     //!< periodic progress update
    };

    Kind                         kind{Kind::QueryComplete};
    sf::base::U64                publishedFileId{};   //!< for ItemSubscribed/Unsubscribed/Installed/DownloadProgress
    sf::base::U64                bytesDone{};         //!< for DownloadProgress
    sf::base::U64                bytesTotal{};        //!< for DownloadProgress
    sf::base::String             installFolder;       //!< for ItemInstalled
    sf::base::Vector<WorkshopItem> queryResults;       //!< for QueryComplete
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

    bool request_encrypted_app_ticket();

    [[nodiscard]] bool got_encrypted_app_ticket_response() const noexcept;

    [[nodiscard]] bool got_encrypted_app_ticket() const noexcept;

    [[nodiscard]] sf::base::Optional<sf::base::U64> get_ticket_steam_id() const noexcept;

    // Workshop API additions (see `WorkshopItem` / `WorkshopEvent` above).
    // Each call is a no-op if Steam is not initialized.

    // Kick off an async query. Results arrive via `poll_workshop_event()` as
    // a `QueryComplete` event. Page is 1-based per Steam UGC convention.
    void query_workshop_items(WorkshopQueryMode mode, int page);

    // Subscribe / unsubscribe — `ItemInstalled` will fire after Steam
    // finishes downloading.
    void subscribe_workshop_item  (sf::base::U64 publishedFileId);
    void unsubscribe_workshop_item(sf::base::U64 publishedFileId);

    // Drains one event from the queue. Caller polls until empty.
    [[nodiscard]] sf::base::Optional<WorkshopEvent> poll_workshop_event();
};

} // namespace hg::Steam
