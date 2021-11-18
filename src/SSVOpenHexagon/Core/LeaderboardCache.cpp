// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/LeaderboardCache.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <chrono>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace hg {

void LeaderboardCache::receivedScores(const std::string& levelValidator,
    const std::vector<Database::ProcessedScore>& scores)
{
    CachedScores& cs = _levelValidatorToScores[levelValidator];
    cs._scores = scores;
    cs._cacheTime = HRClock::now();
}

void LeaderboardCache::receivedOwnScore(
    const std::string& levelValidator, const Database::ProcessedScore& score)
{
    CachedScores& cs = _levelValidatorToScores[levelValidator];
    cs._ownScore = score;
    cs._cacheTime = HRClock::now();
}

void LeaderboardCache::requestedScores(const std::string& levelValidator)
{
    _levelValidatorToScores[levelValidator]._cacheTime = HRClock::now();
}

[[nodiscard]] bool LeaderboardCache::shouldRequestScores(
    const std::string& levelValidator) const
{
    const auto it = _levelValidatorToScores.find(levelValidator);
    if(it == _levelValidatorToScores.end())
    {
        return true;
    }

    const CachedScores& cs = it->second;

    return (HRClock::now() - cs._cacheTime) > std::chrono::seconds(6);
}

[[nodiscard]] const std::vector<Database::ProcessedScore>&
LeaderboardCache::getScores(const std::string& levelValidator) const
{
    SSVOH_ASSERT(hasInformation(levelValidator));
    return _levelValidatorToScores.at(levelValidator)._scores;
}

[[nodiscard]] const Database::ProcessedScore* LeaderboardCache::getOwnScore(
    const std::string& levelValidator) const
{
    SSVOH_ASSERT(hasInformation(levelValidator));

    const auto& os = _levelValidatorToScores.at(levelValidator)._ownScore;
    return os.has_value() ? &*os : nullptr;
}

[[nodiscard]] bool LeaderboardCache::hasInformation(
    const std::string& levelValidator) const
{
    return _levelValidatorToScores.find(levelValidator) !=
           _levelValidatorToScores.end();
}

} // namespace hg
