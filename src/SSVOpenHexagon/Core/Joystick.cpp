// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Joystick.hpp"

#include "SSVOpenHexagon/Utils/Casts.hpp"

#include <SFML/Window/Joystick.hpp>

#include <utility>
#include <array>

/*

XBOX 360 Controller Mapping

- buttons
    - A 0
    - B 1
    - X 2
    - Y 3
    - LB 4
    - RB 5
    - back 6
    - start 7
    - L (left stick) 8
    - R (right stick) 9

- left stick axis
    - left - X
    - right + X
    - up - Y
    - down + Y

- right stick axis
    - left - U
    - right + U
    - up - R
    - down + R

- dpad
    - left - povX
    - right + povX
    - up + povY
    - down - povY

- triggers
    - LT + Z
    - RT - Z

*/

namespace hg::Joystick {

struct JoystickState
{
    bool ignoreAllPresses;

    std::array<bool, toSizeT(Jdir::JoystickDirectionsCount)> dirWasPressed;
    std::array<bool, toSizeT(Jdir::JoystickDirectionsCount)> dirPressed;
    std::array<bool, toSizeT(Jid::JoystickBindsCount)> wasPressed;
    std::array<bool, toSizeT(Jid::JoystickBindsCount)> pressed;

    unsigned int joystickInputs[toSizeT(hg::Joystick::Jid::JoystickBindsCount)];
};

[[nodiscard]] static JoystickState& getJoystickState()
{
    static JoystickState res{};
    return res;
}

void ignoreAllPresses(const bool ignore)
{
    auto& s = getJoystickState();

    // If xxxWasPressed values are true all new presses return false.
    // Placed here to not be reiterated every update() cycle.

    for (bool& b : s.dirWasPressed)
    {
        b = ignore;
    }

    for (bool& b : s.wasPressed)
    {
        b = ignore;
    }

    getJoystickState().ignoreAllPresses = ignore;
}

void setJoystickBind(const unsigned int button, const int buttonID)
{
    getJoystickState().joystickInputs[buttonID] = button;
}

enum class AxisDir : int
{
    Left = -1,
    Dead = 0,
    Right = 1
};

[[nodiscard]] AxisDir operator-(AxisDir dir)
{
    return static_cast<AxisDir>(-static_cast<int>(dir));
}

[[nodiscard]] static AxisDir axisPressed(const float deadzone,
    const unsigned int joyId, const sf::Joystick::Axis axis)
{
    const auto pos = sf::Joystick::getAxisPosition(joyId, axis);

    if (pos < -deadzone)
    {
        return AxisDir::Left;
    }

    if (pos > deadzone)
    {
        return AxisDir::Right;
    }

    return AxisDir::Dead;
}

void update(const float deadzone)
{
    constexpr unsigned int joyId = 0;
    auto& s = getJoystickState();

    // all presses are being ignored, try again later
    if (s.ignoreAllPresses)
    {
        return;
    }

    const auto dpadXIs = [&](const AxisDir axisDir) {
        return axisPressed(deadzone, joyId, sf::Joystick::Axis::PovX) ==
               axisDir;
    };

    const auto dpadYIs = [&](const AxisDir axisDir) {
        return axisPressed(deadzone, joyId, sf::Joystick::Axis::PovY) ==
               axisDir;
    };

    const auto leftStickXIs = [&](const AxisDir axisDir)
    { return axisPressed(deadzone, joyId, sf::Joystick::Axis::X) == axisDir; };

    const auto leftStickYIs = [&](const AxisDir axisDir)
    { return axisPressed(deadzone, joyId, sf::Joystick::Axis::Y) == axisDir; };

    const auto xIs = [&](const AxisDir axisDir)
    { return dpadXIs(axisDir) || leftStickXIs(axisDir); };

    const auto yIs = [&](const AxisDir axisDir)
    { return dpadYIs(axisDir) || leftStickYIs(-axisDir); };

    const auto doDir = [&](const Jdir jdir, const bool check)
    {
        s.dirWasPressed[toSizeT(jdir)] =
            std::exchange(s.dirPressed[toSizeT(jdir)], check);
    };

    doDir(Jdir::Left, xIs(AxisDir::Left));
    doDir(Jdir::Right, xIs(AxisDir::Right));
    doDir(Jdir::Up, yIs(AxisDir::Right));
    doDir(Jdir::Down, yIs(AxisDir::Left));

    const auto doButton = [&](const Jid jid)
    {
        s.wasPressed[toSizeT(jid)] = std::exchange(
            s.pressed[toSizeT(jid)], sf::Joystick::isButtonPressed(joyId,
                                         s.joystickInputs[toSizeT(jid)]));
    };

    doButton(Jid::Select);
    doButton(Jid::Exit);
    doButton(Jid::Focus);
    doButton(Jid::Swap);
    doButton(Jid::ForceRestart);
    doButton(Jid::Restart);
    doButton(Jid::Replay);
    doButton(Jid::Screenshot);
    doButton(Jid::NextPack);
    doButton(Jid::PreviousPack);
    doButton(Jid::AddToFavorites);
    doButton(Jid::FavoritesMenu);
}

[[nodiscard]] bool pressed(const Jdir jdir)
{
    return getJoystickState().dirPressed[toSizeT(jdir)];
}

[[nodiscard]] bool risingEdge(const Jdir jdir)
{
    return getJoystickState().dirPressed[toSizeT(jdir)] &&
           !getJoystickState().dirWasPressed[toSizeT(jdir)];
}

[[nodiscard]] bool pressed(const Jid jid)
{
    return getJoystickState().pressed[toSizeT(jid)];
}

[[nodiscard]] bool risingEdge(const Jid jid)
{
    return getJoystickState().pressed[toSizeT(jid)] &&
           !getJoystickState().wasPressed[toSizeT(jid)];
}

} // namespace hg::Joystick
