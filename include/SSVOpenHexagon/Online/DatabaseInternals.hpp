// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// Internal database record types backed by sqlite_orm. sqlite_orm requires
// `std::string` and `std::vector` field types for its column traits, so these
// stay decoupled from the public `DatabaseRecords.hpp` (which is included by
// HexagonClient/HexagonServer). Include this header only from
// `src/SSVOpenHexagon/Online/Database.cpp`.

#include "SFML/Base/IntTypes.hpp"

#include <string>
#include <vector>

namespace hg::Database
{

struct User
{
    sf::base::U32     id;
    sf::base::U64     steamId;
    std::string       name;
    std::vector<char> passwordHash;
};

struct LoginToken
{
    sf::base::U32 id;
    sf::base::U32 userId;
    sf::base::U64 timestamp;
    sf::base::U64 token;
};

struct Score
{
    sf::base::U32 id;
    std::string   levelValidator;
    sf::base::U64 timestamp;
    sf::base::U64 userSteamId;
    double        value;
};

} // namespace hg::Database
