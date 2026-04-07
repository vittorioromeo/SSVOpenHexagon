// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Global/StringHash.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"
#include "SSVOpenHexagon/Utils/LoadFromJson.hpp"

#include "SFML/Base/String.hpp"

namespace hg::Utils
{

[[nodiscard]] MusicData loadMusicFromJson(const ssvuj::Obj& mRoot)
{
    MusicData result{ssvuj::getExtr<sf::base::String>(mRoot, "id"),
                     ssvuj::getExtr<sf::base::String>(mRoot, "file_name"),
                     ssvuj::getExtr<sf::base::String>(mRoot, "name"),
                     ssvuj::getExtr<sf::base::String>(mRoot, "album"),
                     ssvuj::getExtr<sf::base::String>(mRoot, "author")};

    for (const auto& segment : ssvuj::getObj(mRoot, "segments"))
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
    const GameVersion version = ssvuj::isObj("version") ? loadVersionFromJson(ssvuj::getObj(mRoot, "version"))
                                                        : GameVersion{-1, 0, 0};

    return {version,
            ssvuj::getExtr<sf::base::String>(mRoot, "name"),
            ssvuj::getExtr<std::unordered_map<sf::base::String, float>>(mRoot, "scores"),
            ssvuj::getExtr<sf::base::Vector<sf::base::String>>(mRoot, "favorites", {})};
}

} // namespace hg::Utils
