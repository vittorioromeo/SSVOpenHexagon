// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"


namespace ssvs::Input
{

class InputState;
class Manager;

class Bind
{
private:
    using InputFunc = sf::base::FixedFunction<void(float), 64>;

    Manager&        manager;
    Trigger         trigger;
    InputFunc       on;
    InputFunc       off;
    sf::base::SizeT priorityCombo{0u};
    sf::base::SizeT priorityUser{0u};
    Type            type{Type::Always};
    Mode            mode{Mode::Overlap};
    bool            released{true};
    int             triggerID{-1};

    [[nodiscard]] bool isDown(InputState& inputState) const
    {
        for (const auto& combo : trigger.getCombos())
        {
            if (combo.isDown(manager, inputState, mode))
            {
                return true;
            }
        }

        return false;
    }

    void recalculatePriorityCombo()
    {
        sf::base::SizeT maxPriority{0u};

        for (const auto& combo : trigger.getCombos())
        {
            maxPriority = sf::base::max(combo.getKeys().count() + combo.getBtns().count(), maxPriority);
        }

        priorityCombo = maxPriority;
    }

public:
    Bind(Manager&         manager,
         Trigger          trigger,
         const Type       type,
         const Mode       mode,
         const int        triggerID,
         const InputFunc& on  = [](float) {},
         const InputFunc& off = [](float) {}) :
        manager{manager},
        trigger{SFML_BASE_MOVE(trigger)},
        on{on},
        off{off},
        type{type},
        mode{mode},
        triggerID{triggerID}
    {
        recalculatePriorityCombo();
    }

    void update(const float ft, InputState& inputState)
    {
        isActive(inputState) ? on(ft) : off(ft);
    }

    void refresh(InputState& inputState)
    {
        if (!released && !isDown(inputState))
        {
            released = true;
        }
    }

    void refreshTrigger(const Trigger& newTrigger)
    {
        trigger = newTrigger;
        recalculatePriorityCombo();
    }

    [[nodiscard]] int getTriggerID() const
    {
        return triggerID;
    }

    void setType(const Type newType) noexcept
    {
        type = newType;
    }

    void setMode(const Mode newMode) noexcept
    {
        mode = newMode;
    }

    void setReleased(const bool value) noexcept
    {
        released = value;
    }

    [[nodiscard]] bool isActive(InputState& inputState)
    {
        if (type == Type::Always)
        {
            return isDown(inputState);
        }

        if (released && isDown(inputState))
        {
            released = false;
            return true;
        }

        return false;
    }

    [[nodiscard]] bool operator<(const Bind& rhs) const noexcept
    {
        if (priorityUser != rhs.priorityUser)
        {
            return priorityCombo > rhs.priorityCombo;
        }

        return priorityUser < rhs.priorityUser;
    }

    void setPriorityUser(const sf::base::SizeT value) noexcept;
};

} // namespace ssvs::Input
