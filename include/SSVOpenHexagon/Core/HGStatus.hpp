// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/ObfuscatedValue.hpp"

#include <SFML/Graphics/Color.hpp>

#include <chrono>

namespace hg
{

struct HexagonGameStatus
{
private:
    // Current time of the level
    std::chrono::time_point<std::chrono::steady_clock> lastTp;

    // When we started playing the level
    std::chrono::time_point<std::chrono::steady_clock> levelStartTp;

    // When the last increment happened
    std::chrono::time_point<std::chrono::steady_clock> lastIncrementTp;

    // When the timer was last paused, and for how long
    std::chrono::time_point<std::chrono::steady_clock> lastTimerPauseTp;
    std::chrono::milliseconds pauseDuration{100ms};

public:
    float pulseDirection{1};
    float pulseDelay{0};
    float pulseDelayHalf{0};
    float beatPulse{0};
    float beatPulseDelay{0};
    float pulse3D{1.f};
    float pulse3DDirection{1};
    float flashEffect{0};
    float radius{75};
    float fastSpin{0};
    bool hasDied{false}, mustRestart{false};
    bool scoreInvalid{false};
    bool started{false};
    sf::Color overrideColor{sf::Color::Transparent};
    ssvu::ObfuscatedValue<float> lostFrames{0};

    void start() {
        lastTp = std::chrono::steady_clock::now();
        startTp = lastTp;
        incrementTp = lastTp;
        pauseTp = lastTp;
        started = true;
    }

    [[nodiscard]] float getIncrementTimeSeconds();
    [[nodiscard]] float getTimeSeconds();
    [[nodiscard]] bool isTimePaused();
    void pauseTime(float seconds);
    void resetIncrementTime();
    void updateTime();
};

} // namespace hg
