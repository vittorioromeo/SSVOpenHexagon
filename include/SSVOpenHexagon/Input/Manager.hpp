// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Input/Bind.hpp"
#include "SSVOpenHexagon/Input/InputState.hpp"

#include <SFML/Base/UniquePtr.hpp>

#include <algorithm>
#include <utility>
#include <vector>

namespace ssvs::Input {

class Combo;
class Bind;

class Manager
{
    friend Combo;
    friend Bind;

private:
    InputState processedInput;
    std::vector<sf::base::UniquePtr<Bind>> binds;
    bool isIgnoringNext{false};
    bool isIgnoringAll{false};
    bool mustSort{false};

public:
    void update(InputState& inputState, const float ft)
    {
        if (isIgnoringAll)
        {
            return;
        }

        for (auto& bind : binds)
        {
            bind->update(ft, inputState);

            if (isIgnoringNext)
            {
                break;
            }
        }
    }

    void refresh(InputState& inputState)
    {
        if (mustSort)
        {
            std::sort(std::begin(binds), std::end(binds),
                [](const auto& a, const auto& b) { return *a < *b; });
            mustSort = false;
        }

        isIgnoringNext = false;
        processedInput.reset();

        for (auto& bind : binds)
        {
            bind->refresh(inputState);
        }
    }

    template <typename... TArgs>
    Bind& emplace(TArgs&&... args)
    {
        auto& result = binds.emplace_back(
            sf::base::makeUnique<Bind>(*this, std::forward<TArgs>(args)...));
        mustSort = true;
        return *result;
    }

    void refreshTriggers(const Trigger& trigger, const int bindID)
    {
        for (auto& bind : binds)
        {
            if (bind->getTriggerID() == bindID)
            {
                bind->refreshTrigger(trigger);
            }
        }
    }

    void ignoreNextInputs() noexcept
    {
        isIgnoringNext = true;
    }

    void ignoreAllInputs(const bool ignore) noexcept
    {
        isIgnoringAll = ignore;
    }
};

inline bool Combo::isDown(
    Manager& manager, InputState& inputState, const Mode mode) const
{
    if (isUnbound())
    {
        return false;
    }

    if ((inputState.getKeys() & keys) != keys)
    {
        return false;
    }

    if ((inputState.getBtns() & btns) != btns)
    {
        return false;
    }

    if (mode == Mode::Exclusive)
    {
        if ((manager.processedInput.getKeys() & keys).any())
        {
            return false;
        }

        if ((manager.processedInput.getBtns() & btns).any())
        {
            return false;
        }
    }

    manager.processedInput.getKeys() |= keys;
    manager.processedInput.getBtns() |= btns;

    return true;
}

inline void Bind::setPriorityUser(const std::size_t value) noexcept
{
    priorityUser = value;
    manager.mustSort = true;
}

} // namespace ssvs::Input
