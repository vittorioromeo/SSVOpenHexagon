// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Data/TrackedVariable.hpp"

namespace hg
{
class LevelData
{
private:
    ssvuj::Obj root;

public:
    Path packPath;

    std::string id{
        packPath.getStr() + ssvuj::getExtr<std::string>(root, "id", "nullId")};
    std::string name{ssvuj::getExtr<std::string>(root, "name", "nullName")};
    std::string description{
        ssvuj::getExtr<std::string>(root, "description", "")};
    std::string author{ssvuj::getExtr<std::string>(root, "author", "")};
    int menuPriority{ssvuj::getExtr<int>(root, "menuPriority", 0)};
    bool selectable{ssvuj::getExtr<bool>(root, "selectable", true)};
    std::string musicId{
        ssvuj::getExtr<std::string>(root, "musicId", "nullMusicId")};
    std::string styleId{
        ssvuj::getExtr<std::string>(root, "styleId", "nullStyleId")};
    Path luaScriptPath{
        packPath + ssvuj::getExtr<std::string>(root, "luaFile", "nullLuaPath")};
    std::vector<float> difficultyMults{
        ssvuj::getExtr<std::vector<float>>(root, "difficultyMults", {})};

    LevelData(const ssvuj::Obj& mRoot, const Path& mPackPath)
        : root{mRoot}, packPath{mPackPath}
    {
        difficultyMults.emplace_back(1.f);
        ssvu::sort(difficultyMults);
    }

    std::string getRootString() const
    {
        return ssvuj::getWriteToString(root);
    }
};

struct LevelStatus
{
    std::vector<TrackedVariable> trackedVariables;

    float speedMult{1.f};
    float speedInc{0.f};
    float speedMax{0.f};
    float rotationSpeed{0.f};
    float rotationSpeedInc{0.f};
    float rotationSpeedMax{0.f};
    float delayMult{1.f};
    float delayInc{0.f};
    float fastSpin{0.f};
    float incTime{15.f};
    float pulseMin{75.f};
    float pulseMax{80.f};
    float pulseSpeed{0.f};
    float pulseSpeedR{0.f};
    float pulseDelayMax{0.f};
    float pulseDelayHalfMax{0.f};
    float beatPulseMax{0.f};
    float beatPulseDelayMax{0.f};
    float radiusMin{72.f};
    float wallSkewLeft{0.f};
    float wallSkewRight{0.f};
    float wallAngleLeft{0.f};
    float wallAngleRight{0.f};
    float _3dEffectMultiplier{1.f};

    int cameraShake{0};

    unsigned int sides{6};
    unsigned int sidesMax{6};
    unsigned int sidesMin{6};

    bool swapEnabled{false};
    bool tutorialMode{false};
    bool incEnabled{true};
    bool rndSideChangesEnabled{true};
    bool darkenUnevenBackgroundChunk{true};

    std::size_t currentIncrements{0u};
};

} // namespace hg
