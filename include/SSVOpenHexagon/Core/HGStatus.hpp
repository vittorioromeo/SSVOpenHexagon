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
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

private:
    // Current time of the level
    TimePoint currentTp;

    // When we started playing the level
    TimePoint levelStartTp;

    // When the last increment happened
    TimePoint lastIncrementTp;

    // When the timer was last paused, and for how long
    TimePoint lastTimerPauseTp;

    // Duration of the current timer pause
    std::chrono::milliseconds pauseDuration{100};

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

    // Reset all the time points and signal that we started
    void start() noexcept;

    // Number of seconds that have passed since last increment
    [[nodiscard]] double getIncrementTimeSeconds() noexcept;

    // Game timer, in seconds
    [[nodiscard]] double getTimeSeconds() noexcept;

    // Absolute time, as time point
    [[nodiscard]] TimePoint getCurrentTP() noexcept;

    // Game timer, as time point
    [[nodiscard]] TimePoint getTimeTP() noexcept;

    // Level start, as time point
    [[nodiscard]] TimePoint getLevelStartTP() noexcept;

    // `true` if we are currently paused
    [[nodiscard]] bool isTimePaused() noexcept;

    // Start a new pause or extend the current pause by `seconds`
    void pauseTime(const double seconds) noexcept;

    // Reset the increment time to the last non-pause time point
    void resetIncrementTime() noexcept;

    // Update the timer (called every frame)
    void updateTime() noexcept;
};

} // namespace hg
