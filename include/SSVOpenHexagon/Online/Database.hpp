// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <sqlite3.h>
#include <sqlite_orm.h>

#include <string>
#include <cstdint>
#include <optional>

namespace hg::Database
{

struct User
{
    std::uint32_t id;
    std::uint64_t steamId;
    std::string name;
    std::string passwordHash;
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

};

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

} // namespace hg::Database
