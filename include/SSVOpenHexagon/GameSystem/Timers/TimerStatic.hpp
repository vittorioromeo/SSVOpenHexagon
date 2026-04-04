// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include "SSVOpenHexagon/Core/Frametime.hpp"

namespace ssvs {

class TimerStatic
{
private:
    sf::Clock clock;
    float frameTime{0.f};
    float fps{0.f};
    float step;
    float timeSlice;
    float time{0.f};
    float maxLoops;
    float loops{0.f};

public:
    TimerStatic(const float step = 1.f, const float timeSlice = 1.f,
        const float maxLoops = 50.f) noexcept
        : step{step}, timeSlice{timeSlice}, maxLoops{maxLoops}
    {}

    void reset()
    {
        time = 0.f;
        loops = 0.f;
    }

    template <typename F>
    void runUpdate(F&& updateFn)
    {
        loops = 0.f;
        time += frameTime;

        while (time >= timeSlice && loops < maxLoops)
        {
            updateFn(step);
            time -= timeSlice;
            ++loops;
        }
    }

    void runFrameTime()
    {
        frameTime = hg::getSecondsToFT(clock.restart().asSeconds());
    }

    void runFPS()
    {
        fps = hg::getFTToFPS(frameTime);
    }

    void setStep(const float newStep) noexcept
    {
        step = newStep;
    }

    void setTimeSlice(const float newTimeSlice) noexcept
    {
        timeSlice = newTimeSlice;
    }

    void setMaxLoops(const float newMaxLoops) noexcept
    {
        maxLoops = newMaxLoops;
    }

    [[nodiscard]] float getFrameTime() const noexcept
    {
        return frameTime;
    }

    [[nodiscard]] float getFPS() const noexcept
    {
        return fps;
    }

    [[nodiscard]] float getStep() const noexcept
    {
        return step;
    }

    [[nodiscard]] float getTimeSlice() const noexcept
    {
        return timeSlice;
    }

    [[nodiscard]] float getTime() const noexcept
    {
        return time;
    }

    [[nodiscard]] float getMaxLoops() const noexcept
    {
        return maxLoops;
    }

    [[nodiscard]] float getLoops() const noexcept
    {
        return loops;
    }
};

} // namespace ssvs
