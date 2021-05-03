// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/LevelData.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Utils/LevelValidator.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

#include <string>
#include <vector>
#include <set>

namespace hg {

LevelData::LevelData(const ssvuj::Obj& mRoot, const std::string& mPackPath,
    const std::string& mPackId)
    : packPath{mPackPath},
      packId{mPackId},
      id{ssvuj::getExtr<std::string>(mRoot, "id", "nullId")},
      name{ssvuj::getExtr<std::string>(mRoot, "name", "nullName")},
      description{ssvuj::getExtr<std::string>(mRoot, "description", "")},
      author{ssvuj::getExtr<std::string>(mRoot, "author", "")},
      menuPriority{ssvuj::getExtr<int>(mRoot, "menuPriority", 0)},
      selectable{ssvuj::getExtr<bool>(mRoot, "selectable", true)},
      musicId{ssvuj::getExtr<std::string>(mRoot, "musicId", "nullMusicId")},
      soundId{ssvuj::getExtr<std::string>(mRoot, "soundId", "nullSoundId")},
      styleId{ssvuj::getExtr<std::string>(mRoot, "styleId", "nullStyleId")},
      luaScriptPath{packPath + ssvuj::getExtr<std::string>(
                                   mRoot, "luaFile", "nullLuaPath")},
      difficultyMults{
          ssvuj::getExtr<std::vector<float>>(mRoot, "difficultyMults", {})},
      unscored{ssvuj::getExtr<bool>(mRoot, "unscored", false)}
{
    difficultyMults.emplace_back(1.f);
    std::sort(difficultyMults.begin(), difficultyMults.end());

    for(const float dm : difficultyMults)
    {
        validators[dm] =
            Utils::getLevelValidator(Utils::concat(packId, '_', id), dm);

        validatorsWithoutPackId[dm] = Utils::getLevelValidator(id, dm);
    }
}

[[nodiscard]] float LevelData::getNthDiffMult(int index) const noexcept
{
    while(index < 0)
    {
        index += difficultyMults.size();
    }

    while(index >= static_cast<int>(difficultyMults.size()))
    {
        index -= difficultyMults.size();
    }

    return difficultyMults.at(index);
}

[[nodiscard]] const std::string& LevelData::getValidator(
    const float diffMult) const
{
    SSVOH_ASSERT(validators.find(diffMult) != validators.end());
    return validators.at(diffMult);
}

[[nodiscard]] const std::string& LevelData::getValidatorWithoutPackId(
    const float diffMult) const
{
    SSVOH_ASSERT(validatorsWithoutPackId.find(diffMult) !=
                 validatorsWithoutPackId.end());
    return validatorsWithoutPackId.at(diffMult);
}

} // namespace hg
