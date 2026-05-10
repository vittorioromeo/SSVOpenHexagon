// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Input/BitsetUtils.hpp"
#include "SSVOpenHexagon/Input/Enums.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/Base/InitializerList.hpp"

namespace ssvs::Input
{

class InputState;
class Manager;

class Combo
{
private:
    KeyBitset keys;
    BtnBitset btns;

public:
    Combo() = default;

    Combo(const std::initializer_list<sf::Keyboard::Key>& initKeys,
          const std::initializer_list<sf::Mouse::Button>& initButtons = {})
    {
        for (const sf::Keyboard::Key key : initKeys)
        {
            addKey(key);
        }

        for (const sf::Mouse::Button button : initButtons)
        {
            addBtn(button);
        }
    }

    Combo(const std::initializer_list<sf::Mouse::Button>& initButtons) : Combo{{}, initButtons}
    {
    }

    [[nodiscard]] bool operator==(const Combo& rhs) const noexcept
    {
        return keys == rhs.keys && btns == rhs.btns;
    }

    [[nodiscard]] bool operator!=(const Combo& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    [[nodiscard]] bool isDown(Manager& manager, InputState& inputState, Mode mode) const;

    void addKey(const sf::Keyboard::Key key) noexcept
    {
        setKeyBit(keys, key, true);

        if (key != sf::Keyboard::Key::Unknown)
        {
            setKeyBit(keys, sf::Keyboard::Key::Unknown, false);
        }
    }

    void addBtn(const sf::Mouse::Button button) noexcept
    {
        setBtnBit(btns, button, true);
        setKeyBit(keys, sf::Keyboard::Key::Unknown, false);
    }

    void clearBind()
    {
        keys.resetAll();
        btns.resetAll();
        setKeyBit(keys, sf::Keyboard::Key::Unknown, true);
    }

    [[nodiscard]] bool isUnbound() const
    {
        return getKeyBit(keys, sf::Keyboard::Key::Unknown);
    }

    [[nodiscard]] const auto& getKeys() const noexcept
    {
        return keys;
    }

    [[nodiscard]] const auto& getBtns() const noexcept
    {
        return btns;
    }
};

} // namespace ssvs::Input
