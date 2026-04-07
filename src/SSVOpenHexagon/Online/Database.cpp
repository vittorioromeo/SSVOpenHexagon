// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Online/Database.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/Timestamp.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"

#include <sqlite3.h>
#include <sqlite_orm.h>
#include <string>


static auto& dlog(const char* funcName)
{
    return ::hg::lo(::hg::Utils::concat("hg::Database::", funcName));
}

#define SSVOH_DLOG ::dlog(__func__)

#define SSVOH_DLOG_VERBOSE \
    if (_verbose)          \
    ::dlog(__func__)

#define SSVOH_DLOG_ERROR ::dlog(__func__) << "[ERROR] "

#define SSVOH_DLOG_VAR(x) '\'' << #x << "': '" << x << '\''

namespace hg::Database
{

namespace Impl
{

inline auto makeStorage()
{
    using namespace sqlite_orm;

    auto storage = make_storage("ohdb.sqlite",                                                          //
                                                                                                        //
                                make_table("users",                                                     //
                                           make_column("id", &User::id, primary_key().autoincrement()), //
                                           make_column("steamId", &User::steamId, unique()),            //
                                           make_column("name", &User::name),                            //
                                           make_column("passwordHash", &User::passwordHash)             //
                                           ),                                                           //
                                                                                                        //
                                make_table("loginTokens",                                               //
                                           make_column(                                                 //
                                               "id",
                                               &LoginToken::id,
                                               primary_key().autoincrement()),                   //
                                           make_column("userId", &LoginToken::userId, unique()), //
                                           make_column("timestamp", &LoginToken::timestamp),     //
                                           make_column("token", &LoginToken::token)              //
                                           ),                                                    //
                                                                                                 //
                                make_table("scores",                                             //
                                           make_column(                                          //
                                               "id",
                                               &Score::id,
                                               primary_key().autoincrement()),                    //
                                           make_column("levelValidator", &Score::levelValidator), //
                                           make_column("timestamp", &Score::timestamp),           //
                                           make_column("userSteamId", &Score::userSteamId),       //
                                           make_column("value", &Score::value)                    //
                                           )                                                      //
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

void addUser(const User& user)
{
    const int id = Impl::getStorage().insert(user);

    SSVOH_DLOG << "Added user with id '" << id << "' to storage:\n" << Impl::getStorage().dump(user) << '\n';
}

void removeUser(const sf::base::U32 id)
{
    Impl::getStorage().remove<User>(id);

    SSVOH_DLOG << "Removed user with id '" << id << "' from storage\n";
}

void dumpUsers()
{
    SSVOH_DLOG << "Dumping all users\n";

    const auto users = Impl::getStorage().get_all<User>();

    SSVOH_DLOG << "users (" << users.size() << "):\n";

    for (const auto& user : users)
    {
        SSVOH_DLOG << Impl::getStorage().dump(user) << '\n';
    }
}

[[nodiscard]] bool anyUserWithSteamId(const sf::base::U64 steamId)
{
    return !getAllUsersWithSteamId(steamId).empty();
}

[[nodiscard]] bool anyUserWithName(const sf::base::String& name)
{
    using namespace sqlite_orm;

    const std::string nameStd(name.cStr());
    auto              query = Impl::getStorage().get_all<User>(where(nameStd == c(&User::name)));

    return !query.empty();
}

[[nodiscard]] sf::base::Optional<User> getUserWithSteamIdAndName(const sf::base::U64 steamId, const sf::base::String& name)
{
    using namespace sqlite_orm;

    const std::string nameStd(name.cStr());
    auto query = Impl::getStorage().get_all<User>(where(steamId == c(&User::steamId) && nameStd == c(&User::name)));

    if (query.empty())
    {
        return sf::base::nullOpt;
    }

    if (query.size() > 1)
    {
        SSVOH_DLOG_ERROR << "Database integrity error, multiple users with same steamId '" << steamId << "' and name '"
                         << name << "'\n";

        return sf::base::nullOpt;
    }

    SSVOH_ASSERT(query.size() == 1);
    return sf::base::makeOptional<User>(query[0]);
}

void removeAllLoginTokensForUser(const sf::base::U32 userId)
{
    using namespace sqlite_orm;

    Impl::getStorage().remove_all<LoginToken>(where(userId == c(&LoginToken::userId)));
}

void addLoginToken(const LoginToken& loginToken)
{
    const int id = Impl::getStorage().insert(loginToken);

    SSVOH_DLOG << "Added login token with id '" << id << "' to storage:\n"
               << Impl::getStorage().dump(loginToken) << '\n';
}

[[nodiscard]] sf::base::Vector<User> getAllUsersWithSteamId(const sf::base::U64 steamId)
{
    using namespace sqlite_orm;

    auto query = Impl::getStorage().get_all<User>(where(steamId == c(&User::steamId)));

    sf::base::Vector<User> result;
    result.reserve(query.size());
    for (auto& u : query)
        result.emplaceBack(std::move(u));
    return result;
}

[[nodiscard]] sf::base::Optional<User> getUserWithSteamId(const sf::base::U64 steamId)
{
    const auto query = getAllUsersWithSteamId(steamId);

    if (query.empty())
    {
        return sf::base::nullOpt;
    }

    if (query.size() > 1)
    {
        SSVOH_DLOG_ERROR << "Database integrity error, multiple users with same steamId '" << steamId << "'\n";

        return sf::base::nullOpt;
    }

    SSVOH_ASSERT(query.size() == 1);
    return sf::base::makeOptional<User>(query[0]);
}

constexpr int tokenValiditySeconds = 3600;

[[nodiscard]] static bool isLoginTokenTimestampValid(const LoginToken& lt)
{
    const Utils::SCTimePoint now = Utils::SCClock::now();

    return (now - Utils::toTimepoint(lt.timestamp)) < std::chrono::seconds(tokenValiditySeconds);
}

[[nodiscard]] sf::base::Vector<LoginToken> getAllStaleLoginTokens()
{
    using namespace sqlite_orm;

    auto query = Impl::getStorage().get_all<LoginToken>();

    query.erase(std::remove_if(query.begin(),
                               query.end(),
                               [&](const LoginToken& lt) { return isLoginTokenTimestampValid(lt); }),
                std::end(query));

    sf::base::Vector<LoginToken> result;
    result.reserve(query.size());
    for (auto& lt : query)
        result.emplaceBack(std::move(lt));
    return result;
}

void removeAllStaleLoginTokens()
{
    using namespace sqlite_orm;

    const auto staleTokens = getAllStaleLoginTokens();
    for (const LoginToken& lt : staleTokens)
    {
        Impl::getStorage().remove<LoginToken>(lt.id);
    }
}

[[nodiscard]] sf::base::Vector<ProcessedScore> getTopScores(const int topLimit, const sf::base::String& levelValidator)
{
    using namespace sqlite_orm;

    const std::string levelValidatorStd(levelValidator.cStr());
    auto              query = Impl::getStorage().select(columns(&User::name, &Score::timestamp, &Score::value),
                                                        join<Score>(on(c(&User::steamId) == &Score::userSteamId)),
                                                        where(levelValidatorStd == c(&Score::levelValidator)),
                                                        order_by(&Score::value).desc(),
                                                        limit(topLimit));

    sf::base::Vector<ProcessedScore> result;

    sf::base::U32 index = 0;
    for (const auto& row : query)
    {
        result.pushBack( //
            ProcessedScore{
                .position       = index,            //
                .userName       = std::get<0>(row), //
                .scoreTimestamp = std::get<1>(row), //
                .scoreValue     = std::get<2>(row), //
            });

        ++index;
    }

    return result;
}

[[nodiscard]] bool isLoginTokenValid(sf::base::U64 token)
{
    using namespace sqlite_orm;

    const auto query = Impl::getStorage().get_all<LoginToken>(where(token == c(&LoginToken::token)));

    if (query.empty() || query.size() > 1)
    {
        return false;
    }

    return isLoginTokenTimestampValid(query.at(0));
}

void addScore(const sf::base::String& levelValidator,
              const sf::base::U64     timestamp,
              const sf::base::U64     userSteamId,
              const double            value)
{
    using namespace sqlite_orm;

    Score score{
        .levelValidator = std::string(levelValidator.cStr()), //
        .timestamp      = timestamp,                          //
        .userSteamId    = userSteamId,                        //
        .value          = value                               //
    };

    const std::string levelValidatorStd(levelValidator.cStr());

    const auto query = Impl::getStorage().get_all<Score>(
        where(userSteamId == c(&Score::userSteamId) && levelValidatorStd == c(&Score::levelValidator)));

    if (query.empty())
    {
        const int id = Impl::getStorage().insert(score);

        SSVOH_DLOG << "Added score with id '" << id << "' to storage:\n" << Impl::getStorage().dump(score) << '\n';

        return;
    }

    const Score& existingScore = query.at(0);
    if (existingScore.value >= value)
    {
        return;
    }

    score.id = existingScore.id;

    Impl::getStorage().update(score);

    SSVOH_DLOG << "Updated score with id '" << score.id << "' to storage:\n" << Impl::getStorage().dump(score) << '\n';
}

[[nodiscard]] sf::base::Optional<ProcessedScore> getScore(const sf::base::String& levelValidator, const sf::base::U64 userSteamId)
{
    using namespace sqlite_orm;

    const std::string levelValidatorStd(levelValidator.cStr());
    const auto query = Impl::getStorage().select(columns(&User::name, &Score::timestamp, &Score::value, &Score::userSteamId),
                                                 join<Score>(on(c(&User::steamId) == &Score::userSteamId)),
                                                 where(levelValidatorStd == c(&Score::levelValidator)),
                                                 order_by(&Score::value).desc());

    if (query.empty())
    {
        return sf::base::nullOpt;
    }

    sf::base::U32 index = 0;
    for (const auto& row : query)
    {
        if (std::get<3>(row) == userSteamId)
        {
            return sf::base::makeOptional(ProcessedScore{
                .position       = index,            //
                .userName       = std::get<0>(row), //
                .scoreTimestamp = std::get<1>(row), //
                .scoreValue     = std::get<2>(row), //
            });
        }

        ++index;
    }

    return sf::base::nullOpt;
}

[[nodiscard]] sf::base::Optional<sf::base::String> execute(const sf::base::String& query)
{
    const auto callback = [](void* a_param, int argc, char** argv, char** column) -> int
    {
        (void)a_param;
        (void)column;

        for (int i = 0; i < argc; i++)
        {
            std::printf("%s,\t", argv[i]);
        }

        std::printf("\n");
        return 0;
    };

    sqlite3* db = Impl::getStorage().get_connection().get();

    char* error = nullptr;
    sqlite3_exec(db, query.cStr(), callback, nullptr, &error);

    if (error != nullptr)
    {
        SFML_BASE_SCOPE_GUARD({ sqlite3_free(error); });
        return sf::base::makeOptional<sf::base::String>(error);
    }

    return sf::base::nullOpt;
}

} // namespace hg::Database
