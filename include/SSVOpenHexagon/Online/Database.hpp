// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include <sqlite3.h>
#include <sqlite_orm.h>

#include <string>
#include <cstdint>
#include <optional>
#include <chrono>

namespace hg::Database
{

namespace Impl
{

inline auto makeStorage()
{
    using namespace sqlite_orm;

    auto storage = make_storage("ohdb.sqlite",                            //
                                                                          //
        make_table("users",                                               //
            make_column("id", &User::id, autoincrement(), primary_key()), //
            make_column("steamId", &User::steamId, unique()),             //
            make_column("name", &User::name),                             //
            make_column("passwordHash", &User::passwordHash)              //
            ),                                                            //
                                                                          //
        make_table("loginTokens",                                         //
            make_column(                                                  //
                "id", &LoginToken::id, autoincrement(), primary_key()),   //
            make_column("userId", &LoginToken::userId, unique()),         //
            make_column("timestamp", &LoginToken::timestamp),             //
            make_column("token", &LoginToken::token)                      //
            ),                                                            //
                                                                          //
        make_table("scores",                                              //
            make_column(                                                  //
                "id", &Score::id, autoincrement(), primary_key()),        //
            make_column("levelValidator", &Score::levelValidator),        //
            make_column("timestamp", &Score::timestamp),                  //
            make_column("userSteamId", &Score::userSteamId),              //
            make_column("value", &Score::value)                           //
            )                                                             //
        //
    );

    storage.sync_schema(true /* preserve */);
    return storage;
}

inline auto& getStorage()
{
    static auto storage = makeStorage();
    return storage;
}

} // namespace Impl

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

[[nodiscard]] inline std::uint64_t timestamp(const TimePoint tp)
{
    return std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch())
        .count();
}

[[nodiscard]] inline std::uint64_t nowTimestamp()
{
    return timestamp(Clock::now());
}

[[nodiscard]] inline TimePoint toTimepoint(const std::uint64_t timestamp)
{
    return TimePoint{} + std::chrono::seconds(timestamp);
}

void addUser(const User& user);

void removeUser(const std::uint32_t id);

void dumpUsers();

[[nodiscard]] bool anyUserWithSteamId(const std::uint64_t steamId);

[[nodiscard]] bool anyUserWithName(const std::string& name);

[[nodiscard]] std::optional<User> getUserWithSteamIdAndName(
    const std::uint64_t steamId, const std::string& name);

void removeAllLoginTokensForUser(const std::uint32_t userId);

void addLoginToken(const LoginToken& loginToken);

[[nodiscard]] std::vector<User> getAllUsersWithSteamId(
    const std::uint64_t steamId);

[[nodiscard]] std::optional<User> getUserWithSteamId(
    const std::uint64_t steamId);

[[nodiscard]] std::vector<LoginToken> getAllStaleLoginTokens();
void removeAllStaleLoginTokens();

[[nodiscard]] std::vector<ProcessedScore> getTopScores(
    const int topLimit, const std::string& levelValidator);

[[nodiscard]] bool isLoginTokenValid(std::uint64_t token);

void addScore(const std::string& levelValidator, const std::uint64_t timestamp,
    const std::uint64_t userSteamId, const double value);

[[nodiscard]] std::optional<ProcessedScore> getScore(
    const std::string& levelValidator, const std::uint64_t userSteamId);

} // namespace hg::Database
