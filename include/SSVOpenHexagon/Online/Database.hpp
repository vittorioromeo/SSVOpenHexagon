// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <sqlite3.h>
#include <sqlite_orm.h>

#include <string>
#include <cstdint>

namespace hg::Database
{

struct User
{
    std::uint32_t id;
    std::uint64_t steamId;
    std::string name;
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
            make_column("steam_id", &User::steamId, unique()),            //
            make_column("name", &User::name)                              //
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

} // namespace hg::Database
