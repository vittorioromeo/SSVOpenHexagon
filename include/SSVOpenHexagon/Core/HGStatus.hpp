// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/Clock.hpp"

#include <SFML/Graphics/Color.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include <string>

namespace hg {

enum class StateChange
{
    None,
    MustRestart,
    MustReplay
};

enum class RenderStage : std::size_t
{
    BackgroundTris = 0,
    WallQuads3D = 1,
    PivotQuads3D = 2,
    PlayerTris3D = 3,
    WallQuads = 4,
    CapTris = 5,
    PivotQuads = 6,
    PlayerTris = 7,
    Text = 8,

    Count = 9
};

struct HexagonGameStatus
{
public:
    struct FlashColor
    {
        int r;
        int g;
        int b;
    };

private:
    double totalFrametimeAccumulator{};  // Total time (including pauses)
    double playedFrametimeAccumulator{}; // Played time (no pauses)
    double pausedFrametimeAccumulator{}; // Paused time (only pauses)
    double currentPause{0.1 * 60};       // Current pause time
    double currentIncrementTime{};       // Time since last increment
    float customScore{};                 // Value for alternative scoring

public:
    float pulse{75};
    float pulseDirection{1};
    float pulseDelay{0};
    float beatPulse{0};
    float beatPulseDelay{0};
    float pulse3D{1.f};
    float pulse3DDirection{1};
    float flashEffect{0};
    float radius{75};
    float fastSpin{0};
    float cameraShake{0};
    bool hasDied{false};
    StateChange mustStateChange{StateChange::None};
    bool scoreInvalid{false};
    std::string invalidReason{""};
    bool started{false};
    std::string restartInput;
    std::string replayInput;
    bool showPlayerTrail{true};

    // Shaders
    std::array<std::optional<std::size_t>,
        static_cast<std::size_t>(RenderStage::Count)>
        fragmentShaderIds;

    // Reset all the time points and signal that we started
    void start() noexcept;

    // Number of seconds that have passed since last increment
    [[nodiscard]] double getIncrementTimeSeconds() const noexcept;

    // Game timer, in seconds
    [[nodiscard]] double getTimeSeconds() const noexcept;

    // Absolute time, as time point
    [[nodiscard]] HRTimePoint getCurrentTP() const noexcept;

    // Game timer, as time point
    [[nodiscard]] HRTimePoint getTimeTP() const noexcept;

    // Level start, as time point
    [[nodiscard]] HRTimePoint getLevelStartTP() const noexcept;

    // `true` if we are currently paused
    [[nodiscard]] bool isTimePaused() const noexcept;

    // Start a new pause or extend the current pause by `seconds`
    void pauseTime(const double seconds) noexcept;

    // Reset the increment time to the last non-pause time point
    void resetIncrementTime() noexcept;

    // Resets the current time and increment time
    void resetTime();

    // Accumulate the time spent in a frame into the total
    void accumulateFrametime(const double ft) noexcept;

    // Update the custom score
    void updateCustomScore(const float score) noexcept;

    // Get total accumulated frametime
    [[nodiscard]] double getTotalAccumulatedFrametime() const noexcept;

    // Get total accumulated frametime, in seconds
    [[nodiscard]] double getTotalAccumulatedFrametimeInSeconds() const noexcept;

    // Get played accumulated frametime
    [[nodiscard]] double getPlayedAccumulatedFrametime() const noexcept;

    // Get played accumulated frametime, in seconds
    [[nodiscard]] double
    getPlayedAccumulatedFrametimeInSeconds() const noexcept;

    // Get paused accumulated frametime
    [[nodiscard]] double getPausedAccumulatedFrametime() const noexcept;

    // Get paused accumulated frametime, in seconds
    [[nodiscard]] double
    getPausedAccumulatedFrametimeInSeconds() const noexcept;

    // Get custom score
    [[nodiscard]] float getCustomScore() const noexcept;
};

} // namespace hg
