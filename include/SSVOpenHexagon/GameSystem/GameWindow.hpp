// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/GameSystem/GameState.hpp"
#include "SSVOpenHexagon/GameSystem/Timers/TimerStatic.hpp"
#include "SSVOpenHexagon/Input/Bind.hpp"
#include "SSVOpenHexagon/Input/BitsetUtils.hpp"
#include "SSVOpenHexagon/Input/Combo.hpp"
#include "SSVOpenHexagon/Input/InputState.hpp"
#include "SSVUtils/Delegate/Inc/Delegate.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/Touch.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Path.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <SSVUtils/Delegate/Delegate.hpp>
#include <ratio>
#include <utility>

#include <cassert>
#include <cstddef>

namespace ssvs
{

class GameWindow
{
private:
    Input::InputState                       inputState;
    TimerStatic                             timer;
    GameState*                              gameState{nullptr};
    bool                                    running{true};
    sf::base::Optional<sf::RenderWindow>    renderWindow;
    sf::base::String                        title;
    float                                   msUpdate{0.f};
    float                                   msDraw{0.f};
    float                                   maxFPS{60.f};
    float                                   pixelMult{1.f};
    unsigned int                            width{640};
    unsigned int                            height{480};
    unsigned int                            antialiasingLevel{3};
    bool                                    fpsLimited{false};
    bool                                    focus{true};
    bool                                    mustRecreate{true};
    bool                                    vsync{false};
    bool                                    fullscreen{false};
    sf::base::Array<sf::Vec2i, fingerCount> fingerPositions{};

    void runEvents()
    {
        while (const auto optEvent = renderWindow->pollEvent())
        {
            auto& event = optEvent.value();

            if (event.is<sf::Event::Closed>())
            {
                stop();
            }
            else if (event.is<sf::Event::FocusGained>())
            {
                focus = true;
            }
            else if (event.is<sf::Event::FocusLost>())
            {
                inputState.reset();
                focus = false;
            }
            else if (auto* e = event.getIf<sf::Event::KeyPressed>())
            {
                inputState[e->code] = true;
            }
            else if (auto* e = event.getIf<sf::Event::KeyReleased>())
            {
                inputState[e->code] = false;
            }
            else if (auto* e = event.getIf<sf::Event::MouseButtonPressed>())
            {
                inputState[e->button] = true;
            }
            else if (auto* e = event.getIf<sf::Event::MouseButtonReleased>())
            {
                inputState[e->button] = false;
            }
            else if (auto* e = event.getIf<sf::Event::TouchBegan>())
            {
                inputState.getFinger(e->finger) = true;
                fingerPositions[e->finger]      = e->position;
            }
            else if (auto* e = event.getIf<sf::Event::TouchMoved>())
            {
                fingerPositions[e->finger] = e->position;
            }
            else if (auto* e = event.getIf<sf::Event::TouchEnded>())
            {
                inputState.getFinger(e->finger) = false;
                fingerPositions[e->finger]      = e->position;
            }

            gameState->handleEvent(event);
        }
    }

    void recreateWindow()
    {
        renderWindow.reset();

        sf::WindowSettings settings{
            .size            = {width, height},
            .title           = title,
            .fullscreen      = fullscreen,
            .contextSettings = {.attributeFlags = sf::ContextSettings::Attribute::Default},
        };

        renderWindow = sf::RenderWindow::create(settings);
        renderWindow->setSize(sf::Vec2u(width * pixelMult, height * pixelMult));
        renderWindow->setVerticalSyncEnabled(vsync);
        renderWindow->setFramerateLimit(fpsLimited ? maxFPS : 0);

        inputState.reset();
        mustRecreate = false;
        onRecreation();
    }

public:
    ssvu::Delegate<void()> onRecreation;

    explicit GameWindow(const float timerStep, const float timerTimeSlice) : timer{timerStep, timerTimeSlice}
    {
        recreateWindow();
    }

    GameWindow(const GameWindow&)            = delete;
    GameWindow& operator=(const GameWindow&) = delete;
    GameWindow(GameWindow&&)                 = delete;
    GameWindow& operator=(GameWindow&&)      = delete;

    void run()
    {
        using FTDuration = std::chrono::duration<float, std::milli>;

        assert(gameState != nullptr);

        while (running)
        {
            if (mustRecreate)
            {
                recreateWindow();
            }

            (void)renderWindow->setActive(true);
            clear();

            auto tempMs = std::chrono::high_resolution_clock::now();
            {
                runEvents();

                gameState->refreshInput(inputState);
                timer.runUpdate([this](const float step)
                {
                    gameState->updateInput(inputState, step);
                    gameState->update(step);
                });
                gameState->onPostUpdate();
            }
            msUpdate = std::chrono::duration_cast<FTDuration>(std::chrono::high_resolution_clock::now() - tempMs).count();

            tempMs = std::chrono::high_resolution_clock::now();
            {
                gameState->draw();
                renderWindow->display();
            }
            msDraw = std::chrono::duration_cast<FTDuration>(std::chrono::high_resolution_clock::now() - tempMs).count();

            timer.runFrameTime();
            timer.runFPS();
        }
    }

    void stop() noexcept
    {
        running = false;
    }

    void clear(const sf::Color& color = sf::Color::Transparent)
    {
        renderWindow->clear(color);
    }

    template <typename... Ts>
    void draw(Ts&&... xs)
    {
        renderWindow->draw(std::forward<Ts>(xs)...);
    }

    void saveScreenshot(const sf::base::String& path) const
    {
        auto texture = sf::Texture::create({renderWindow->getSize().x, renderWindow->getSize().y});

        if (!texture.hasValue())
        {
            return;
        }

        (void)texture->update(*renderWindow);
        auto img = texture->copyToImage();
        (void)img.saveToFile(path);
    }

    void setFullscreen(const bool newFullscreen) noexcept
    {
        fullscreen   = newFullscreen;
        mustRecreate = true;
    }

    void setSize(const unsigned int newWidth, const unsigned int newHeight) noexcept
    {
        width        = newWidth;
        height       = newHeight;
        mustRecreate = true;
    }

    void setAntialiasingLevel(const unsigned int level) noexcept
    {
        antialiasingLevel = level;
        mustRecreate      = true;
    }

    void setVsync(const bool enabled) noexcept
    {
        vsync        = enabled;
        mustRecreate = true;
    }

    void setPixelMult(const float newPixelMult) noexcept
    {
        pixelMult    = newPixelMult;
        mustRecreate = true;
    }

    void setMouseCursorVisible(const bool enabled)
    {
        renderWindow->setMouseCursorVisible(enabled);
    }

    void setTitle(sf::base::String newTitle)
    {
        title = std::move(newTitle);
        renderWindow->setTitle(title);
    }

    void setMaxFPS(const float newMaxFPS)
    {
        maxFPS = newMaxFPS;
        renderWindow->setFramerateLimit(fpsLimited ? maxFPS : 0);
    }

    void setFPSLimited(const bool newFPSLimited)
    {
        fpsLimited = newFPSLimited;
        renderWindow->setFramerateLimit(fpsLimited ? maxFPS : 0);
    }

    void setGameState(GameState& newGameState) noexcept
    {
        gameState = &newGameState;
    }

    operator sf::RenderWindow&() noexcept
    {
        return *renderWindow;
    }

    [[nodiscard]] auto& getRenderWindow() noexcept
    {
        return *renderWindow;
    }

    [[nodiscard]] const auto& getRenderWindow() const noexcept
    {
        return *renderWindow;
    }

    [[nodiscard]] bool getFullscreen() const noexcept
    {
        return fullscreen;
    }

    [[nodiscard]] auto getWidth() const noexcept
    {
        return width;
    }

    [[nodiscard]] auto getHeight() const noexcept
    {
        return height;
    }

    [[nodiscard]] auto getAntialiasingLevel() const noexcept
    {
        return antialiasingLevel;
    }

    [[nodiscard]] bool hasFocus() const noexcept
    {
        return focus;
    }

    [[nodiscard]] bool getVsync() const noexcept
    {
        return vsync;
    }

    [[nodiscard]] float getMsUpdate() const noexcept
    {
        return msUpdate;
    }

    [[nodiscard]] float getMsDraw() const noexcept
    {
        return msDraw;
    }

    [[nodiscard]] auto getMousePosition() const noexcept
    {
        return sf::Mouse::getPosition(*renderWindow);
    }

    [[nodiscard]] auto getFingerPosition(const FingerID finger) const noexcept
    {
        return fingerPositions[finger];
    }

    [[nodiscard]] const auto& getInputState() const noexcept
    {
        return inputState;
    }

    [[nodiscard]] auto getFingerDownCount() const noexcept
    {
        return inputState.getFingers().count();
    }

    [[nodiscard]] auto getFingerDownPositions() const noexcept
    {
        sf::base::Vector<sf::Vec2i> result;

        for (std::size_t i = 0; i < fingerCount; ++i)
        {
            if (inputState.getFingers()[i])
            {
                result.emplaceBack(getFingerPosition(i).to<sf::Vec2i>());
            }
        }

        return result;
    }

    [[nodiscard]] auto getFPS() const noexcept
    {
        return timer.getFPS();
    }

    void resetTimer()
    {
        timer.reset();
    }

    void recreate() noexcept
    {
        mustRecreate = true;
    }
};

} // namespace ssvs
