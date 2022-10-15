// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <unordered_map>

namespace hg {

struct LevelStatus
{
    std::unordered_map<std::string, std::string> trackedVariables;

    // Allows alternative scoring to be possible
    bool scoreOverridden{false};
    std::string scoreOverride;

    // Music and sound related attributes
    bool syncMusicToDM{true};
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
    float pulseInitialDelay{0.f};
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
    float wallSpawnDistance{1600.f};

    float cameraShake{0};

    unsigned int sides{6};
    unsigned int sidesMax{6};
    unsigned int sidesMin{6};

    bool swapEnabled{false};
    bool tutorialMode{false};
    bool _3DRequired{false};
    bool shadersRequired{false};
    bool incEnabled{true};
    bool rndSideChangesEnabled{true};
    bool darkenUnevenBackgroundChunk{true};

    bool manualPulseControl{false};
    bool manualBeatPulseControl{false};

    unsigned long currentIncrements{0u};

    explicit LevelStatus(
        const bool mSyncMusicToDM, const float mWallSpawnDistance);

    [[nodiscard]] bool hasSpeedMaxLimit() const noexcept;

    [[nodiscard]] bool hasDelayMaxLimit() const noexcept;
};

} // namespace hg
