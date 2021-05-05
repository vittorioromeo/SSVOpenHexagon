// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/UniquePtr.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <string>

namespace hg::Steam {

class steam_manager
{
private:
    class steam_manager_impl;

    Utils::UniquePtr<steam_manager_impl> _impl;

    [[nodiscard]] const steam_manager_impl& impl() const noexcept;
    [[nodiscard]] steam_manager_impl& impl() noexcept;

public:
    explicit steam_manager();
    ~steam_manager();

    steam_manager(const steam_manager&) = delete;
    steam_manager& operator=(const steam_manager&) = delete;

    steam_manager(steam_manager&&) = delete;
    steam_manager& operator=(steam_manager&&) = delete;

    [[nodiscard]] bool is_initialized() const noexcept;

    bool request_stats_and_achievements();

    bool run_callbacks();

    bool store_stats();
    bool unlock_achievement(std::string_view name);

    bool set_rich_presence_in_menu();
    bool set_rich_presence_in_game(std::string_view level_name_format,
        std::string_view difficulty_mult_format, std::string_view time_format);

    bool set_and_store_stat(std::string_view name, int data);
    [[nodiscard]] bool get_achievement(bool* out, std::string_view name);
    [[nodiscard]] bool get_stat(int* out, std::string_view name);

    bool update_hardcoded_achievements();

    void for_workshop_pack_folders(
        const std::function<void(const std::string&)>& f) const;

    bool request_encrypted_app_ticket();

    [[nodiscard]] bool got_encrypted_app_ticket_response() const noexcept;

    [[nodiscard]] bool got_encrypted_app_ticket() const noexcept;

    [[nodiscard]] std::optional<std::uint64_t>
    get_ticket_steam_id() const noexcept;
};

} // namespace hg::Steam
