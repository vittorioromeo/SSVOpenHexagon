// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <cstddef>

namespace hg {

class Ticker
{
private:
    ssvu::FT target;
    ssvu::FT current{0.f};
    ssvu::FT total{0.f};
    bool running{true};
    bool loop{true};
    std::size_t ticks{0};

public:
    Ticker(ssvu::FT mTarget, bool mRunning = true) noexcept;

    bool update(ssvu::FT) noexcept;
    bool update(ssvu::FT mFT, ssvu::FT mTarget) noexcept;

    void pause() noexcept;
    void resume() noexcept;
    void stop() noexcept;
    void restart() noexcept;
    void restart(ssvu::FT mTarget) noexcept;

    void resetCurrent() noexcept;
    void resetTicks() noexcept;
    void resetTotal() noexcept;
    void resetAll() noexcept;

    void setLoop(bool mX) noexcept;

    [[nodiscard]] bool getLoop() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] ssvu::FT getTarget() const noexcept;
    [[nodiscard]] ssvu::FT getCurrent() const noexcept;
    [[nodiscard]] ssvu::FT getTotal() const noexcept;
    [[nodiscard]] std::size_t getTicks() const noexcept;

    template <typename T = ssvu::FT>
    [[nodiscard]] T getTotalSecs() const noexcept
    {
        return static_cast<T>(ssvu::getFTToSeconds(total));
    }

    template <typename T = ssvu::FT>
    [[nodiscard]] T getCurrentSecs() const noexcept
    {
        return static_cast<T>(ssvu::getFTToSeconds(current));
    }
};

} // namespace hg
