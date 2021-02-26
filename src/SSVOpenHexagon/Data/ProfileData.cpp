// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"

#include <string>

namespace hg
{

void ProfileData::addFavoriteLevel(const std::string& mLevelID)
{
    // Add the level to the favorites keeping them sorted
    // in alphabetical order.
    auto it{favoriteLevelsDataIDs.begin()};
    const auto end{favoriteLevelsDataIDs.end()};
    const std::string tweakedFavName{
        ssvu::toLower(assets.getLevelData(mLevelID).name)};
    std::string tweakedLevelName;

    while(it != end)
    {
        tweakedLevelName = ssvu::toLower(assets.getLevelData(*it).name);
        if(tweakedLevelName > tweakedFavName)
        {
            break;
        }
        ++it;
    }
    if(it == end)
    {
        favoriteLevelsDataIDs.emplace_back(mLevelID);
    }
    else
    {
        favoriteLevelsDataIDs.insert(it, mLevelID);
    }
}

void ProfileData::removeFavoriteLevel(const std::string& mLevelID)
{
    favoriteLevelsDataIDs.erase(std::find(
        favoriteLevelsDataIDs.begin(), favoriteLevelsDataIDs.end(), mLevelID));
}

void ProfileData::checkFavoriteLevelsHealth()
{
    // Remove invalid levelIDs that might have been added to the files.
    for(const std::string& favID : favoriteLevelsDataIDs)
    {
        if(!assets.checkLevelIDValidity(favID))
        {
            removeFavoriteLevel(favID);
        }
    }

    std::sort(favoriteLevelsDataIDs.begin(), favoriteLevelsDataIDs.end(),
        [this](const std::string& a, const std::string& b) -> bool {
            return ssvu::toLower(assets.getLevelData(a).name) <
                    ssvu::toLower(assets.getLevelData(b).name);
        });
}

} // namespace hg