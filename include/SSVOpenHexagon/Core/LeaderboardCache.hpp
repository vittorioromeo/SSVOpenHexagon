// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"

#include <string>
#include <unordered_map>

namespace hg
{

class LeaderboardCache
{
private:
    struct CachedScores
    {
        sf::base::Vector<Database::ProcessedScore>   _scores;
        sf::base::Optional<Database::ProcessedScore> _ownScore;
        HRTimePoint                                  _cacheTime;
    };

    std::unordered_map<std::string, CachedScores> _levelValidatorToScores;

public:
    void receivedScores(const std::string& levelValidator, const sf::base::Vector<Database::ProcessedScore>& scores);

    void receivedOwnScore(const std::string& levelValidator, const Database::ProcessedScore& score);

    void requestedScores(const std::string& levelValidator);

    [[nodiscard]] bool shouldRequestScores(const std::string& levelValidator) const;

    [[nodiscard]] const sf::base::Vector<Database::ProcessedScore>& getScores(const std::string& levelValidator) const;

    [[nodiscard]] const Database::ProcessedScore* getOwnScore(const std::string& levelValidator) const;

    [[nodiscard]] bool getSupported(const std::string& levelValidator) const;
    [[nodiscard]] bool hasInformation(const std::string& levelValidator) const;
};

} // namespace hg
