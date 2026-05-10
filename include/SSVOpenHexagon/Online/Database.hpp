// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/DatabaseInternals.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


// TODO (P2): remove reliance on steam ID for future platforms

namespace hg::Database
{

void addUser(const User& user);

void removeUser(const sf::base::U32 id);

void dumpUsers();

[[nodiscard]] bool anyUserWithSteamId(const sf::base::U64 steamId);

[[nodiscard]] bool anyUserWithName(const sf::base::String& name);

[[nodiscard]] sf::base::Optional<User> getUserWithSteamIdAndName(const sf::base::U64 steamId, const sf::base::String& name);

void removeAllLoginTokensForUser(const sf::base::U32 userId);

void addLoginToken(const LoginToken& loginToken);

[[nodiscard]] sf::base::Vector<User> getAllUsersWithSteamId(const sf::base::U64 steamId);

[[nodiscard]] sf::base::Optional<User> getUserWithSteamId(const sf::base::U64 steamId);

[[nodiscard]] sf::base::Vector<LoginToken> getAllStaleLoginTokens();
void                                       removeAllStaleLoginTokens();

[[nodiscard]] sf::base::Vector<ProcessedScore> getTopScores(const int topLimit, const sf::base::String& levelValidator);

[[nodiscard]] bool isLoginTokenValid(sf::base::U64 token);

// Callers (server-side replay persistence in particular) read this to
// decide whether to write side data like the replay file -- on `Skipped`
// the disk file would point at a row that's already been beaten.
enum class AddScoreOutcome : sf::base::U8
{
    Inserted = 0, //!< no prior row for `(validator, steamId)`; new row written
    Upserted = 1, //!< prior row existed but was beaten; row updated in place
    Skipped  = 2, //!< existing score is >= new value; no DB write
};

[[nodiscard]] AddScoreOutcome addScore(const sf::base::String& levelValidator,
                                       const sf::base::U64     timestamp,
                                       const sf::base::U64     userSteamId,
                                       const double            value);

[[nodiscard]] sf::base::Optional<ProcessedScore> getScore(const sf::base::String& levelValidator,
                                                          const sf::base::U64     userSteamId);

[[nodiscard]] sf::base::Optional<sf::base::String> execute(const sf::base::String& query);

} // namespace hg::Database
