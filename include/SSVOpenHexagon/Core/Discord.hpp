// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>

namespace discord {
class Core;
}

namespace hg::Discord {

class discord_manager
{
private:
    discord::Core* _core;
    bool _initialized;

public:
    explicit discord_manager();
    ~discord_manager();

    discord_manager(const discord_manager&) = delete;
    discord_manager& operator=(const discord_manager&) = delete;

    discord_manager(discord_manager&&) = delete;
    discord_manager& operator=(discord_manager&&) = delete;

    bool run_callbacks();

    bool set_rich_presence_in_menu();
    bool set_rich_presence_on_replay();
    bool set_rich_presence_in_game(const std::string& level_info,
        const std::string& second_info, bool dead = false);
};

} // namespace hg::Discord
