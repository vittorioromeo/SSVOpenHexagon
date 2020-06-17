// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <thread>
#include <chrono>
#include <SSVStart/SSVStart.hpp>

namespace hg
{

class FPSWatcher
{
private:
    ssvs::GameWindow& gameWindow;
    float lostFrames{0};
    const float maxLostFrames{20.f}, minFPS{25.f};
    bool disabled{true}, running{true}, check{false};
    std::future<void> watchFuture{
        std::async(std::launch::async, [this] { watch(); })};

    void watch()
    {
        while(running)
        {
            std::this_thread::sleep_for(80ms);
            if(disabled)
            {
                continue;
            }

            if(check)
            {
                check = false;
                std::this_thread::sleep_for(50ms);
                while(!check)
                {
                    loseFrame();
                    std::this_thread::sleep_for(12ms);
                }
            }

            std::this_thread::sleep_for(80ms);
            if(gameWindow.getFPS() < minFPS)
            {
                loseFrame();
            }
        }
    }

    void loseFrame()
    {
        if(lostFrames > maxLostFrames)
        {
            return;
        }

        ++lostFrames;

        ssvu::lo("FPSWatcher::watch")
            << "Slowdown " << lostFrames << "/" << maxLostFrames << "\n";
    }

public:
    FPSWatcher(ssvs::GameWindow& mGameWindow) noexcept : gameWindow(mGameWindow)
    {
    }

    ~FPSWatcher() noexcept
    {
        running = false;
    }

    [[nodiscard]] bool isLimitReached() const noexcept
    {
        return lostFrames >= maxLostFrames;
    }

    void reset() noexcept
    {
        lostFrames = 0;
        disabled = true;
        check = false;
    }

    void update() noexcept
    {
        check = true;
    }

    void enable() noexcept
    {
        disabled = false;
    }

    void disable() noexcept
    {
        disabled = true;
    }
};

} // namespace hg
