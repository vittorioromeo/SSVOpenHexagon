// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Data/TrackedVariable.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <string>
#include <array>
#include <vector>
#include <regex>

namespace hg
{
class LevelData
{
private:
    ssvuj::Obj root;

    void readLuaVariablesForMenu()
    {
        // Find the lua file.
        std::ifstream luaFile;
        luaFile.open(luaScriptPath);
        if(!luaFile.is_open())
        {
            ssvu::lo("hg::LevelData::readLuaVariablesForMenu()") <<
                "No lua file " << luaScriptPath;
            return;
        }

        // Scroll down the file until "function onInit()"
        // or any whitespace uncommented variant is found.
        std::string line;
        const std::regex initExp(
            "^(?:(?!--).)*function(?:\\s+)onInit(?:\\s|\\()+\\)");

        while(!luaFile.eof())
        {
            getline(luaFile, line);
            if(std::regex_search(line, initExp))
            {
                break;
            }
        }

        // If onInit() does not exist let user know.
        if(luaFile.eof())
        {
            ssvu::lo("hg::Menugame::setIndex()") <<
                "No function onInit() in file " << luaScriptPath;
            luaFile.close();
            return;
        }

        // Search for the uncommented variables.
        struct SearchItem
        {
            using SetValue = std::function<void(const std::string&)>;

            const std::regex exp;
            const SetValue setValue;
            bool found{false};

            SearchItem(const std::string& search, const std::string& capture,
                const SetValue& mAction)
                : exp{"^(?:(?!--).)*" + search + "(?:\\s|\\()+(" + capture + ")"},
                  setValue{[this, mAction](const std::string& mValue) {
                      mAction(mValue);
                      found = true;
                  }}
            {
            }
        };

        std::array<SearchItem, 3> searches{{
            {"l_setSides", "[0-9]+",
                [this](const std::string& mValue) {
                    menuSides = std::stoi(mValue);
                }},
            {"l_setRotationSpeed", "(-?)([0-9]?)+(\\.?)([0-9]?)+",
                [this](const std::string& mValue) {
                    menuRotation = std::stof(mValue);
                }},
            {"l_setDarkenUnevenBackgroundChunk", "true",
                [this](const std::string& /*unused*/) {
                    menuDarkenUnevenBackgroundChunk = true;
                }}
        }};

        std::smatch sm;
        int foundResults{0};
        while(!luaFile.eof())
        {
            getline(luaFile, line);

            // Empty line, no need to search.
            if(line == "\n")
            {
                continue;
            }
            // keep the search within the onInit function.
            if(line.find("end") != std::string::npos)
            {
                break;
            }

            for(SearchItem& s : searches)
            {
                // If a variable has already been found no need to look again.
                // Check the result of regex search.
                if(s.found || !std::regex_search(line, sm, s.exp))
                {
                    continue;
                }
                s.setValue(sm[1]);
                ++foundResults;
                break; // there can only be a function per line.
            }

            if(foundResults == searches.size())
            {
                break;
            }
        }

        luaFile.close();
    }

public:
    ssvufs::Path packPath;
    std::string packId;

    std::string id{ssvuj::getExtr<std::string>(root, "id", "nullId")};
    std::string name{ssvuj::getExtr<std::string>(root, "name", "nullName")};
    std::string description{
        ssvuj::getExtr<std::string>(root, "description", "")};
    std::string author{ssvuj::getExtr<std::string>(root, "author", "")};
    int menuPriority{ssvuj::getExtr<int>(root, "menuPriority", 0)};
    bool selectable{ssvuj::getExtr<bool>(root, "selectable", true)};
    std::string musicId{
        ssvuj::getExtr<std::string>(root, "musicId", "nullMusicId")};
    std::string soundId{
        ssvuj::getExtr<std::string>(root, "soundId", "nullSoundId")};
    std::string styleId{
        ssvuj::getExtr<std::string>(root, "styleId", "nullStyleId")};
    ssvufs::Path luaScriptPath{
        packPath + ssvuj::getExtr<std::string>(root, "luaFile", "nullLuaPath")};
    std::vector<float> difficultyMults{
        ssvuj::getExtr<std::vector<float>>(root, "difficultyMults", {})};

    // Values that are only required in MenuGame.
    unsigned int menuSides{6};
    float menuRotation{0.f};
    bool menuDarkenUnevenBackgroundChunk{false};
    bool favorite{false};

    LevelData(const ssvuj::Obj& mRoot, const ssvufs::Path& mPackPath,
        const std::string& mPackId)
        : root{mRoot}, packPath{mPackPath}, packId{mPackId}
    {
        difficultyMults.emplace_back(1.f);
        ssvu::sort(difficultyMults);
        readLuaVariablesForMenu();
    }

    std::string getRootString() const
    {
        return ssvuj::getWriteToString(root);
    }
};

struct LevelStatus
{
    std::vector<TrackedVariable> trackedVariables;

    // Allows alternative scoring to be possible
    bool scoreOverridden{false};
    std::string scoreOverride;

    // Music and sound related attributes
    bool syncMusicToDM = Config::getMusicSpeedDMSync();
    float musicPitch{1.f};
    std::string beepSound{"beep.ogg"};
    std::string levelUpSound{"increment.ogg"};
    std::string swapSound{"swap.ogg"};
    std::string deathSound{"death.ogg"};

    float speedMult{1.f};
    float playerSpeedMult{1.f};
    float speedInc{0.f};
    float speedMax{0.f};
    float rotationSpeed{0.f};
    float rotationSpeedInc{0.f};
    float rotationSpeedMax{0.f};
    float delayMult{1.f};
    float delayInc{0.f};
    float delayMin{0.f};
    float delayMax{0.f};
    float fastSpin{0.f};
    float incTime{15.f};
    float pulseMin{75.f};
    float pulseMax{80.f};
    float pulseSpeed{0.f};
    float pulseSpeedR{0.f};
    float pulseDelayMax{0.f};
    float pulseDelayHalfMax{0.f};
    float swapCooldownMult{1.f};

    // ------------------------------------------------------------------------
    // A "beat pulse" controls the size of the center polygon. It is supposed
    // to match the beat of the music.
    float beatPulseInitialDelay{0.f}; // Initial delay of the beat pulse.
    float beatPulseMax{0.f};          // Max size increment of the polygon.
    float beatPulseDelayMax{0.f};     // Delay between beat pulses.
    float beatPulseSpeedMult{1.f};    // How fast the pulse "moves" back.

    float radiusMin{72.f};
    float wallSkewLeft{0.f};
    float wallSkewRight{0.f};
    float wallAngleLeft{0.f};
    float wallAngleRight{0.f};
    float wallSpawnDistance{Config::getSpawnDistance()};
    float _3dEffectMultiplier{1.f};

    float cameraShake{0};

    unsigned int sides{6};
    unsigned int sidesMax{6};
    unsigned int sidesMin{6};

    bool swapEnabled{false};
    bool tutorialMode{false};
    bool _3DRequired{false};
    bool incEnabled{true};
    bool rndSideChangesEnabled{true};
    bool darkenUnevenBackgroundChunk{true};


    std::size_t currentIncrements{0u};

    [[nodiscard]] bool hasSpeedMaxLimit() const noexcept
    {
        return speedMax > 0.f;
    }

    [[nodiscard]] bool hasDelayMaxLimit() const noexcept
    {
        return delayMax > 0.f;
    }
};

} // namespace hg
