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

} // namespace hg::Database
