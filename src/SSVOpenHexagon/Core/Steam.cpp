// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Steam.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include "steam/steam_api.h"

#include <array>
#include <charconv>

namespace hg::Steam
{

[[nodiscard]] static bool initialize_steamworks()
{
    ssvu::lo("Steam") << "Initializing Steam API\n";

    if(SteamAPI_Init())
    {
        ssvu::lo("Steam") << "Steam API successfully initialized\n";
        return true;
    }

    ssvu::lo("Steam") << "Failed to initialize Steam API\n";
    return false;
}

static void shutdown_steamworks()
{
    ssvu::lo("Steam") << "Shutting down Steam API\n";
    SteamAPI_Shutdown();
    ssvu::lo("Steam") << "Shut down Steam API\n";
}

void steam_manager::on_user_stats_received(UserStatsReceived_t* data)
{
    (void)data;

    ssvu::lo("Steam") << "Received user stats (rc: " << data->m_eResult
                      << ")\n";

    _got_stats = true;
}

void steam_manager::on_user_stats_stored(UserStatsStored_t* data)
{
    (void)data;

    ssvu::lo("Steam") << "Stored user stats\n";
}

void steam_manager::on_user_achievement_stored(UserAchievementStored_t* data)
{
    (void)data;

    ssvu::lo("Steam") << "Stored user achievement\n";
}

void steam_manager::load_workshop_data()
{
    const auto nSubscribedItems = SteamUGC()->GetNumSubscribedItems();
    std::vector<PublishedFileId_t> subscribedItemsIds(nSubscribedItems);
    SteamUGC()->GetSubscribedItems(subscribedItemsIds.data(), nSubscribedItems);

    constexpr std::size_t folderBufSize = 512;
    char folderBuf[folderBufSize];

    for(PublishedFileId_t id : subscribedItemsIds)
    {
        ssvu::lo("Steam") << "Workshop subscribed item id: " << id << '\n';

        uint64 itemDiskSize;
        uint32 lastUpdateTimestamp;

        const bool installed = SteamUGC()->GetItemInstallInfo(
            id, &itemDiskSize, folderBuf, folderBufSize, &lastUpdateTimestamp);

        if(installed)
        {
            ssvu::lo("Steam")
                << "Workshop id " << id << " is installed, with size "
                << itemDiskSize << " at folder " << std::string{folderBuf}
                << '\n';

            _workshop_pack_folders.emplace(std::string{folderBuf});
        }
    }
}

steam_manager::steam_manager()
    : _initialized{initialize_steamworks()}, _got_stats{false}
{
    if(!_initialized)
    {
        return;
    }

    load_workshop_data();
}

steam_manager::~steam_manager()
{
    if(_initialized)
    {
        shutdown_steamworks();
    }
}

bool steam_manager::request_stats_and_achievements()
{
    if(!_initialized)
    {
        ssvu::lo("Steam") << "Attempted to request stats when uninitialized\n";
        return false;
    }

    if(!SteamUserStats()->RequestCurrentStats())
    {
        ssvu::lo("Steam") << "Failed to get stats and achievements\n";
        _got_stats = false;
        return false;
    }

    ssvu::lo("Steam") << "Successfully requested stats and achievements\n";
    return true;
}

bool steam_manager::run_callbacks()
{
    if(!_initialized)
    {
        return false;
    }

    SteamAPI_RunCallbacks();
    return true;
}

bool steam_manager::store_stats()
{
    if(!_initialized)
    {
        ssvu::lo("Steam") << "Attempted to store stats when uninitialized\n";
        return false;
    }

    if(!_got_stats)
    {
        ssvu::lo("Steam") << "Attempted to store stat without stats\n";
        return false;
    }

    if(!SteamUserStats()->StoreStats())
    {
        ssvu::lo("Steam") << "Failed to store stats\n";
        return false;
    }

    return true;
}

bool steam_manager::unlock_achievement(std::string_view name)
{
    if(!_initialized)
    {
        return false;
    }

    if(!_got_stats)
    {
        ssvu::lo("Steam") << "Attempted to unlock achievement without stats\n";
        return false;
    }

    if(_unlocked_achievements.contains(std::string(name)))
    {
        return false;
    }

    if(!SteamUserStats()->SetAchievement(name.data()))
    {
        ssvu::lo("Steam") << "Failed to unlock achievement " << name << '\n';
        return false;
    }

    _unlocked_achievements.emplace(name);
    return store_stats();
}

bool steam_manager::set_rich_presence_in_menu()
{
    if(!_initialized)
    {
        return false;
    }

    return SteamFriends()->SetRichPresence("steam_display", "#InMenu");
}

bool steam_manager::set_rich_presence_in_game(
    std::string_view level_name_format, std::string_view difficulty_mult_format,
    std::string_view time_format)
{
    if(!_initialized)
    {
        return false;
    }

    return SteamFriends()->SetRichPresence(
               "levelname", level_name_format.data()) &&
           SteamFriends()->SetRichPresence(
               "difficultymult", difficulty_mult_format.data()) &&
           SteamFriends()->SetRichPresence("time", time_format.data()) &&
           SteamFriends()->SetRichPresence("steam_display", "#InGame");
}

bool steam_manager::set_and_store_stat(std::string_view name, int data)
{
    if(!_initialized)
    {
        return false;
    }

    // Steam API seems to be bugged, and always needs floats even for integer
    // stats.
    const float as_float = data;
    if(!SteamUserStats()->SetStat(name.data(), as_float))
    {
        ssvu::lo("Steam") << "Error setting stat '" << name << "' to '"
                          << as_float << "'\n";

        return false;
    }

    return store_stats();
}

[[nodiscard]] bool steam_manager::get_achievement(
    bool* out, std::string_view name)
{
    if(!_initialized)
    {
        return false;
    }

    if(!_got_stats)
    {
        return false;
    }

    if(!SteamUserStats()->GetAchievement(name.data(), out))
    {
        ssvu::lo("Steam") << "Error getting achievement " << name << '\n';
        return false;
    }

    return true;
}

[[nodiscard]] bool steam_manager::get_stat(int* out, std::string_view name)
{
    if(!_initialized)
    {
        return false;
    }

    if(!_got_stats)
    {
        return false;
    }

    // Steam API seems to be bugged, and always needs floats even for integer
    // stats.
    float as_float;
    if(!SteamUserStats()->GetStat(name.data(), &as_float))
    {
        ssvu::lo("Steam") << "Error getting stat " << name.data() << '\n';
        return false;
    }

    *out = as_float;
    return true;
}

bool steam_manager::update_hardcoded_achievement_cube_master()
{
    if(!_initialized)
    {
        return false;
    }

    if(!_got_stats)
    {
        return false;
    }

    const auto unlocked = [this](const char* name) -> int {
        bool res{false};
        const bool rc = get_achievement(&res, name);

        if(!rc)
        {
            return 0;
        }

        return res ? 1 : 0;
    };

    // "Cube Master"
    {
        int stat;
        const bool rc = get_stat(&stat, "s0_packprogress_cube");

        if(!rc)
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

        if(acc > stat)
        {
            if(!set_and_store_stat("s0_packprogress_cube", acc))
            {
                return false;
            }
        }
    }

    return true;
}

bool steam_manager::update_hardcoded_achievement_hypercube_master()
{
    if(!_initialized)
    {
        return false;
    }

    if(!_got_stats)
    {
        return false;
    }

    const auto unlocked = [this](const char* name) -> int {
        bool res{false};
        const bool rc = get_achievement(&res, name);

        if(!rc)
        {
            return 0;
        }

        return res ? 1 : 0;
    };

    // "Hypercube Master"
    {
        int stat;
        const bool rc = get_stat(&stat, "s1_packprogress_hypercube");

        if(!rc)
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

        if(acc > stat)
        {
            if(!set_and_store_stat("s1_packprogress_hypercube", acc))
            {
                return false;
            }
        }
    }

    return true;
}

bool steam_manager::update_hardcoded_achievements()
{
    bool success = true;

    if(!update_hardcoded_achievement_cube_master())
    {
        success = false;
    }

    if(!update_hardcoded_achievement_hypercube_master())
    {
        success = false;
    }

    return success;
}

void steam_manager::for_workshop_pack_folders(
    const std::function<void(const std::string&)>& f) const
{
    if(!_initialized)
    {
        return;
    }

    for(const std::string& s : _workshop_pack_folders)
    {
        f(s);
    }
}

} // namespace hg::Steam
