// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <chrono>

#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{
struct HexagonGameStatus
{
private:
    // Timestamp at which we started playing the level, and for how long we did
    std::chrono::time_point<std::chrono::steady_clock> startTms;

    // Timestamp of the last increment, and duration since then
    std::chrono::time_point<std::chrono::steady_clock> incrementTms;

    // The timestamp at which the timer was paused, and how long we should pause it
    std::chrono::time_point<std::chrono::steady_clock> pauseTms;
    std::chrono::milliseconds pauseLength{100};

    std::chrono::time_point<std::chrono::steady_clock> lastTickTms;

public:
    float pulse{75};
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

    HexagonGameStatus() {
        lastTickTms = std::chrono::steady_clock::now();
        startTms = lastTickTms;
        incrementTms = lastTickTms;
        pauseTms = lastTickTms;
    }

    float getIncrementTimeSeconds();
    float getTimeSeconds();
    bool isTimePaused();
    void pauseTime(float seconds);
    void resetIncrementTime();
    void updateTime();
};

} // namespace hg
