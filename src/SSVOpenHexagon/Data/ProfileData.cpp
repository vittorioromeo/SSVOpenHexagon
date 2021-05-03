// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ProfileData.hpp"

#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Utils/VectorToSet.hpp"

#include <SSVUtils/Core/String/Utils.hpp>

#include <unordered_set>
#include <string>

namespace hg {

ProfileData::ProfileData(const GameVersion mVersion, const std::string& mName,
    const std::unordered_map<std::string, float>& mScores,
    const std::vector<std::string>& mFavorites)
    : version{mVersion},
      name{mName},
      scores{mScores},
      favoriteLevelsDataIDs{Utils::toUnorderedSet(mFavorites)}
{}

[[nodiscard]] GameVersion ProfileData::getVersion() const noexcept
{
    return version;
}

[[nodiscard]] const std::string& ProfileData::getName() const noexcept
{
    return name;
}

[[nodiscard]] const std::unordered_map<std::string, float>&
ProfileData::getScores() const noexcept
{
    return scores;
}

[[nodiscard]] std::unordered_set<std::string>&
ProfileData::getFavoriteLevelIds() noexcept
{
    return favoriteLevelsDataIDs;
}

[[nodiscard]] const std::unordered_set<std::string>&
ProfileData::getFavoriteLevelIds() const noexcept
{
    return favoriteLevelsDataIDs;
}

void ProfileData::setScore(const std::string& mId, const float mScore)
{
    scores[mId] = mScore;
}

[[nodiscard]] float ProfileData::getScore(const std::string& mId) const
{
    const auto it = scores.find(mId);

    if(it == scores.end())
    {
        return 0.f;
    }

    return it->second;
}

void ProfileData::addFavoriteLevel(const std::string& mLevelID)
{
    favoriteLevelsDataIDs.emplace(mLevelID);
}

void ProfileData::removeFavoriteLevel(const std::string& mLevelID)
{
    favoriteLevelsDataIDs.erase(mLevelID);
}

[[nodiscard]] bool ProfileData::isLevelFavorite(
    const std::string& mLevelID) const noexcept
{
    return favoriteLevelsDataIDs.find(mLevelID) != favoriteLevelsDataIDs.end();
}

} // namespace hg
