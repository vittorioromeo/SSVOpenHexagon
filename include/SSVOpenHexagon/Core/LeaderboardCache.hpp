// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <optional>

namespace hg {

class LeaderboardCache
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

private:
    struct CachedScores
    {
        bool _supported{true};
        std::vector<Database::ProcessedScore> _scores;
        std::optional<Database::ProcessedScore> _ownScore;
        TimePoint _cacheTime;
    };

    std::unordered_map<std::string, CachedScores> _levelValidatorToScores;

public:
    void receivedScores(const std::string& levelValidator,
        const std::vector<Database::ProcessedScore>& scores);

    void receivedOwnScore(const std::string& levelValidator,
        const Database::ProcessedScore& score);

    void receivedScoresUnsupported(const std::string& levelValidator);

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
