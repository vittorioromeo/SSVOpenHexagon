// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once


#include "SSVOpenHexagon/GameSystem/Delegate.hpp"
#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/InputState.hpp"
#include "SSVOpenHexagon/Input/Manager.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"

#include "SFML/Window/Event.hpp"

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/Macros.hpp"

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
    using IFunc    = sf::base::FixedFunction<void(float), 64>;

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
    hg::Delegate<void()>                 onDraw;
    hg::Delegate<void()>                 onPostUpdate;
    hg::Delegate<void(float)>            onUpdate;
    hg::Delegate<void(const sf::Event&)> onAnyEvent;

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
        return inputManager.emplace(SFML_BASE_MOVE(trigger), type, mode, triggerID, on, off);
    }

    auto& addInput(ITrigger    trigger,
                   IFunc       on,
                   const IType type      = IType::Always,
                   const int   triggerID = -1,
                   const IMode mode      = IMode::Overlap)
    {
        return addInput(SFML_BASE_MOVE(trigger), on, [](float) {}, type, triggerID, mode);
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
