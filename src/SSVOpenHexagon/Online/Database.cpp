// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Database.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include "SSVOpenHexagon/Utils/Concat.hpp"

static auto& dlog(const char* funcName)
{
    return ::ssvu::lo(::hg::Utils::concat("hg::Database::", funcName));
}

#define SSVOH_DLOG ::dlog(__func__)

#define SSVOH_DLOG_VERBOSE \
    if(_verbose) ::dlog(__func__)

#define SSVOH_DLOG_ERROR ::dlog(__func__) << "[ERROR] "

#define SSVOH_DLOG_VAR(x) '\'' << #x << "': '" << x << '\''

namespace hg::Database
{

void addUser(const User& user)
{
    const int id = Impl::getStorage().insert(user);

    SSVOH_DLOG << "Added user with id '" << id << "' to storage:\n"
               << Impl::getStorage().dump(user) << '\n';
}

void removeUser(const std::uint32_t id)
{
    Impl::getStorage().remove<User>(id);

    SSVOH_DLOG << "Removed user with id '" << id << "' from storage\n";
}

void dumpUsers()
{
    SSVOH_DLOG << "Dumping all users\n";

    const auto users = Impl::getStorage().get_all<User>();

    SSVOH_DLOG << "users (" << users.size() << "):\n";

    for(const auto& user : users)
    {
        SSVOH_DLOG << Impl::getStorage().dump(user) << '\n';
    }
}

} // namespace hg::Database
