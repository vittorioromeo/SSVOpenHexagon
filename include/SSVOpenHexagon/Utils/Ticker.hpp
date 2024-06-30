// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <cstddef>

namespace hg {

class Ticker
{
private:
    float target;
    float current{0.f};
    float total{0.f};
    bool running{true};
    bool loop{true};
    std::size_t ticks{0};

public:
    Ticker(float mTarget, bool mRunning = true) noexcept;

    bool update(float) noexcept;
    bool update(float mFT, float mTarget) noexcept;

    void pause() noexcept;
    void resume() noexcept;
    void stop() noexcept;
    void restart() noexcept;
    void restart(float mTarget) noexcept;

    void resetCurrent() noexcept;
    void resetTicks() noexcept;
    void resetTotal() noexcept;
    void resetAll() noexcept;

    void setLoop(bool mX) noexcept;

    [[nodiscard]] bool getLoop() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] float getTarget() const noexcept;
    [[nodiscard]] float getCurrent() const noexcept;
    [[nodiscard]] float getTotal() const noexcept;
    [[nodiscard]] std::size_t getTicks() const noexcept;

    template <typename T = float>
    [[nodiscard]] T getTotalSecs() const noexcept
    {
        return static_cast<T>(total / 60.f);
    }

    template <typename T = float>
    [[nodiscard]] T getCurrentSecs() const noexcept
    {
        return static_cast<T>(current / 60.f);
    }
};

} // namespace hg
