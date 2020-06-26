// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Graphics/Color.hpp>

#include "SSVOpenHexagon/Utils/ObfuscatedValue.hpp"

namespace hg
{

struct HexagonGameStatus
{
    ObfuscatedValue<float> currentTime{0.f};
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
    ObfuscatedValue<float> lostFrames{0};
};

} // namespace hg
