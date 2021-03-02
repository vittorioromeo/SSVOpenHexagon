// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"

#include <string>

namespace hg
{

void ProfileData::checkFavoriteLevelsHealth(HGAssets& mAssets)
{
    // Remove invalid levelIDs that might have been added to the files.
    for(const std::string& favID : favoriteLevelsDataIDs)
    {
        if(!mAssets.checkLevelIDValidity(favID))
        {
            removeFavoriteLevel(favID);
        }
    }
}

} // namespace hg
