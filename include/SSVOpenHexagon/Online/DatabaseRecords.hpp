// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <vector>

#include <cstdint>

namespace hg::Database {

struct User
{
    std::uint32_t id;
    std::uint64_t steamId;
    std::string name;
    std::vector<char> passwordHash;
};

struct LoginToken
{
    std::uint32_t id;
    std::uint32_t userId;
    std::uint64_t timestamp;
    std::uint64_t token;
};

struct Score
{
    std::uint32_t id;
    std::string levelValidator;
    std::uint64_t timestamp;
    std::uint64_t userSteamId;
    double value;
};

struct ProcessedScore // not stored in database
{
    std::uint32_t position;
    std::string userName;
    std::uint64_t scoreTimestamp;
    double scoreValue;
};

} // namespace hg::Database
