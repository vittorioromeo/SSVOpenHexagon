// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/LevelValidator.hpp"

#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


namespace hg
{

LevelData::LevelData(const ssvuj::Obj& mRoot, const sf::base::String& mPackPath, const sf::base::String& mPackId) :
    packPath{mPackPath},
    packId{mPackId},
    id{ssvuj::getExtr<sf::base::String>(mRoot, "id", "nullId")},
    name{ssvuj::getExtr<sf::base::String>(mRoot, "name", "nullName")},
    description{ssvuj::getExtr<sf::base::String>(mRoot, "description", "")},
    author{ssvuj::getExtr<sf::base::String>(mRoot, "author", "")},
    menuPriority{ssvuj::getExtr<int>(mRoot, "menuPriority", 0)},
    selectable{ssvuj::getExtr<bool>(mRoot, "selectable", true)},
    musicId{ssvuj::getExtr<sf::base::String>(mRoot, "musicId", "nullMusicId")},
    soundId{ssvuj::getExtr<sf::base::String>(mRoot, "soundId", "nullSoundId")},
    styleId{ssvuj::getExtr<sf::base::String>(mRoot, "styleId", "nullStyleId")},
    luaScriptPath{packPath + ssvuj::getExtr<sf::base::String>(mRoot, "luaFile", "nullLuaPath")},
    difficultyMults{ssvuj::getExtr<sf::base::Vector<float>>(mRoot, "difficultyMults", {})},
    unscored{ssvuj::getExtr<bool>(mRoot, "unscored", false)}
{
    difficultyMults.emplaceBack(1.f);
    sf::base::quickSort(difficultyMults.begin(), difficultyMults.end());

    for (const float dm : difficultyMults)
    {
        const auto lvWithPackId    = Utils::getLevelValidator(Utils::concat(packId, '_', id), dm);
        const auto lvWithoutPackId = Utils::getLevelValidator(id, dm);

        // TODO
        validators[dm].assign(lvWithPackId.data(), lvWithPackId.size());

        // TODO
        validatorsWithoutPackId[dm].assign(lvWithoutPackId.data(), lvWithoutPackId.size());
    }
}

[[nodiscard]] float LevelData::getNthDiffMult(int index) const noexcept
{
    while (index < 0)
    {
        index += difficultyMults.size();
    }

    while (index >= static_cast<int>(difficultyMults.size()))
    {
        index -= difficultyMults.size();
    }

    return difficultyMults[index];
}

[[nodiscard]] const sf::base::String& LevelData::getValidator(const float diffMult) const
{
    SSVOH_ASSERT(validators.find(diffMult) != validators.end());
    return validators.at(diffMult);
}

[[nodiscard]] const sf::base::String& LevelData::getValidatorWithoutPackId(const float diffMult) const
{
    SSVOH_ASSERT(validatorsWithoutPackId.find(diffMult) != validatorsWithoutPackId.end());
    return validatorsWithoutPackId.at(diffMult);
}

} // namespace hg
