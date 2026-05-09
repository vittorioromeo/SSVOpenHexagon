// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp" // IWYU pragma: keep
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Io.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"

#include <stdint.h> // Steam API needs this.

#ifndef SSVOH_ANDROID
    #include "steam/isteamfriends.h"
    #include "steam/isteamhttp.h"
    #include "steam/isteamremotestorage.h"
    #include "steam/isteamugc.h"
    #include "steam/isteamuser.h"
    #include "steam/isteamuserstats.h"
    #include "steam/isteamutils.h"
    #include "steam/steam_api.h"
    #include "steam/steam_api_common.h"
    #include "steam/steam_api_flat.h"
    #include "steam/steamclientpublic.h"
    #include "steam/steamencryptedappticket.h"
    #include "steam/steamhttpenums.h"
    #include "steam/steamtypes.h"
#endif

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <string_view>

#include <cstring>

#ifndef SSVOH_ANDROID

namespace hg::Steam
{

[[nodiscard]] static sf::base::Optional<CSteamID> get_user_steam_id()
{
    // Using C API here because C++ one doesn't work with MinGW.

    ISteamUser* steam_user = SteamAPI_SteamUser_v023();

    if (!SteamAPI_ISteamUser_BLoggedOn(steam_user))
    {
        hg::lo("Steam") << "Attempted to retrieve Steam ID when not logged in\n";

        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(CSteamID{SteamAPI_ISteamUser_GetSteamID(steam_user)});
}

[[nodiscard]] static bool initialize_steamworks()
{
    hg::lo("Steam") << "Initializing Steam API\n";

    if (SteamAPI_Init())
    {
        hg::lo("Steam") << "Steam API successfully initialized\n";

        if (const sf::base::Optional<CSteamID> user_steam_id = get_user_steam_id(); user_steam_id.hasValue())
        {
            hg::lo("Steam") << "User Steam ID: '" << user_steam_id->ConvertToUint64() << "'\n";
        }
        else
        {
            hg::lo("Steam") << "Could not retrieve user Steam ID\n";
        }

        return true;
    }

    hg::lo("Steam") << "Failed to initialize Steam API\n";
    return false;
}

static void shutdown_steamworks()
{
    hg::lo("Steam") << "Shutting down Steam API\n";
    SteamAPI_Shutdown();
    hg::lo("Steam") << "Shut down Steam API\n";
}

class steam_manager::steam_manager_impl
{
private:
    bool _initialized;
    bool _got_stats;
    bool _got_ticket_response;
    bool _got_ticket;

    sf::base::Optional<CSteamID> _ticket_steam_id;

    ankerl::unordered_dense::set<sf::base::String> _unlocked_achievements;
    ankerl::unordered_dense::set<sf::base::String> _workshop_pack_folders;

    // Parallel record to `_workshop_pack_folders`: maps each currently
    // subscribed-and-installed workshop item's `publishedFileId` to its
    // on-disk folder. Built during `load_workshop_data` alongside
    // `_workshop_pack_folders`. Lets callers (notably `MenuGame`) recover
    // the file-id -> pack-id mapping for items that were already
    // installed at boot, since `EK::ItemInstalled` only fires for
    // *new* downloads.
    ankerl::unordered_dense::map<sf::base::U64, sf::base::String> _workshop_file_id_to_folder;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Winvalid-offsetof"
    #if defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
    #endif
    STEAM_CALLBACK(steam_manager_impl, on_user_stats_received, UserStatsReceived_t);
    STEAM_CALLBACK(steam_manager_impl, on_user_stats_stored, UserStatsStored_t);
    STEAM_CALLBACK(steam_manager_impl, on_user_achievement_stored, UserAchievementStored_t);
    #if defined(__clang__)
        #pragma GCC diagnostic pop
    #endif
    #pragma GCC diagnostic pop

    bool update_hardcoded_achievement_cube_master();
    bool update_hardcoded_achievement_hypercube_master();
    bool update_hardcoded_achievement_cube_god();
    bool update_hardcoded_achievement_hypercube_god();

    void load_workshop_data();

    void on_encrypted_app_ticket_response(EncryptedAppTicketResponse_t* data, bool io_failure);

    CCallResult<steam_manager_impl, EncryptedAppTicketResponse_t> _encrypted_app_ticket_response_call_result;

public:
    steam_manager_impl();
    ~steam_manager_impl();

    steam_manager_impl(const steam_manager_impl&)            = delete;
    steam_manager_impl& operator=(const steam_manager_impl&) = delete;

    steam_manager_impl(steam_manager_impl&&)            = delete;
    steam_manager_impl& operator=(steam_manager_impl&&) = delete;

    [[nodiscard]] bool is_initialized() const noexcept;

    bool request_stats_and_achievements();

    bool run_callbacks();

    bool store_stats();
    bool unlock_achievement(std::string_view name);

    bool set_rich_presence_in_menu();
    bool set_rich_presence_in_game(std::string_view level_name_format,
                                   std::string_view difficulty_mult_format,
                                   std::string_view time_format);

    bool                                   set_and_store_stat(std::string_view name, int data);
    [[nodiscard]] bool                     get_achievement(bool* out, std::string_view name);
    [[nodiscard]] bool                     get_stat(int* out, std::string_view name);
    [[nodiscard]] sf::base::Optional<bool> is_achievement_unlocked(const char* name);

    bool update_hardcoded_achievements();

    void for_workshop_pack_folders(sf::base::FixedFunction<void(const sf::base::String&), 64> f) const;

    void for_workshop_subscribed_items(sf::base::FixedFunction<void(sf::base::U64, const sf::base::String&), 64> f) const;

    bool request_encrypted_app_ticket();

    [[nodiscard]] bool got_encrypted_app_ticket_response() const noexcept;

    [[nodiscard]] bool got_encrypted_app_ticket() const noexcept;

    [[nodiscard]] sf::base::Optional<sf::base::U64> get_ticket_steam_id() const noexcept;

    // Workshop API additions (see `Steam.hpp`).
    void               query_workshop_items(WorkshopQueryMode mode, int page);
    void               query_workshop_details(const sf::base::Vector<sf::base::U64>& ids);
    void               subscribe_workshop_item(sf::base::U64 publishedFileId);
    void               unsubscribe_workshop_item(sf::base::U64 publishedFileId);
    [[nodiscard]] bool is_workshop_item_subscribed(sf::base::U64 publishedFileId) const noexcept;
    [[nodiscard]] sf::base::Optional<WorkshopEvent> poll_workshop_event();

    // Async HTTP preview fetches. See `Steam.hpp` for semantics.
    void request_workshop_preview(sf::base::U64 publishedFileId, const sf::base::String& url);
    void pump_workshop_http();

private:
    sf::base::Vector<WorkshopEvent>                           _workshop_events;
    UGCQueryHandle_t                                          _pending_query{k_UGCQueryHandleInvalid};
    bool                                                      _pending_query_is_details{false};
    CCallResult<steam_manager_impl, SteamUGCQueryCompleted_t> _query_call_result;

    void on_query_completed(SteamUGCQueryCompleted_t* data, bool io_failure);

    // In-flight preview-image fetches. Polled by `pump_workshop_http()`
    // each frame using `SteamUtils()->IsAPICallCompleted` so we don't
    // need a CCallResult per request -- simpler than juggling N call-
    // result instances when the user browses through several items.
    struct PendingPreview
    {
        sf::base::U64     publishedFileId{};
        HTTPRequestHandle httpHandle{INVALID_HTTPREQUEST_HANDLE};
        SteamAPICall_t    apiCall{k_uAPICallInvalid};
    };
    sf::base::Vector<PendingPreview> _pending_previews;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Winvalid-offsetof"
    #if defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
    #endif
    STEAM_CALLBACK(steam_manager_impl, on_item_installed, ItemInstalled_t);
    STEAM_CALLBACK(steam_manager_impl, on_item_subscribed, RemoteStoragePublishedFileSubscribed_t);
    STEAM_CALLBACK(steam_manager_impl, on_item_unsubscribed, RemoteStoragePublishedFileUnsubscribed_t);
    STEAM_CALLBACK(steam_manager_impl, on_download_item_result, DownloadItemResult_t);
    #if defined(__clang__)
        #pragma GCC diagnostic pop
    #endif
    #pragma GCC diagnostic pop
};

void steam_manager::steam_manager_impl::on_user_stats_received(UserStatsReceived_t* data)
{
    (void)data;

    hg::lo("Steam") << "Received user stats (rc: " << data->m_eResult << ")\n";

    _got_stats = true;
}

void steam_manager::steam_manager_impl::on_user_stats_stored(UserStatsStored_t* data)
{
    (void)data;

    hg::lo("Steam") << "Stored user stats\n";
}

void steam_manager::steam_manager_impl::on_user_achievement_stored(UserAchievementStored_t* data)
{
    (void)data;

    hg::lo("Steam") << "Stored user achievement\n";
}

void steam_manager::steam_manager_impl::load_workshop_data()
{
    const auto                          nSubscribedItems = SteamUGC()->GetNumSubscribedItems();
    sf::base::Vector<PublishedFileId_t> subscribedItemsIds(nSubscribedItems);
    SteamUGC()->GetSubscribedItems(subscribedItemsIds.data(), nSubscribedItems);

    constexpr sf::base::SizeT folderBufSize = 512;
    char                      folderBuf[folderBufSize];

    ssvuj::Obj cacheArray;

    for (PublishedFileId_t id : subscribedItemsIds)
    {
        hg::lo("Steam") << "Workshop subscribed item id: " << id << '\n';

        uint64 itemDiskSize;
        uint32 lastUpdateTimestamp;

        const bool installed = SteamUGC()->GetItemInstallInfo(id, &itemDiskSize, folderBuf, folderBufSize, &lastUpdateTimestamp);

        if (installed)
        {
            sf::base::String folderBufStr{folderBuf};

            hg::lo("Steam") << "Workshop id " << id << " is installed, with size " << itemDiskSize << " at folder "
                            << folderBufStr << '\n';

            // Write the path to an element in a JSON array.
            ssvuj::arch(cacheArray, _workshop_pack_folders.size(), folderBufStr);

            // Record the (publishedFileId, folder) pair so the host can
            // bridge file-ids to pack-ids for items that were already
            // installed at boot. (See `_workshop_file_id_to_folder` doc.)
            _workshop_file_id_to_folder.emplace(static_cast<sf::base::U64>(id), folderBufStr);

            _workshop_pack_folders.emplace(SFML_BASE_MOVE(folderBufStr));
        }
    }

    // Update the workshop cache with our loaded folders
    if (_workshop_pack_folders.size() > 0)
    {
        hg::lo("Steam") << "Updating workshop cache\n";
        ssvuj::Obj cacheObj;

        ssvuj::arch(cacheObj, "cachedPacks", cacheArray);
        ssvuj::writeToFile(cacheObj, "workshopCache.json");
    }
}

steam_manager::steam_manager_impl::steam_manager_impl() :
    _initialized{initialize_steamworks()},
    _got_stats{false},
    _got_ticket_response{false},
    _got_ticket{false},
    _ticket_steam_id{}
{
    if (!_initialized)
    {
        return;
    }

    load_workshop_data();
}

steam_manager::steam_manager_impl::~steam_manager_impl()
{
    if (_initialized)
    {
        shutdown_steamworks();
    }
}

[[nodiscard]] bool steam_manager::steam_manager_impl::is_initialized() const noexcept
{
    return _initialized;
}

bool steam_manager::steam_manager_impl::request_stats_and_achievements()
{
    if (!_initialized)
    {
        hg::lo("Steam") << "Attempted to request stats when uninitialized\n";
        return false;
    }

    if (!SteamUserStats()->RequestCurrentStats())
    {
        hg::lo("Steam") << "Failed to get stats and achievements\n";
        _got_stats = false;
        return false;
    }

    hg::lo("Steam") << "Successfully requested stats and achievements\n";
    return true;
}

bool steam_manager::steam_manager_impl::run_callbacks()
{
    if (!_initialized)
    {
        return false;
    }

    SteamAPI_RunCallbacks();
    return true;
}

bool steam_manager::steam_manager_impl::store_stats()
{
    if (!_initialized)
    {
        hg::lo("Steam") << "Attempted to store stats when uninitialized\n";
        return false;
    }

    if (!_got_stats)
    {
        hg::lo("Steam") << "Attempted to store stat without stats\n";
        return false;
    }

    if (!SteamUserStats()->StoreStats())
    {
        hg::lo("Steam") << "Failed to store stats\n";
        return false;
    }

    return true;
}

bool steam_manager::steam_manager_impl::unlock_achievement(std::string_view name)
{
    if (!_initialized)
    {
        hg::lo("Steam") << "Attempted to unlock achievement when uninitialized\n";
        return false;
    }

    if (!_got_stats)
    {
        hg::lo("Steam") << "Attempted to unlock achievement without stats\n";
        return false;
    }

    if (_unlocked_achievements.contains(sf::base::String(name)))
    {
        return false;
    }

    if (!SteamUserStats()->SetAchievement(name.data()))
    {
        hg::lo("Steam") << "Failed to unlock achievement " << name << '\n';
        return false;
    }

    _unlocked_achievements.emplace(name);
    return store_stats();
}

bool steam_manager::steam_manager_impl::set_rich_presence_in_menu()
{
    if (!_initialized)
    {
        return false;
    }

    return SteamFriends()->SetRichPresence("steam_display", "#InMenu");
}

bool steam_manager::steam_manager_impl::set_rich_presence_in_game(std::string_view level_name_format,
                                                                  std::string_view difficulty_mult_format,
                                                                  std::string_view time_format)
{
    if (!_initialized)
    {
        return false;
    }

    return SteamFriends()->SetRichPresence("levelname", level_name_format.data()) &&
           SteamFriends()->SetRichPresence("difficultymult", difficulty_mult_format.data()) &&
           SteamFriends()->SetRichPresence("time", time_format.data()) &&
           SteamFriends()->SetRichPresence("steam_display", "#InGame");
}

bool steam_manager::steam_manager_impl::set_and_store_stat(std::string_view name, int data)
{
    if (!_initialized)
    {
        return false;
    }

    // Steam API seems to be bugged, and sometimes needs floats even for integer
    // stats.
    const float as_float = data;
    if (!SteamUserStats()->SetStat(name.data(), as_float) && // Try with float.
        !SteamUserStats()->SetStat(name.data(), data))       // Try with integer.
    {
        hg::lo("Steam") << "Error setting stat '" << name << "' to '" << as_float << "'\n";

        return false;
    }

    return store_stats();
}

[[nodiscard]] bool steam_manager::steam_manager_impl::get_achievement(bool* out, std::string_view name)
{
    if (!_initialized || !_got_stats)
    {
        return false;
    }

    if (!SteamUserStats()->GetAchievement(name.data(), out))
    {
        hg::lo("Steam") << "Error getting achievement " << name << '\n';
        return false;
    }

    return true;
}

[[nodiscard]] bool steam_manager::steam_manager_impl::get_stat(int* out, std::string_view name)
{
    if (!_initialized || !_got_stats)
    {
        return false;
    }

    // Steam API seems to be bugged, and sometimes needs floats even for integer
    // stats.
    float as_float;
    if (SteamUserStats()->GetStat(name.data(), &as_float)) // Try with float.
    {
        *out = as_float;
        return true;
    }

    if (SteamUserStats()->GetStat(name.data(), out)) // Try with integer.
    {
        return true;
    }

    hg::lo("Steam") << "Error getting stat " << name.data() << '\n';
    return false;
}

[[nodiscard]] sf::base::Optional<bool> steam_manager::steam_manager_impl::is_achievement_unlocked(const char* name)
{
    bool       res{false};
    const bool rc = get_achievement(&res, name);

    if (!rc)
    {
        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(res);
}

bool steam_manager::steam_manager_impl::update_hardcoded_achievement_cube_master()
{
    if (!_initialized || !_got_stats)
    {
        return false;
    }

    const auto unlocked = [this](const char* name) -> int
    { return is_achievement_unlocked(name).valueOr(false) ? 1 : 0; };

    // "Cube Master"
    {
        int        stat;
        const bool rc = get_stat(&stat, "s0_packprogress_cube");

        if (!rc)
        {
            return false;
        }

        const int acc = unlocked("a0_babysteps") +  //
                        unlocked("a1_pointless") +  //
                        unlocked("a2_flattering") + //
                        unlocked("a3_seconddim") +  //
                        unlocked("a4_apeirogon") +  //
                        unlocked("a5_commando") +   //
                        unlocked("a6_euclidian") +  //
                        unlocked("a7_pi") +         //
                        unlocked("a8_lab") +        //
                        unlocked("a9_ratio");

        if (acc > stat)
        {
            if (!set_and_store_stat("s0_packprogress_cube", acc))
            {
                return false;
            }
        }
    }

    return true;
}

bool steam_manager::steam_manager_impl::update_hardcoded_achievement_hypercube_master()
{
    if (!_initialized || !_got_stats)
    {
        return false;
    }

    const auto unlocked = [this](const char* name) -> int
    { return is_achievement_unlocked(name).valueOr(false) ? 1 : 0; };

    // "Hypercube Master"
    {
        int        stat;
        const bool rc = get_stat(&stat, "s1_packprogress_hypercube");

        if (!rc)
        {
            return false;
        }

        const int acc = unlocked("a11_evotutorial") +      //
                        unlocked("a12_disco") +            //
                        unlocked("a13_acceleradiant") +    //
                        unlocked("a14_gforce") +           //
                        unlocked("a15_incongruence") +     //
                        unlocked("a16_slither") +          //
                        unlocked("a17_polyhedrug") +       //
                        unlocked("a18_reppaws") +          //
                        unlocked("a19_centrifugalforce") + //
                        unlocked("a20_massacre");

        if (acc > stat)
        {
            if (!set_and_store_stat("s1_packprogress_hypercube", acc))
            {
                return false;
            }
        }
    }

    return true;
}

bool steam_manager::steam_manager_impl::update_hardcoded_achievement_cube_god()
{
    if (!_initialized || !_got_stats)
    {
        return false;
    }

    const auto unlocked = [this](const char* name) -> int
    { return is_achievement_unlocked(name).valueOr(false) ? 1 : 0; };

    // "Cube God"
    {
        int        stat;
        const bool rc = get_stat(&stat, "s2_packprogress_cubegod");

        if (!rc)
        {
            return false;
        }

        const int acc = unlocked("a25_pointless_hard") +  //
                        unlocked("a26_flattering_hard") + //
                        unlocked("a27_seconddim_hard") +  //
                        unlocked("a28_apeirogon_hard") +  //
                        unlocked("a29_commando_hard") +   //
                        unlocked("a30_euclidian_hard") +  //
                        unlocked("a31_pi_hard") +         //
                        unlocked("a32_lab_hard") +        //
                        unlocked("a33_ratio_hard");

        if (acc > stat)
        {
            if (!set_and_store_stat("s2_packprogress_cubegod", acc))
            {
                return false;
            }
        }
    }

    return true;
}

bool steam_manager::steam_manager_impl::update_hardcoded_achievement_hypercube_god()
{
    if (!_initialized || !_got_stats)
    {
        return false;
    }

    const auto unlocked = [this](const char* name) -> int
    { return is_achievement_unlocked(name).valueOr(false) ? 1 : 0; };

    // "Hypercube Master"
    {
        int        stat;
        const bool rc = get_stat(&stat, "s3_packprogress_hypercubegod");

        if (!rc)
        {
            return false;
        }

        const int acc = unlocked("a38_disco_hard") +            //
                        unlocked("a39_acceleradiant_hard") +    //
                        unlocked("a40_gforce_hard") +           //
                        unlocked("a41_incongruence_hard") +     //
                        unlocked("a42_slither_hard") +          //
                        unlocked("a43_polyhedrug_hard") +       //
                        unlocked("a44_reppaws_hard") +          //
                        unlocked("a45_centrifugalforce_hard") + //
                        unlocked("a46_massacre_hard");

        if (acc > stat)
        {
            if (!set_and_store_stat("s3_packprogress_hypercubegod", acc))
            {
                return false;
            }
        }
    }

    return true;
}

bool steam_manager::steam_manager_impl::update_hardcoded_achievements()
{
    // Intentionally not short-circuiting via boolean operators here.

    int failures = 0;

    failures += static_cast<int>(!update_hardcoded_achievement_cube_master()) +
                static_cast<int>(!update_hardcoded_achievement_hypercube_master()) +
                static_cast<int>(!update_hardcoded_achievement_cube_god()) +
                static_cast<int>(!update_hardcoded_achievement_hypercube_god());

    return failures == 0;
}

void steam_manager::steam_manager_impl::for_workshop_pack_folders(
    sf::base::FixedFunction<void(const sf::base::String&), 64> f) const
{
    if (!_initialized)
    {
        return;
    }

    for (const sf::base::String& s : _workshop_pack_folders)
    {
        f(s);
    }
}

void steam_manager::steam_manager_impl::for_workshop_subscribed_items(
    sf::base::FixedFunction<void(sf::base::U64, const sf::base::String&), 64> f) const
{
    if (!_initialized)
    {
        return;
    }

    for (const auto& [fileId, folder] : _workshop_file_id_to_folder)
    {
        f(fileId, folder);
    }
}

// ----------------------------------------------------------------------------
// Workshop API impl

void steam_manager::steam_manager_impl::query_workshop_items(WorkshopQueryMode mode, int page)
{
    if (!_initialized)
    {
        return;
    }

    EUGCQuery sortMode = k_EUGCQuery_RankedByVote;
    switch (mode)
    {
        case WorkshopQueryMode::MostPopular:
            sortMode = k_EUGCQuery_RankedByVote;
            break;
        case WorkshopQueryMode::Newest:
            sortMode = k_EUGCQuery_RankedByPublicationDate;
            break;
        case WorkshopQueryMode::Trending:
            sortMode = k_EUGCQuery_RankedByTrend;
            break;
        case WorkshopQueryMode::All:
            sortMode = k_EUGCQuery_RankedByVote;
            break;
    }

    const AppId_t appId = SteamUtils()->GetAppID();

    UGCQueryHandle_t handle = SteamUGC()->CreateQueryAllUGCRequest(sortMode,
                                                                   k_EUGCMatchingUGCType_All,
                                                                   appId, // creatorAppID
                                                                   appId, // consumerAppID
                                                                   page < 1 ? 1 : page);

    if (handle == k_UGCQueryHandleInvalid)
    {
        hg::lo("Steam") << "Workshop: failed to create query\n";
        return;
    }

    SteamUGC()->SetReturnLongDescription(handle, true);
    // Ask Steam to also surface the children (declared dependencies) of
    // each item so `on_query_completed` can populate `WorkshopItem::dependencies`.
    SteamUGC()->SetReturnChildren(handle, true);

    _pending_query            = handle;
    _pending_query_is_details = false;
    SteamAPICall_t call       = SteamUGC()->SendQueryUGCRequest(handle);
    _query_call_result.Set(call, this, &steam_manager_impl::on_query_completed);
}

void steam_manager::steam_manager_impl::on_query_completed(SteamUGCQueryCompleted_t* data, bool io_failure)
{
    if (data == nullptr || io_failure || data->m_eResult != k_EResultOK || _pending_query == k_UGCQueryHandleInvalid)
    {
        hg::lo("Steam") << "Workshop query failed (rc: " << (data ? static_cast<int>(data->m_eResult) : -1) << ")\n";
        if (_pending_query != k_UGCQueryHandleInvalid)
        {
            SteamUGC()->ReleaseQueryUGCRequest(_pending_query);
            _pending_query = k_UGCQueryHandleInvalid;
        }
        _pending_query_is_details = false;
        return;
    }

    WorkshopEvent ev;
    ev.kind = _pending_query_is_details ? WorkshopEvent::Kind::DetailsComplete : WorkshopEvent::Kind::QueryComplete;
    ev.totalMatching = data->m_unTotalMatchingResults;

    for (uint32 i = 0; i < data->m_unNumResultsReturned; ++i)
    {
        SteamUGCDetails_t details{};
        if (!SteamUGC()->GetQueryUGCResult(_pending_query, i, &details))
        {
            continue;
        }

        WorkshopItem item;
        item.publishedFileId = details.m_nPublishedFileId;
        item.title           = details.m_rgchTitle;
        item.description     = details.m_rgchDescription;
        item.sizeBytes       = static_cast<sf::base::U64>(details.m_nFileSize);
        // Author and isSubscribed/isInstalled require separate calls; leave
        // as defaults for now. The browse UI shows what we have; expansion
        // can come later.
        item.isInstalled  = SteamUGC()->GetItemState(details.m_nPublishedFileId) & k_EItemStateInstalled;
        item.isSubscribed = SteamUGC()->GetItemState(details.m_nPublishedFileId) & k_EItemStateSubscribed;

        // Primary preview URL (the "main" Workshop screenshot). The UI
        // displays this as a per-item preview thumbnail; downloading
        // and decoding the image is the UI layer's responsibility.
        char previewUrlBuf[1024] = {};
        if (SteamUGC()->GetQueryUGCPreviewURL(_pending_query, i, previewUrlBuf, sizeof(previewUrlBuf)))
        {
            item.previewUrl = previewUrlBuf;
        }

        // Pull declared dependencies (children) into the item. Steam packs
        // them into a flat array; we copy out so the WorkshopItem is
        // self-contained once the query handle is released below.
        if (details.m_unNumChildren > 0)
        {
            sf::base::Vector<PublishedFileId_t> buf;
            buf.resize(details.m_unNumChildren);
            if (SteamUGC()->GetQueryUGCChildren(_pending_query, i, buf.data(), details.m_unNumChildren))
            {
                item.dependencies.reserve(details.m_unNumChildren);
                for (PublishedFileId_t childId : buf)
                {
                    item.dependencies.emplaceBack(static_cast<sf::base::U64>(childId));
                }
            }
        }

        ev.queryResults.emplaceBack(SFML_BASE_MOVE(item));
    }

    SteamUGC()->ReleaseQueryUGCRequest(_pending_query);
    _pending_query            = k_UGCQueryHandleInvalid;
    _pending_query_is_details = false;

    _workshop_events.emplaceBack(SFML_BASE_MOVE(ev));
}

void steam_manager::steam_manager_impl::query_workshop_details(const sf::base::Vector<sf::base::U64>& ids)
{
    if (!_initialized)
        return;
    if (ids.empty())
        return;
    if (_pending_query != k_UGCQueryHandleInvalid)
    {
        // A query is already in flight; KISS -- drop this details request.
        // The caller (UI) will retry next time the user navigates.
        return;
    }

    sf::base::Vector<PublishedFileId_t> nativeIds;
    nativeIds.reserve(ids.size());
    for (sf::base::U64 id : ids)
    {
        nativeIds.emplaceBack(static_cast<PublishedFileId_t>(id));
    }

    UGCQueryHandle_t handle = SteamUGC()->CreateQueryUGCDetailsRequest(nativeIds.data(),
                                                                       static_cast<uint32>(nativeIds.size()));

    if (handle == k_UGCQueryHandleInvalid)
    {
        hg::lo("Steam") << "Workshop: failed to create details query\n";
        return;
    }

    SteamUGC()->SetReturnLongDescription(handle, true);
    SteamUGC()->SetReturnChildren(handle, true);

    _pending_query            = handle;
    _pending_query_is_details = true;
    SteamAPICall_t call       = SteamUGC()->SendQueryUGCRequest(handle);
    _query_call_result.Set(call, this, &steam_manager_impl::on_query_completed);
}

void steam_manager::steam_manager_impl::subscribe_workshop_item(sf::base::U64 publishedFileId)
{
    if (!_initialized)
        return;
    SteamUGC()->SubscribeItem(static_cast<PublishedFileId_t>(publishedFileId));
    SteamUGC()->DownloadItem(static_cast<PublishedFileId_t>(publishedFileId), /* highPriority */ true);
}

void steam_manager::steam_manager_impl::unsubscribe_workshop_item(sf::base::U64 publishedFileId)
{
    if (!_initialized)
        return;
    SteamUGC()->UnsubscribeItem(static_cast<PublishedFileId_t>(publishedFileId));
}

bool steam_manager::steam_manager_impl::is_workshop_item_subscribed(sf::base::U64 publishedFileId) const noexcept
{
    if (!_initialized)
        return false;
    return (SteamUGC()->GetItemState(static_cast<PublishedFileId_t>(publishedFileId)) & k_EItemStateSubscribed) != 0;
}

sf::base::Optional<WorkshopEvent> steam_manager::steam_manager_impl::poll_workshop_event()
{
    if (_workshop_events.empty())
    {
        return sf::base::nullOpt;
    }
    WorkshopEvent ev = SFML_BASE_MOVE(_workshop_events.front());
    _workshop_events.erase(_workshop_events.begin());
    return sf::base::makeOptional(SFML_BASE_MOVE(ev));
}

void steam_manager::steam_manager_impl::request_workshop_preview(sf::base::U64 publishedFileId, const sf::base::String& url)
{
    if (!_initialized || url.empty())
    {
        return;
    }

    // Skip duplicates: if the same item already has a request in flight,
    // don't fire a second one. The UI relies on a "request once, await
    // event" model, so multiple concurrent fetches for the same id would
    // just waste bandwidth.
    for (const auto& p : _pending_previews)
    {
        if (p.publishedFileId == publishedFileId)
        {
            return;
        }
    }

    HTTPRequestHandle h = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, url.cStr());
    if (h == INVALID_HTTPREQUEST_HANDLE)
    {
        return;
    }

    SteamAPICall_t call{};
    if (!SteamHTTP()->SendHTTPRequest(h, &call))
    {
        SteamHTTP()->ReleaseHTTPRequest(h);
        return;
    }

    _pending_previews.emplaceBack(PendingPreview{publishedFileId, h, call});
}

void steam_manager::steam_manager_impl::pump_workshop_http()
{
    if (!_initialized || _pending_previews.empty())
    {
        return;
    }

    // Walk the list of in-flight requests. Iterate over indices because
    // we mutate the vector mid-loop (erase completed entries).
    for (sf::base::SizeT i = 0; i < _pending_previews.size();)
    {
        PendingPreview& p         = _pending_previews[i];
        bool            ioFailed  = false;
        const bool      completed = SteamUtils()->IsAPICallCompleted(p.apiCall, &ioFailed);
        if (!completed)
        {
            ++i;
            continue;
        }

        // Pull the call result. If `IsAPICallCompleted` reported failure
        // (`ioFailed`) we still need to release the handle, but we skip
        // pulling the body.
        HTTPRequestCompleted_t result{};
        bool                   pullFailed = false;
        const bool             pulled     = SteamUtils()->GetAPICallResult(p.apiCall,
                                                                           &result,
                                                                           sizeof(result),
                                                                           HTTPRequestCompleted_t::k_iCallback,
                                                                           &pullFailed);

        if (pulled && !pullFailed && !ioFailed && result.m_bRequestSuccessful && result.m_eStatusCode >= 200 &&
            result.m_eStatusCode < 300)
        {
            uint32 bodySize = 0;
            if (SteamHTTP()->GetHTTPResponseBodySize(p.httpHandle, &bodySize) && bodySize > 0)
            {
                sf::base::Vector<sf::base::U8> bytes;
                bytes.resize(static_cast<sf::base::SizeT>(bodySize));
                if (SteamHTTP()->GetHTTPResponseBodyData(p.httpHandle, bytes.data(), bodySize))
                {
                    WorkshopEvent ev;
                    ev.kind            = WorkshopEvent::Kind::PreviewDownloaded;
                    ev.publishedFileId = p.publishedFileId;
                    ev.previewBytes    = SFML_BASE_MOVE(bytes);
                    _workshop_events.emplaceBack(SFML_BASE_MOVE(ev));
                }
            }
        }

        SteamHTTP()->ReleaseHTTPRequest(p.httpHandle);
        _pending_previews.erase(_pending_previews.begin() + i);
    }
}

void steam_manager::steam_manager_impl::on_item_installed(ItemInstalled_t* data)
{
    if (data == nullptr)
        return;

    hg::lo("Steam") << "on_item_installed fileId=" << data->m_nPublishedFileId << '\n';

    constexpr sf::base::SizeT folderBufSize            = 512;
    char                      folderBuf[folderBufSize] = {};
    uint64                    diskSize{};
    uint32                    timestamp{};
    if (SteamUGC()->GetItemInstallInfo(data->m_nPublishedFileId, &diskSize, folderBuf, folderBufSize, &timestamp))
    {
        hg::lo("Steam") << "  GetItemInstallInfo ok, folder='" << folderBuf << "'\n";
        WorkshopEvent ev;
        ev.kind            = WorkshopEvent::Kind::ItemInstalled;
        ev.publishedFileId = data->m_nPublishedFileId;
        ev.installFolder   = folderBuf;
        _workshop_events.emplaceBack(SFML_BASE_MOVE(ev));
    }
    else
    {
        hg::lo("Steam") << "  GetItemInstallInfo FAILED -- ItemInstalled event NOT enqueued\n";
    }
}

void steam_manager::steam_manager_impl::on_item_subscribed(RemoteStoragePublishedFileSubscribed_t* data)
{
    if (data == nullptr)
        return;

    const auto fileId    = data->m_nPublishedFileId;
    const auto stateBits = SteamUGC()->GetItemState(fileId);

    hg::lo("Steam") << "on_item_subscribed fileId=" << fileId << " ItemState bits=" << stateBits << '\n';

    {
        WorkshopEvent ev;
        ev.kind            = WorkshopEvent::Kind::ItemSubscribed;
        ev.publishedFileId = fileId;
        _workshop_events.emplaceBack(SFML_BASE_MOVE(ev));
    }

    // Re-subscribing to a pack whose files are already on disk (e.g.
    // user unsubscribed earlier and the disk content was preserved, or
    // the pack was already installed at boot and unsubscribed via the
    // Steam overlay). Steam takes the fast path: no `DownloadItem` is
    // queued, so `on_download_item_result` never fires. Without this
    // synthesis, the host never sees `ItemInstalled` for this case and
    // the LevelSelect screen doesn't refresh until the game restarts.
    //
    // Fresh subscribes (pack never on disk) won't have the install bit
    // here -- they take the normal path through `on_download_item_result`,
    // which itself synthesizes `ItemInstalled` once the bytes land.
    if (stateBits & k_EItemStateInstalled)
    {
        constexpr sf::base::SizeT folderBufSize            = 512;
        char                      folderBuf[folderBufSize] = {};
        uint64                    diskSize{};
        uint32                    timestamp{};
        if (SteamUGC()->GetItemInstallInfo(fileId, &diskSize, folderBuf, folderBufSize, &timestamp))
        {
            hg::lo("Steam") << "  already installed, synthesizing ItemInstalled, folder='" << folderBuf << "'\n";
            WorkshopEvent installed;
            installed.kind            = WorkshopEvent::Kind::ItemInstalled;
            installed.publishedFileId = fileId;
            installed.installFolder   = folderBuf;
            _workshop_events.emplaceBack(SFML_BASE_MOVE(installed));
        }
        else
        {
            hg::lo("Steam") << "  install bit set but GetItemInstallInfo failed; deferring to download path\n";
        }
    }
}

void steam_manager::steam_manager_impl::on_item_unsubscribed(RemoteStoragePublishedFileUnsubscribed_t* data)
{
    if (data == nullptr)
        return;

    hg::lo("Steam") << "on_item_unsubscribed fileId=" << data->m_nPublishedFileId << '\n';

    WorkshopEvent ev;
    ev.kind            = WorkshopEvent::Kind::ItemUnsubscribed;
    ev.publishedFileId = data->m_nPublishedFileId;
    _workshop_events.emplaceBack(SFML_BASE_MOVE(ev));
}

void steam_manager::steam_manager_impl::on_download_item_result(DownloadItemResult_t* data)
{
    if (data == nullptr)
        return;

    hg::lo("Steam") << "on_download_item_result fileId=" << data->m_nPublishedFileId << " result="
                    << static_cast<int>(data->m_eResult) << " (k_EResultOK is " << static_cast<int>(k_EResultOK) << ")\n";

    // Surface a progress tick at completion regardless of result.
    {
        WorkshopEvent ev;
        ev.kind            = WorkshopEvent::Kind::DownloadProgress;
        ev.publishedFileId = data->m_nPublishedFileId;
        _workshop_events.emplaceBack(SFML_BASE_MOVE(ev));
    }

    // On a successful download, ALSO synthesize an `ItemInstalled`
    // event. Despite what `isteamugc.h` documents, `ItemInstalled_t`
    // observably does not fire promptly (or at all) for first-time
    // workshop subscribes in the field -- the fresh-subscribe path
    // can complete the actual download but `ItemInstalled_t` either
    // arrives much later (after several other Steam events) or never
    // arrives within the user's patience. Without this fallback, a
    // brand-new pack only shows up after a game restart.
    //
    // `DownloadItemResult_t` is the result of the explicit
    // `DownloadItem()` call we make in `subscribe_workshop_item` for
    // the high-priority hint, so it fires reliably whenever the bytes
    // land. Treating it as a redundant install signal means we may
    // dispatch `ItemInstalled` twice for the same pack (once from
    // here, once from `on_item_installed` if it does fire). That's
    // safe: `HGAssets::installPackAtRuntime` is idempotent -- a
    // second call for the same folder finds the existing `PackData`
    // and returns its id without touching state.
    if (data->m_eResult == k_EResultOK)
    {
        constexpr sf::base::SizeT folderBufSize            = 512;
        char                      folderBuf[folderBufSize] = {};
        uint64                    diskSize{};
        uint32                    timestamp{};
        if (SteamUGC()->GetItemInstallInfo(data->m_nPublishedFileId, &diskSize, folderBuf, folderBufSize, &timestamp))
        {
            hg::lo("Steam") << "  download ok + GetItemInstallInfo ok -> synthesize ItemInstalled, folder='"
                            << folderBuf << "'\n";
            WorkshopEvent ev;
            ev.kind            = WorkshopEvent::Kind::ItemInstalled;
            ev.publishedFileId = data->m_nPublishedFileId;
            ev.installFolder   = folderBuf;
            _workshop_events.emplaceBack(SFML_BASE_MOVE(ev));
        }
        else
        {
            hg::lo("Steam") << "  download ok but GetItemInstallInfo FAILED -- ItemState bits: "
                            << SteamUGC()->GetItemState(data->m_nPublishedFileId) << '\n';
        }
    }
    else
    {
        hg::lo("Steam") << "  download FAILED (result=" << static_cast<int>(data->m_eResult) << ")\n";
    }
}

// ----------------------------------------------------------------------------

[[maybe_unused]] static sf::base::U32 unSecretData = 123'456;

bool steam_manager::steam_manager_impl::request_encrypted_app_ticket()
{
    if (!_initialized)
    {
        hg::lo("Steam") << "Attempted to request encrypted app ticket when uninitialized\n";

        return false;
    }

    #if __has_include("SSVOpenHexagon/Online/SecretSteamKey.hpp")
    const SteamAPICall_t handle = SteamUser()->RequestEncryptedAppTicket(&unSecretData, sizeof(unSecretData));

    _encrypted_app_ticket_response_call_result
        .Set(handle, this, &steam_manager::steam_manager_impl::on_encrypted_app_ticket_response);

    return true;
    #else
    hg::lo("Steam") << "Attempted to request encrypted app ticket without secret key\n";

    return false;
    #endif
}

void steam_manager::steam_manager_impl::on_encrypted_app_ticket_response([[maybe_unused]] EncryptedAppTicketResponse_t* data,
                                                                         [[maybe_unused]] bool io_failure)
{
    #if __has_include("SSVOpenHexagon/Online/SecretSteamKey.hpp")
    hg::lo("Steam") << "Received encrypted app ticket response\n";
    _got_ticket_response = true;

    if (io_failure)
    {
        hg::lo("Steam") << "Error: encrypted app ticket response IO failure\n";

        return;
    }

    if (data->m_eResult == k_EResultNoConnection)
    {
        hg::lo("Steam") << "Error: requested encrypted app ticket while not connected to "
                           "Steam\n";

        return;
    }

    if (data->m_eResult == k_EResultDuplicateRequest)
    {
        hg::lo("Steam") << "Error: requested encrypted app ticket while there is already a "
                           "pending request\n";

        return;
    }

    if (data->m_eResult == k_EResultLimitExceeded)
    {
        hg::lo("Steam") << "Error: requested encrypted app ticket more than "
                           "once per minute\n";

        return;
    }

    if (data->m_eResult != k_EResultOK)
    {
        hg::lo("Steam") << "Error: requested encrypted app ticket, got unexpected result '" << data->m_eResult << "'\n";

        return;
    }

    SSVOH_ASSERT(data->m_eResult == k_EResultOK);

    sf::base::U8  rgubTicket[1024];
    sf::base::U32 cubTicket;

    if (!SteamUser()->GetEncryptedAppTicket(rgubTicket, sizeof(rgubTicket), &cubTicket))
    {
        hg::lo("Steam") << "Error: 'GetEncryptedAppTicket' failed\n";
        return;
    }

    sf::base::U8  rgubDecrypted[1024];
    sf::base::U32 cubDecrypted = sizeof(rgubDecrypted);

    // clang-format off
    constexpr sf::base::U8 rgubKey[k_nSteamEncryptedAppTicketSymmetricKeyLen] =
    {
        #include "SSVOpenHexagon/Online/SecretSteamKey.hpp"
    };
    // clang-format on

    if (!SteamEncryptedAppTicket_BDecryptTicket(rgubTicket, cubTicket, rgubDecrypted, &cubDecrypted, rgubKey, sizeof(rgubKey)))
    {
        hg::lo("Steam") << "Error: 'BDecryptTicket' failed\n";
        return;
    }

    if (!SteamEncryptedAppTicket_BIsTicketForApp(rgubDecrypted, cubDecrypted, SteamUtils()->GetAppID()))
    {
        hg::lo("Steam") << "Error: ticket for wrong app id\n";
        return;
    }

    CSteamID steamIDFromTicket;
    SteamEncryptedAppTicket_GetTicketSteamID(rgubDecrypted, cubDecrypted, &steamIDFromTicket);

    if (const sf::base::Optional<CSteamID> user_steam_id = get_user_steam_id(); user_steam_id.hasValue())
    {
        if (steamIDFromTicket != *user_steam_id)
        {
            hg::lo("Steam") << "Error: ticket for wrong user\n";
            return;
        }
        else
        {
            hg::lo("Steam") << "Steam ID ticket matches user Steam ID\n";
        }
    }
    else
    {
        hg::lo("Steam") << "Could not retrieve user Steam ID\n";
        return;
    }

    sf::base::U32 cubData;
    sf::base::U32 pUnSecretData;

    const sf::base::U8* receivedData = SteamEncryptedAppTicket_GetUserVariableData(rgubDecrypted, cubDecrypted, &cubData);

    std::memcpy(static_cast<void*>(&pUnSecretData), static_cast<const void*>(receivedData), sizeof(pUnSecretData));

    if (cubData != sizeof(sf::base::U32) || pUnSecretData != unSecretData)
    {
        hg::lo("Steam") << "Error: failed to retrieve secret data\n";
    }

    _got_ticket = true;
    _ticket_steam_id.emplace(steamIDFromTicket);

    hg::lo("Steam") << "GetEncryptedAppTicket succeeded (steamId: '" << steamIDFromTicket.ConvertToUint64() << "')\n";
    #else
    _got_ticket_response = true;
    _got_ticket          = false;
    _ticket_steam_id.reset();
    #endif
}

[[nodiscard]] bool steam_manager::steam_manager_impl::got_encrypted_app_ticket_response() const noexcept
{
    return _got_ticket_response;
}

[[nodiscard]] bool steam_manager::steam_manager_impl::got_encrypted_app_ticket() const noexcept
{
    return _got_ticket;
}

[[nodiscard]] sf::base::Optional<sf::base::U64> steam_manager::steam_manager_impl::get_ticket_steam_id() const noexcept
{
    return sf::base::makeOptional(_ticket_steam_id->ConvertToUint64());
}

// ----------------------------------------------------------------------------

[[nodiscard]] const steam_manager::steam_manager_impl& steam_manager::impl() const noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

[[nodiscard]] steam_manager::steam_manager_impl& steam_manager::impl() noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

steam_manager::steam_manager() : _impl{sf::base::makeUnique<steam_manager_impl>()}
{
}

steam_manager::~steam_manager() = default;

[[nodiscard]] bool steam_manager::is_initialized() const noexcept
{
    return impl().is_initialized();
}

bool steam_manager::request_stats_and_achievements()
{
    return impl().request_stats_and_achievements();
}

bool steam_manager::run_callbacks()
{
    return impl().run_callbacks();
}

bool steam_manager::store_stats()
{
    return impl().store_stats();
}

bool steam_manager::unlock_achievement(std::string_view name)
{
    return impl().unlock_achievement(name);
}

bool steam_manager::set_rich_presence_in_menu()
{
    return impl().set_rich_presence_in_menu();
}

bool steam_manager::set_rich_presence_in_game(std::string_view level_name_format,
                                              std::string_view difficulty_mult_format,
                                              std::string_view time_format)
{
    return impl().set_rich_presence_in_game(level_name_format, difficulty_mult_format, time_format);
}

bool steam_manager::set_and_store_stat(std::string_view name, int data)
{
    return impl().set_and_store_stat(name, data);
}

[[nodiscard]] bool steam_manager::get_achievement(bool* out, std::string_view name)
{
    return impl().get_achievement(out, name);
}

[[nodiscard]] bool steam_manager::get_stat(int* out, std::string_view name)
{
    return impl().get_stat(out, name);
}

bool steam_manager::update_hardcoded_achievements()
{
    return impl().update_hardcoded_achievements();
}

void steam_manager::for_workshop_pack_folders(sf::base::FixedFunction<void(const sf::base::String&), 64> f) const
{
    return impl().for_workshop_pack_folders(f);
}

void steam_manager::for_workshop_subscribed_items(sf::base::FixedFunction<void(sf::base::U64, const sf::base::String&), 64> f) const
{
    return impl().for_workshop_subscribed_items(f);
}

bool steam_manager::request_encrypted_app_ticket()
{
    return impl().request_encrypted_app_ticket();
}

[[nodiscard]] bool steam_manager::got_encrypted_app_ticket_response() const noexcept
{
    return impl().got_encrypted_app_ticket_response();
}

[[nodiscard]] bool steam_manager::got_encrypted_app_ticket() const noexcept
{
    return impl().got_encrypted_app_ticket();
}

[[nodiscard]] sf::base::Optional<sf::base::U64> steam_manager::get_ticket_steam_id() const noexcept
{
    return impl().get_ticket_steam_id();
}

void steam_manager::query_workshop_items(WorkshopQueryMode mode, int page)
{
    impl().query_workshop_items(mode, page);
}

void steam_manager::query_workshop_details(const sf::base::Vector<sf::base::U64>& ids)
{
    impl().query_workshop_details(ids);
}

void steam_manager::subscribe_workshop_item(sf::base::U64 publishedFileId)
{
    impl().subscribe_workshop_item(publishedFileId);
}

void steam_manager::unsubscribe_workshop_item(sf::base::U64 publishedFileId)
{
    impl().unsubscribe_workshop_item(publishedFileId);
}

[[nodiscard]] bool steam_manager::is_workshop_item_subscribed(sf::base::U64 publishedFileId) const noexcept
{
    return impl().is_workshop_item_subscribed(publishedFileId);
}

[[nodiscard]] sf::base::Optional<WorkshopEvent> steam_manager::poll_workshop_event()
{
    return impl().poll_workshop_event();
}

void steam_manager::request_workshop_preview(sf::base::U64 publishedFileId, const sf::base::String& url)
{
    impl().request_workshop_preview(publishedFileId, url);
}

void steam_manager::pump_workshop_http()
{
    impl().pump_workshop_http();
}

} // namespace hg::Steam

#else

namespace hg::Steam
{

class steam_manager::steam_manager_impl
{
};

steam_manager::steam_manager() : _impl{nullptr}
{
}

steam_manager::~steam_manager() = default;

[[nodiscard]] bool steam_manager::is_initialized() const noexcept
{
    return false;
}

bool steam_manager::request_stats_and_achievements()
{
    return false;
}

bool steam_manager::run_callbacks()
{
    return false;
}

bool steam_manager::store_stats()
{
    return false;
}

bool steam_manager::unlock_achievement([[maybe_unused]] std::string_view name)
{
    return false;
}

bool steam_manager::set_rich_presence_in_menu()
{
    return false;
}

bool steam_manager::set_rich_presence_in_game([[maybe_unused]] std::string_view level_name_format,
                                              [[maybe_unused]] std::string_view difficulty_mult_format,
                                              [[maybe_unused]] std::string_view time_format)
{
    return false;
}

bool steam_manager::set_and_store_stat([[maybe_unused]] std::string_view name, [[maybe_unused]] int data)
{
    return false;
}

[[nodiscard]] bool steam_manager::get_achievement([[maybe_unused]] bool* out, [[maybe_unused]] std::string_view name)
{
    return false;
}

[[nodiscard]] bool steam_manager::get_stat([[maybe_unused]] int* out, [[maybe_unused]] std::string_view name)
{
    return false;
}

bool steam_manager::update_hardcoded_achievements()
{
    return false;
}

void steam_manager::for_workshop_pack_folders([[maybe_unused]] sf::base::FixedFunction<void(const sf::base::String&), 64> f) const
{
}

void steam_manager::for_workshop_subscribed_items(
    [[maybe_unused]] sf::base::FixedFunction<void(sf::base::U64, const sf::base::String&), 64> f) const
{
}

bool steam_manager::request_encrypted_app_ticket()
{
    return false;
}

[[nodiscard]] bool steam_manager::got_encrypted_app_ticket_response() const noexcept
{
    return false;
}

[[nodiscard]] bool steam_manager::got_encrypted_app_ticket() const noexcept
{
    return false;
}

[[nodiscard]] sf::base::Optional<sf::base::U64> steam_manager::get_ticket_steam_id() const noexcept
{
    return sf::base::nullOpt;
}

void steam_manager::query_workshop_items([[maybe_unused]] WorkshopQueryMode mode, [[maybe_unused]] int page)
{
}

void steam_manager::query_workshop_details([[maybe_unused]] const sf::base::Vector<sf::base::U64>& ids)
{
}

void steam_manager::subscribe_workshop_item([[maybe_unused]] sf::base::U64 publishedFileId)
{
}

void steam_manager::unsubscribe_workshop_item([[maybe_unused]] sf::base::U64 publishedFileId)
{
}

[[nodiscard]] bool steam_manager::is_workshop_item_subscribed([[maybe_unused]] sf::base::U64 publishedFileId) const noexcept
{
    return false;
}

[[nodiscard]] sf::base::Optional<WorkshopEvent> steam_manager::poll_workshop_event()
{
    return sf::base::nullOpt;
}

void steam_manager::request_workshop_preview([[maybe_unused]] sf::base::U64           publishedFileId,
                                             [[maybe_unused]] const sf::base::String& url)
{
}

void steam_manager::pump_workshop_http()
{
}

} // namespace hg::Steam

#endif
