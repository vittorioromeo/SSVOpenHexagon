// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <unordered_set>
#include <string>

namespace hg
{

class ProfileData
{
private:
    GameVersion version;
    std::string name;
    ssvuj::Obj scores;
    std::vector<std::string> trackedNames;
    std::unordered_set<std::string> favoriteLevelsDataIDs;

public:
    ProfileData(const GameVersion mVersion, const std::string& mName,
        const ssvuj::Obj& mScores,
        const std::vector<std::string>& mTrackedNames,
        const std::vector<std::string>& mFavorites)
        : version{mVersion}, name{mName}, scores{mScores},
          trackedNames{mTrackedNames}, favoriteLevelsDataIDs{
                                           mFavorites.begin(), mFavorites.end()}
    {
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

    [[nodiscard]] std::unordered_set<std::string>&
    getFavoriteLevelIds() noexcept
    {
        return favoriteLevelsDataIDs;
    }

    [[nodiscard]] const std::unordered_set<std::string>&
    getFavoriteLevelIds() const noexcept
    {
        return favoriteLevelsDataIDs;
    }

    void setScore(const std::string& mId, const float mScore)
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
        favoriteLevelsDataIDs.emplace(mLevelID);
    }

    void removeFavoriteLevel(const std::string& mLevelID)
    {
        favoriteLevelsDataIDs.erase(mLevelID);
    }

    [[nodiscard]] bool isLevelFavorite(
        const std::string& mLevelID) const noexcept
    {
        return favoriteLevelsDataIDs.find(mLevelID) !=
               favoriteLevelsDataIDs.end();
    }
};

} // namespace hg
