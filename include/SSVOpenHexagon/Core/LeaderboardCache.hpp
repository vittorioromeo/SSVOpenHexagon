// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include "SSVOpenHexagon/Utils/Clock.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <optional>

namespace hg {

class LeaderboardCache
{
private:
    struct CachedScores
    {
        std::vector<Database::ProcessedScore> _scores;
        std::optional<Database::ProcessedScore> _ownScore;
        HRTimePoint _cacheTime;
    };

    std::unordered_map<std::string, CachedScores> _levelValidatorToScores;

public:
    void receivedScores(const std::string& levelValidator,
        const std::vector<Database::ProcessedScore>& scores);

    void receivedOwnScore(const std::string& levelValidator,
        const Database::ProcessedScore& score);

    void requestedScores(const std::string& levelValidator);

    [[nodiscard]] bool shouldRequestScores(
        const std::string& levelValidator) const;

    [[nodiscard]] const std::vector<Database::ProcessedScore>& getScores(
        const std::string& levelValidator) const;

    [[nodiscard]] const Database::ProcessedScore* getOwnScore(
        const std::string& levelValidator) const;

    [[nodiscard]] bool getSupported(const std::string& levelValidator) const;
    [[nodiscard]] bool hasInformation(const std::string& levelValidator) const;
};

} // namespace hg
