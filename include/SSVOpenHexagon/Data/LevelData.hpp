// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace Json {
class Value;
}

namespace ssvuj {
using Obj = Json::Value;
}

namespace hg {

struct LevelData
{
    std::string packPath;
    std::string packId;

    std::string id;
    std::string name;
    std::string description;
    std::string author;
    int menuPriority;
    bool selectable;
    std::string musicId;
    std::string soundId;
    std::string styleId;
    std::string luaScriptPath;
    std::vector<float> difficultyMults;
    bool unscored;
    std::unordered_map<float, std::string> validators;
    std::unordered_map<float, std::string> validatorsWithoutPackId;

    LevelData(const ssvuj::Obj& mRoot, const std::string& mPackPath,
        const std::string& mPackId);

    [[nodiscard]] const std::string& getValidator(const float diffMult) const;

    [[nodiscard]] const std::string& getValidatorWithoutPackId(
        const float diffMult) const;

    [[nodiscard]] float getNthDiffMult(int index) const noexcept;
};

} // namespace hg
