// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once


#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/InputState.hpp"
#include "SSVOpenHexagon/Input/Manager.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"
#include "SSVUtils/Delegate/Inc/Delegate.hpp"

#include <SFML/Window/Event.hpp>
#include <SSVUtils/Delegate/Delegate.hpp>
#include <functional>
#include <utility>

namespace ssvs
{

class GameWindow;

class GameState
{
    friend GameWindow;

private:
    using ITrigger = Input::Trigger;
    using IType    = Input::Type;
    using IMode    = Input::Mode;
    using IFunc    = std::function<void(float)>;

    Input::Manager inputManager;

    void handleEvent(const sf::Event& event)
    {
        onAnyEvent(event);
    }

    void update(const float ft)
    {
        onUpdate(ft);
    }

    void draw()
    {
        onDraw();
    }

    void updateInput(Input::InputState& inputState, const float ft)
    {
        inputManager.update(inputState, ft);
    }

    void refreshInput(Input::InputState& inputState)
    {
        inputManager.refresh(inputState);
    }

public:
    ssvu::Delegate<void()>                 onDraw;
    ssvu::Delegate<void()>                 onPostUpdate;
    ssvu::Delegate<void(float)>            onUpdate;
    ssvu::Delegate<void(const sf::Event&)> onAnyEvent;

    GameState() = default;

    GameState(const GameState&)            = delete;
    GameState& operator=(const GameState&) = delete;

    auto& addInput(ITrigger    trigger,
                   IFunc       on,
                   IFunc       off,
                   const IType type      = IType::Always,
                   const int   triggerID = -1,
                   const IMode mode      = IMode::Overlap)
    {
        return inputManager.emplace(std::move(trigger), type, mode, triggerID, on, off);
    }

    auto& addInput(ITrigger    trigger,
                   IFunc       on,
                   const IType type      = IType::Always,
                   const int   triggerID = -1,
                   const IMode mode      = IMode::Overlap)
    {
        return addInput(std::move(trigger), on, [](float) {}, type, triggerID, mode);
    }

    void refreshTrigger(const Input::Trigger& trigger, const int bindID)
    {
        inputManager.refreshTriggers(trigger, bindID);
    }

    void ignoreNextInputs() noexcept
    {
        inputManager.ignoreNextInputs();
    }

    void ignoreAllInputs(const bool ignore) noexcept
    {
        inputManager.ignoreAllInputs(ignore);
    }
};

} // namespace ssvs
