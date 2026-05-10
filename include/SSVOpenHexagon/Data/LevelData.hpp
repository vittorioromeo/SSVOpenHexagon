// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/System/Path.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <unordered_map>

namespace Json
{
class Value;
}

namespace ssvuj
{
using Obj = Json::Value;
}

namespace hg
{

struct LevelData
{
    sf::Path         packPath;
    sf::base::String packId;

    sf::base::String                            id;
    sf::base::String                            name;
    sf::base::String                            description;
    sf::base::String                            author;
    int                                         menuPriority;
    bool                                        selectable;
    sf::base::String                            musicId;
    sf::base::String                            soundId;
    sf::base::String                            styleId;
    sf::base::String                            luaScriptPath;
    sf::base::Vector<float>                     difficultyMults;
    bool                                        unscored;
    sf::base::Vector<sf::base::String>          tags; //!< optional, parsed from `level.json` "tags" array
    std::unordered_map<float, sf::base::String> validators;
    std::unordered_map<float, sf::base::String> validatorsWithoutPackId;

    LevelData(const ssvuj::Obj& mRoot, const sf::Path& mPackPath, const sf::base::String& mPackId);

    [[nodiscard]] const sf::base::String& getValidator(const float diffMult) const;

    [[nodiscard]] const sf::base::String& getValidatorWithoutPackId(const float diffMult) const;

    [[nodiscard]] float getNthDiffMult(int index) const noexcept;
};

} // namespace hg
