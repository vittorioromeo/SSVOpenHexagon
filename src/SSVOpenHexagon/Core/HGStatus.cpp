// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HGStatus.hpp"

#include <chrono>

namespace hg
{

template <typename Duration>
[[nodiscard]] static auto toMilliseconds(const Duration d) noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(d);
}

void HexagonGameStatus::start() noexcept
{
    // Reset everything to the current time:
    currentTp = Clock::now();
    levelStartTp = currentTp;
    lastIncrementTp = currentTp;
    lastTimerPauseTp = currentTp;

    // Signal that we started:
    started = true;
}

[[nodiscard]] double HexagonGameStatus::getIncrementTimeSeconds() noexcept
{
    // If we are paused, do not count passing time towards an increment:
    const auto ms =
        toMilliseconds(isTimePaused() ? lastTimerPauseTp - lastIncrementTp
                                      : currentTp - lastIncrementTp);

    return static_cast<double>(ms.count()) / 1000.0;
}

[[nodiscard]] double HexagonGameStatus::getTimeSeconds() noexcept
{
    // If we are paused, do not count passing time as significant:
    const auto ms =
        toMilliseconds(isTimePaused() ? lastTimerPauseTp - levelStartTp
                                      : currentTp - levelStartTp);

    return static_cast<double>(ms.count()) / 1000.0;
}

[[nodiscard]] bool HexagonGameStatus::isTimePaused() noexcept
{
    return pauseDuration > (currentTp - lastTimerPauseTp);
}

void HexagonGameStatus::pauseTime(const double seconds) noexcept
{
    const auto ms =
        std::chrono::milliseconds(static_cast<int>(seconds * 1000.0));

    // If we are paused, add to the current (in progress) pause duration:
    if(isTimePaused())
    {
        pauseDuration += ms;
        return;
    }

    // Otherwise, start a new pause:
    lastTimerPauseTp = currentTp;
    pauseDuration = ms;
}

void HexagonGameStatus::resetIncrementTime() noexcept
{
    // If we are paused, use the last time point before pausing as our next
    // starting point for an increment. Otherwise, use the current time:
    lastIncrementTp = isTimePaused() ? lastTimerPauseTp : currentTp;
}

void HexagonGameStatus::updateTime() noexcept
{
    const bool wasPaused = isTimePaused();

    currentTp = HexagonGameStatus::Clock::now();

    // If we were paused on last frame, but we just stopped being paused...
    if(wasPaused && !isTimePaused())
    {
        // ...subtract the pause duration from the total time, so that the time
        // returned by `getTimeSeconds()` is still correct:
        levelStartTp += pauseDuration;

        // ...signal that we are not in a pause anymore:
        lastTimerPauseTp = levelStartTp;
        pauseDuration = std::chrono::milliseconds{0};
    }
}

} // namespace hg
