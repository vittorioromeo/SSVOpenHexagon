// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

#include <SFML/Window.hpp>

#include <utility>

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

namespace hg::Joystick
{

struct JoystickState
{
    bool leftWasPressed;
    bool leftPressed;

    bool rightWasPressed;
    bool rightPressed;

    bool upWasPressed;
    bool upPressed;

    bool downWasPressed;
    bool downPressed;

    bool selectWasPressed;
    bool selectPressed;

    bool startWasPressed;
    bool startPressed;

    bool aWasPressed;
    bool aPressed;

    bool bWasPressed;
    bool bPressed;
};

[[nodiscard]] static JoystickState& getJoystickState()
{
    static JoystickState res{};
    return res;
}

enum class AxisDir : int
{
    Left = -1,
    Dead = 0,
    Right = 1
};

[[nodiscard]] static AxisDir axisPressed(
    const unsigned int joyId, const sf::Joystick::Axis axis)
{
    const float deadzone = Config::getJoystickDeadzone();
    const auto pos = sf::Joystick::getAxisPosition(joyId, axis);

    if(pos < -deadzone)
    {
        return AxisDir::Left;
    }

    if(pos > deadzone)
    {
        return AxisDir::Right;
    }

    return AxisDir::Dead;
}

void update()
{
    sf::Joystick::update();

    constexpr unsigned int joyId = 0;
    auto& s = getJoystickState();

    const auto dpadXIs = [&](const AxisDir axisDir) {
        return axisPressed(joyId, sf::Joystick::Axis::PovX) == axisDir;
    };

    const auto dpadYIs = [&](const AxisDir axisDir) {
        return axisPressed(joyId, sf::Joystick::Axis::PovY) == axisDir;
    };

    const auto leftStickXIs = [&](const AxisDir axisDir) {
        return axisPressed(joyId, sf::Joystick::Axis::X) == axisDir;
    };

    const auto leftStickYIs = [&](const AxisDir axisDir) {
        return axisPressed(joyId, sf::Joystick::Axis::Y) == axisDir;
    };

    const auto xIs = [&](const AxisDir axisDir) {
        return dpadXIs(axisDir) || leftStickXIs(axisDir);
    };

    const auto yIs = [&](const AxisDir axisDir) {
        return dpadYIs(axisDir) || leftStickYIs(axisDir);
    };

    s.leftWasPressed = std::exchange(s.leftPressed, xIs(AxisDir::Left));
    s.rightWasPressed = std::exchange(s.rightPressed, xIs(AxisDir::Right));
    s.upWasPressed = std::exchange(s.upPressed, yIs(AxisDir::Right));
    s.downWasPressed = std::exchange(s.downPressed, yIs(AxisDir::Left));

    s.selectWasPressed =
        std::exchange(s.selectPressed, sf::Joystick::isButtonPressed(joyId, 6));

    s.startWasPressed =
        std::exchange(s.startPressed, sf::Joystick::isButtonPressed(joyId, 7));

    s.aWasPressed =
        std::exchange(s.aPressed, sf::Joystick::isButtonPressed(joyId, 0));

    s.bWasPressed =
        std::exchange(s.bPressed, sf::Joystick::isButtonPressed(joyId, 1));
}



[[nodiscard]] bool leftPressed()
{
    return getJoystickState().leftPressed;
}

[[nodiscard]] bool leftRisingEdge()
{
    return getJoystickState().leftPressed && !getJoystickState().leftWasPressed;
}



[[nodiscard]] bool rightPressed()
{
    return getJoystickState().rightPressed;
}

[[nodiscard]] bool rightRisingEdge()
{
    return getJoystickState().rightPressed &&
           !getJoystickState().rightWasPressed;
}



[[nodiscard]] bool upPressed()
{
    return getJoystickState().upPressed;
}

[[nodiscard]] bool upRisingEdge()
{
    return getJoystickState().upPressed && !getJoystickState().upWasPressed;
}



[[nodiscard]] bool downPressed()
{
    return getJoystickState().downPressed;
}

[[nodiscard]] bool downRisingEdge()
{
    return getJoystickState().downPressed && !getJoystickState().downWasPressed;
}



[[nodiscard]] bool selectPressed()
{
    return getJoystickState().selectPressed;
}

[[nodiscard]] bool selectRisingEdge()
{
    return getJoystickState().selectPressed &&
           !getJoystickState().selectWasPressed;
}



[[nodiscard]] bool startPressed()
{
    return getJoystickState().startPressed;
}

[[nodiscard]] bool startRisingEdge()
{
    return getJoystickState().startPressed &&
           !getJoystickState().startWasPressed;
}



[[nodiscard]] bool aPressed()
{
    return getJoystickState().aPressed;
}

[[nodiscard]] bool aRisingEdge()
{
    return getJoystickState().aPressed && !getJoystickState().aWasPressed;
}



[[nodiscard]] bool bPressed()
{
    return getJoystickState().bPressed;
}

[[nodiscard]] bool bRisingEdge()
{
    return getJoystickState().bPressed && !getJoystickState().bWasPressed;
}

} // namespace hg::Joystick
