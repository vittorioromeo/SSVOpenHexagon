// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Global/StringHash.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Utils/VectorToSet.hpp"

#include "SFML/Base/String.hpp"


namespace hg
{

ProfileData::ProfileData(const GameVersion                                  mVersion,
                         const sf::base::String&                            mName,
                         const std::unordered_map<sf::base::String, float>& mScores,
                         const sf::base::Vector<sf::base::String>&          mFavorites) :
    version{mVersion},
    name{mName},
    scores{mScores},
    favoriteLevelsDataIDs{Utils::toUnorderedSet(mFavorites)}
{
}

[[nodiscard]] GameVersion ProfileData::getVersion() const noexcept
{
    return version;
}

[[nodiscard]] const sf::base::String& ProfileData::getName() const noexcept
{
    return name;
}

[[nodiscard]] const std::unordered_map<sf::base::String, float>& ProfileData::getScores() const noexcept
{
    return scores;
}

[[nodiscard]] std::unordered_set<sf::base::String>& ProfileData::getFavoriteLevelIds() noexcept
{
    return favoriteLevelsDataIDs;
}

[[nodiscard]] const std::unordered_set<sf::base::String>& ProfileData::getFavoriteLevelIds() const noexcept
{
    return favoriteLevelsDataIDs;
}

void ProfileData::setScore(const sf::base::String& mId, const float mScore)
{
    scores[mId] = mScore;
}

[[nodiscard]] float ProfileData::getScore(const sf::base::String& mId) const
{
    const auto it = scores.find(mId);

    if (it == scores.end())
    {
        return 0.f;
    }

    return it->second;
}

void ProfileData::addFavoriteLevel(const sf::base::String& mLevelID)
{
    favoriteLevelsDataIDs.emplace(mLevelID);
}

void ProfileData::removeFavoriteLevel(const sf::base::String& mLevelID)
{
    favoriteLevelsDataIDs.erase(mLevelID);
}

[[nodiscard]] bool ProfileData::isLevelFavorite(const sf::base::String& mLevelID) const noexcept
{
    return favoriteLevelsDataIDs.find(mLevelID) != favoriteLevelsDataIDs.end();
}

[[nodiscard]] PerLevelState ProfileData::getPerLevelState(const sf::base::String& mLevelId) const noexcept
{
    const auto it = perLevelState.find(mLevelId);
    if (it == perLevelState.end())
    {
        return {};
    }
    return it->second;
}

[[nodiscard]] PerLevelState& ProfileData::getOrCreatePerLevelState(const sf::base::String& mLevelId)
{
    return perLevelState[mLevelId];
}

[[nodiscard]] const std::unordered_map<sf::base::String, PerLevelState>& ProfileData::getPerLevelStates() const noexcept
{
    return perLevelState;
}

} // namespace hg
