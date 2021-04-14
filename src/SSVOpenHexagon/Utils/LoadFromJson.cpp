// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LoadFromJson.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"

namespace hg::Utils {

[[nodiscard]] MusicData loadMusicFromJson(const ssvuj::Obj& mRoot)
{
    MusicData result{ssvuj::getExtr<std::string>(mRoot, "id"),
        ssvuj::getExtr<std::string>(mRoot, "file_name"),
        ssvuj::getExtr<std::string>(mRoot, "name"),
        ssvuj::getExtr<std::string>(mRoot, "album"),
        ssvuj::getExtr<std::string>(mRoot, "author")};

    for(const auto& segment : ssvuj::getObj(mRoot, "segments"))
    {
        result.addSegment(ssvuj::getExtr<float>(segment, "time"),
            ssvuj::getExtr<float>(segment, "beat_pulse_delay_offset", 0.f));
    }

    return result;
}

[[nodiscard]] GameVersion loadVersionFromJson(const ssvuj::Obj& mRoot)
{
    return {ssvuj::getExtr<int>(mRoot, "major"),
        ssvuj::getExtr<int>(mRoot, "minor"),
        ssvuj::getExtr<int>(mRoot, "micro")};
}

[[nodiscard]] ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot)
{
    const GameVersion version =
        ssvuj::isObj("version")
            ? loadVersionFromJson(ssvuj::getObj(mRoot, "version"))
            : GameVersion{-1, 0, 0};

    return {version, ssvuj::getExtr<std::string>(mRoot, "name"),
        ssvuj::getExtr<std::unordered_map<std::string, float>>(mRoot, "scores"),
        ssvuj::getExtr<std::vector<std::string>>(mRoot, "favorites", {})};
}

} // namespace hg::Utils
