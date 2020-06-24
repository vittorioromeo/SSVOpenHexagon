// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"

#include "steam/steam_api.h"

#include <string_view>

namespace hg::Steam
{

class steam_manager
{
private:
    bool _initialized;
    bool _got_stats;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    STEAM_CALLBACK(steam_manager, on_user_stats_received, UserStatsReceived_t);
    STEAM_CALLBACK(steam_manager, on_user_stats_stored, UserStatsStored_t);
    STEAM_CALLBACK(
        steam_manager, on_user_achievement_stored, UserAchievementStored_t);
#pragma GCC diagnostic pop

    bool store_stats();

public:
    steam_manager();
    ~steam_manager();

    steam_manager(const steam_manager&) = delete;
    steam_manager& operator=(const steam_manager&) = delete;

    steam_manager(steam_manager&&) = delete;
    steam_manager& operator=(steam_manager&&) = delete;

    bool request_stats_and_achievements();

    bool run_callbacks();

    bool unlock_achievement(std::string_view name);
};

} // namespace hg::Steam
