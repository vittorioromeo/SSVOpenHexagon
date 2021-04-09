// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Ticker.hpp"

namespace hg {

Ticker::Ticker(ssvu::FT mTarget, bool mRunning) noexcept
    : target{mTarget}, running{mRunning}
{}

bool Ticker::update(ssvu::FT mFT) noexcept
{
    const float increment = mFT * static_cast<float>(running);

    current += increment;
    total += increment;

    if(current < target)
    {
        return false;
    }

    ++ticks;
    resetCurrent();
    running = loop;
    return true;
}

bool Ticker::update(ssvu::FT mFT, ssvu::FT mTarget) noexcept
{
    target = mTarget;
    return update(mFT);
}

void Ticker::pause() noexcept
{
    running = false;
}

void Ticker::resume() noexcept
{
    running = true;
}

void Ticker::stop() noexcept
{
    resetCurrent();
    pause();
}

void Ticker::restart() noexcept
{
    resetCurrent();
    resume();
}

void Ticker::restart(ssvu::FT mTarget) noexcept
{
    target = mTarget;
    restart();
}

void Ticker::resetCurrent() noexcept
{
    current = 0.f;
}

void Ticker::resetTicks() noexcept
{
    ticks = 0;
}

void Ticker::resetTotal() noexcept
{
    total = 0.f;
}

void Ticker::resetAll() noexcept
{
    resetCurrent();
    resetTicks();
    resetTotal();
}

void Ticker::setLoop(bool mX) noexcept
{
    loop = mX;
}

[[nodiscard]] bool Ticker::getLoop() const noexcept
{
    return loop;
}

[[nodiscard]] bool Ticker::isRunning() const noexcept
{
    return running;
}

[[nodiscard]] ssvu::FT Ticker::getTarget() const noexcept
{
    return target;
}

[[nodiscard]] ssvu::FT Ticker::getCurrent() const noexcept
{
    return current;
}

[[nodiscard]] ssvu::FT Ticker::getTotal() const noexcept
{
    return total;
}

[[nodiscard]] std::size_t Ticker::getTicks() const noexcept
{
    return ticks;
}

} // namespace hg
