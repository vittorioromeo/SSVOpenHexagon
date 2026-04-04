// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

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

struct ProcessedScore // not stored in database
{
    sf::base::U32 position;
    std::string   userName;
    sf::base::U64 scoreTimestamp;
    double        scoreValue;
};

} // namespace hg::Database
