// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/LeaderboardCache.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <SFML/Base/IntTypes.hpp>
#include <unordered_map>
#include <unordered_set>

namespace hg
{

void LeaderboardCache::receivedScores(const sf::base::String&                           levelValidator,
                                      const sf::base::Vector<Database::ProcessedScore>& scores)
{
    CachedScores& cs = _levelValidatorToScores[levelValidator];
    cs._scores       = scores;
    cs._cacheTime    = HRClock::now();
    cs._received     = true;
}

void LeaderboardCache::receivedOwnScore(const sf::base::String& levelValidator, const Database::ProcessedScore& score)
{
    CachedScores& cs = _levelValidatorToScores[levelValidator];
    cs._ownScore.emplace(score);
    cs._cacheTime = HRClock::now();
}

void LeaderboardCache::requestedScores(const sf::base::String& levelValidator)
{
    _levelValidatorToScores[levelValidator]._cacheTime = HRClock::now();
}

[[nodiscard]] bool LeaderboardCache::shouldRequestScores(const sf::base::String& levelValidator) const
{
    const auto it = _levelValidatorToScores.find(levelValidator);
    if (it == _levelValidatorToScores.end())
    {
        return true;
    }

    const CachedScores& cs = it->second;

    return (HRClock::now() - cs._cacheTime) > std::chrono::seconds(6);
}

[[nodiscard]] const sf::base::Vector<Database::ProcessedScore>& LeaderboardCache::getScores(
    const sf::base::String& levelValidator) const
{
    SSVOH_ASSERT(hasInformation(levelValidator));
    return _levelValidatorToScores.at(levelValidator)._scores;
}

[[nodiscard]] const Database::ProcessedScore* LeaderboardCache::getOwnScore(const sf::base::String& levelValidator) const
{
    SSVOH_ASSERT(hasInformation(levelValidator));

    const auto& os = _levelValidatorToScores.at(levelValidator)._ownScore;
    return os.hasValue() ? &*os : nullptr;
}

[[nodiscard]] bool LeaderboardCache::hasInformation(const sf::base::String& levelValidator) const
{
    return _levelValidatorToScores.find(levelValidator) != _levelValidatorToScores.end();
}

[[nodiscard]] bool LeaderboardCache::hasReceivedScores(const sf::base::String& levelValidator) const
{
    const auto it = _levelValidatorToScores.find(levelValidator);
    return it != _levelValidatorToScores.end() && it->second._received;
}

void LeaderboardCache::markReplayUnavailable(const sf::base::String& levelValidator, const sf::base::U64 scoreTimestamp)
{
    _levelValidatorToScores[levelValidator]._unavailableTimestamps.insert(scoreTimestamp);
}

const std::unordered_set<sf::base::U64> LeaderboardCache::kEmptyUnavailable;

[[nodiscard]] const std::unordered_set<sf::base::U64>& LeaderboardCache::getUnavailableTimestamps(
    const sf::base::String& levelValidator) const
{
    const auto it = _levelValidatorToScores.find(levelValidator);
    return (it == _levelValidatorToScores.end()) ? kEmptyUnavailable : it->second._unavailableTimestamps;
}

} // namespace hg
