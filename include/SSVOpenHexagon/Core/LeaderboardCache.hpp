// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/StringHash.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

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

    std::unordered_map<sf::base::String, CachedScores> _levelValidatorToScores;

public:
    void receivedScores(const sf::base::String& levelValidator, const sf::base::Vector<Database::ProcessedScore>& scores);

    void receivedOwnScore(const sf::base::String& levelValidator, const Database::ProcessedScore& score);

    void requestedScores(const sf::base::String& levelValidator);

    [[nodiscard]] bool shouldRequestScores(const sf::base::String& levelValidator) const;

    [[nodiscard]] const sf::base::Vector<Database::ProcessedScore>& getScores(const sf::base::String& levelValidator) const;

    [[nodiscard]] const Database::ProcessedScore* getOwnScore(const sf::base::String& levelValidator) const;

    [[nodiscard]] bool getSupported(const sf::base::String& levelValidator) const;
    [[nodiscard]] bool hasInformation(const sf::base::String& levelValidator) const;
};

} // namespace hg
