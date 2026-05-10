// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Input/BitsetUtils.hpp"

namespace ssvs
{
class GameWindow;

namespace Input
{

class InputState
{
    friend ssvs::GameWindow;

private:
    FingerBitset fingers;
    KeyBitset    keys;
    BtnBitset    btns;

public:
    [[nodiscard]] bool getFinger(const FingerID finger) const noexcept
    {
        return getFingerBit(fingers, finger);
    }

    [[nodiscard]] bool operator[](const sf::Keyboard::Key key) const noexcept
    {
        return getKeyBit(keys, key);
    }

    [[nodiscard]] bool operator[](const sf::Mouse::Button button) const noexcept
    {
        return getBtnBit(btns, button);
    }

    // Mutating accessors used by GameWindow to record key/btn events.
    void setFinger(const FingerID finger, const bool value) noexcept
    {
        setFingerBit(fingers, finger, value);
    }

    void setKey(const sf::Keyboard::Key key, const bool value) noexcept
    {
        setKeyBit(keys, key, value);
    }

    void setBtn(const sf::Mouse::Button button, const bool value) noexcept
    {
        setBtnBit(btns, button, value);
    }

    void reset() noexcept
    {
        fingers.resetAll();
        keys.resetAll();
        btns.resetAll();
    }

    [[nodiscard]] auto& getFingers() noexcept
    {
        return fingers;
    }

    [[nodiscard]] auto& getKeys() noexcept
    {
        return keys;
    }

    [[nodiscard]] auto& getBtns() noexcept
    {
        return btns;
    }

    [[nodiscard]] const auto& getFingers() const noexcept
    {
        return fingers;
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

} // namespace Input
} // namespace ssvs
