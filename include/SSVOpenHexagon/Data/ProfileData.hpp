// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <unordered_set>

namespace hg
{

class HGAssets;

class ProfileData
{
private:
    HGAssets& assets;
    GameVersion version;
    std::string name;
    ssvuj::Obj scores;
    std::vector<std::string> trackedNames;
    std::unordered_set<std::string> favoriteLevelsDataIDs;

    void checkFavoriteLevelsHealth();

public:
    ProfileData(HGAssets& mAssets, const GameVersion mVersion,
        const std::string& mName, const ssvuj::Obj& mScores,
        const std::vector<std::string>& mTrackedNames,
        const std::vector<std::string>& mFavorites)
        : assets{mAssets}, version{mVersion}, name{mName}, scores{mScores},
          trackedNames{mTrackedNames}
    {
        std::copy(mFavorites.begin(), mFavorites.end(),
            std::inserter(favoriteLevelsDataIDs, favoriteLevelsDataIDs.end()));
        checkFavoriteLevelsHealth();
    }

    [[nodiscard]] constexpr GameVersion getVersion() const noexcept
    {
        return version;
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
        return name;
    }

    [[nodiscard]] const ssvuj::Obj& getScores() const noexcept
    {
        return scores;
    }

    [[nodiscard]] const std::vector<std::string>&
    getTrackedNames() const noexcept
    {
        return trackedNames;
    }

    [[nodiscard]] const std::unordered_set<std::string>&
    getFavoriteLevelIds() const noexcept
    {
        return favoriteLevelsDataIDs;
    }

    void setScore(const std::string& mId, float mScore)
    {
        ssvuj::arch(scores, mId, mScore);
    }

    [[nodiscard]] float getScore(const std::string& mId) const
    {
        return ssvuj::getExtr<float>(scores, mId);
    }

    void addTrackedName(const std::string& mTrackedName)
    {
        trackedNames.emplace_back(ssvu::toLower(mTrackedName));
    }

    void clearTrackedNames()
    {
        trackedNames.clear();
    }

    void addFavoriteLevel(const std::string& mLevelID)
    {
        favoriteLevelsDataIDs.insert(mLevelID);
    }
    void removeFavoriteLevel(const std::string& mLevelID)
    {
        favoriteLevelsDataIDs.erase(mLevelID);
    }

    [[nodiscard]] bool isLevelFavorite(const std::string& mLevelID)
    {
        return favoriteLevelsDataIDs.find(mLevelID) != favoriteLevelsDataIDs.end();
    }
};

} // namespace hg
