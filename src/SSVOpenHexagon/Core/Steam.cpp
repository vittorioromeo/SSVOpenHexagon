// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Steam.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include "steam/steam_api.h"

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
}

void steam_manager::on_user_stats_stored(UserStatsStored_t* data)
{
    (void)data;
}

void steam_manager::on_user_achievement_stored(UserAchievementStored_t* data)
{
    (void)data;
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
        ssvu::lo("Steam") << "Attempted to unlock achievement without stats\n";
        return false;
    }

    if(!SteamUserStats()->StoreStats())
    {
        ssvu::lo("Steam") << "Failed to store stats\n";
        return false;
    }

    return true;
}

steam_manager::steam_manager()
    : _initialized{initialize_steamworks()}, _got_stats{false}
{
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
    _got_stats = true;
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

    if(!SteamUserStats()->SetAchievement(name.data()))
    {
        ssvu::lo("Steam") << "Failed to unlock achievement " << name << '\n';
        return false;
    }

    return store_stats();
}

} // namespace hg::Steam
