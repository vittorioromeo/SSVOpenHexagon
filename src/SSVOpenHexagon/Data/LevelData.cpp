// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/LevelData.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <string>
#include <vector>

namespace hg {

LevelData::LevelData(const ssvuj::Obj& mRoot, const ssvufs::Path& mPackPath,
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
    ssvu::sort(difficultyMults);
}

} // namespace hg
