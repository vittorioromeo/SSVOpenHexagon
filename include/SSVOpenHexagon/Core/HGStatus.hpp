// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <sys/time.h>

#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{
struct HexagonGameStatus
{
private:
    ssvu::ObfuscatedValue<uint64_t> globalTms{0};
    ssvu::ObfuscatedValue<uint64_t> incrementTms{0};

public:
    ssvu::ObfuscatedValue<float> currentTime{0.f};
    float incrementTime{0};
    float timeStop{100};
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
        struct timeval time;
        gettimeofday(&time, NULL);
        globalTms = time.tv_sec * 1000000 + time.tv_usec;
        incrementTms = globalTms;
    }

    void resetIncrementTime() {
        struct timeval time;
        gettimeofday(&time, NULL);
        incrementTms = time.tv_sec * 1000000 + time.tv_usec;
        incrementTime = 0.f;
    }

    void updateTime() {
        struct timeval time;
        gettimeofday(&time, NULL);
        uint64_t currentTms = time.tv_sec * 1000000 + time.tv_usec;

        currentTime = (float)(currentTms - globalTms) / 1000000.f;
        incrementTime = (float)(currentTms - incrementTms) / 1000000.f;
    }
};

} // namespace hg
