// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Database.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

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

[[nodiscard]] bool anyUserWithSteamId(const std::uint64_t steamId)
{
    using namespace sqlite_orm;

    auto query =
        Impl::getStorage().get_all<User>(where(steamId == c(&User::steamId)));

    return !query.empty();
}

[[nodiscard]] bool anyUserWithName(const std::string& name)
{
    using namespace sqlite_orm;

    auto query =
        Impl::getStorage().get_all<User>(where(name == c(&User::name)));

    return !query.empty();
}

[[nodiscard]] std::optional<User> getUserWithSteamIdAndName(
    const std::uint64_t steamId, const std::string& name)
{
    using namespace sqlite_orm;

    auto query = Impl::getStorage().get_all<User>(
        where(steamId == c(&User::steamId) && name == c(&User::name)));

    if(query.empty())
    {
        return std::nullopt;
    }

    if(query.size() > 1)
    {
        SSVOH_DLOG_ERROR
            << "Database integrity error, multiple users with same steamId '"
            << steamId << "' and name '" << name << "'\n";

        return std::nullopt;
    }

    SSVOH_ASSERT(query.size() == 1);
    return {query[0]};
}

} // namespace hg::Database
