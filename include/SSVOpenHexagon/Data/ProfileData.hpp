// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Version.hpp"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

namespace hg {

class ProfileData
{
private:
    GameVersion version;
    std::string name;
    std::unordered_map<std::string, float> scores;
    std::unordered_set<std::string> favoriteLevelsDataIDs;

public:
    ProfileData(const GameVersion mVersion, const std::string& mName,
        const std::unordered_map<std::string, float>& mScores,
        const std::vector<std::string>& mFavorites);

    [[nodiscard]] GameVersion getVersion() const noexcept;
    [[nodiscard]] const std::string& getName() const noexcept;
    [[nodiscard]] const std::unordered_map<std::string, float>&
    getScores() const noexcept;

    [[nodiscard]] std::unordered_set<std::string>&
    getFavoriteLevelIds() noexcept;

    [[nodiscard]] const std::unordered_set<std::string>&
    getFavoriteLevelIds() const noexcept;

    void setScore(const std::string& mId, const float mScore);
    [[nodiscard]] float getScore(const std::string& mId) const;

    void addFavoriteLevel(const std::string& mLevelID);
    void removeFavoriteLevel(const std::string& mLevelID);

    [[nodiscard]] bool isLevelFavorite(
        const std::string& mLevelID) const noexcept;
};

} // namespace hg
