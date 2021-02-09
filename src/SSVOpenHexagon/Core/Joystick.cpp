// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

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
    bool ignoreAllPresses;

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

    bool exitWasPressed;
    bool exitPressed;

    bool focusWasPressed;
    bool focusPressed;

    bool swapWasPressed;
    bool swapPressed;

    bool forceRestartWasPressed;
    bool forceRestartPressed;

    bool restartWasPressed;
    bool restartPressed;

    bool replayWasPressed;
    bool replayPressed;

    bool screenshotWasPressed;
    bool screenshotPressed;

    bool nextPackWasPressed;
    bool nextPackPressed;

    bool previousPackWasPressed;
    bool previousPackPressed;

    bool addToFavoritesWasPressed;
    bool addToFavoritesPressed;

    bool favoritesMenuWasPressed;
    bool favoritesMenuPressed;

    unsigned int joystickInputs[JoystickBindsCount];
};

[[nodiscard]] static JoystickState& getJoystickState()
{
    static JoystickState res{};
    return res;
}

void ignoreAllPresses(bool ignore)
{
    auto& s = getJoystickState();

    // If xxxWasPressed values are true all new presses return false.
    // Placed here to not be reiterated every update() cycle.
    s.leftWasPressed = s.rightWasPressed = s.upWasPressed = s.downWasPressed =
        s.selectWasPressed = s.exitWasPressed = s.focusWasPressed =
            s.swapWasPressed = s.forceRestartWasPressed = s.restartWasPressed =
                s.replayWasPressed = s.screenshotWasPressed =
                    s.nextPackWasPressed = s.previousPackWasPressed = 
                        s.addToFavoritesWasPressed = s.favoritesMenuWasPressed =
                            ignore;

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

    // all presses are being ignored, try again later
    if(s.ignoreAllPresses)
    {
        return;
    }

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
        return dpadYIs(axisDir) || leftStickYIs(-axisDir);
    };

    s.leftWasPressed = std::exchange(s.leftPressed, xIs(AxisDir::Left));
    s.rightWasPressed = std::exchange(s.rightPressed, xIs(AxisDir::Right));
    s.upWasPressed = std::exchange(s.upPressed, yIs(AxisDir::Right));
    s.downWasPressed = std::exchange(s.downPressed, yIs(AxisDir::Left));

    s.selectWasPressed = std::exchange(s.selectPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::Select]));

    s.exitWasPressed = std::exchange(s.exitPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::Exit]));

    s.focusWasPressed = std::exchange(s.focusPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::Focus]));

    s.swapWasPressed = std::exchange(s.swapPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::Swap]));

    s.forceRestartWasPressed = std::exchange(
        s.forceRestartPressed, sf::Joystick::isButtonPressed(
            joyId, s.joystickInputs[Jid::ForceRestart]));

    s.restartWasPressed = std::exchange(s.restartPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::Restart]));

    s.replayWasPressed = std::exchange(s.replayPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::Replay]));

    s.screenshotWasPressed = std::exchange(
        s.screenshotPressed, sf::Joystick::isButtonPressed(
            joyId, s.joystickInputs[Jid::Screenshot]));

    s.nextPackWasPressed = std::exchange(s.nextPackPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::NextPack]));

    s.previousPackWasPressed = std::exchange(
        s.previousPackPressed, sf::Joystick::isButtonPressed(
            joyId, s.joystickInputs[Jid::PreviousPack]));

    s.addToFavoritesWasPressed = std::exchange(s.addToFavoritesPressed,
        sf::Joystick::isButtonPressed(joyId, s.joystickInputs[Jid::AddToFavorites]));

    s.favoritesMenuWasPressed = std::exchange(
        s.favoritesMenuPressed, sf::Joystick::isButtonPressed(
            joyId, s.joystickInputs[Jid::FavoritesMenu]));
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


[[nodiscard]] bool exitPressed()
{
    return getJoystickState().exitPressed;
}
[[nodiscard]] bool exitRisingEdge()
{
    return getJoystickState().exitPressed && !getJoystickState().exitWasPressed;
}


[[nodiscard]] bool focusPressed()
{
    return getJoystickState().focusPressed;
}
[[nodiscard]] bool focusRisingEdge()
{
    return getJoystickState().focusPressed &&
           !getJoystickState().focusWasPressed;
}


[[nodiscard]] bool swapPressed()
{
    return getJoystickState().swapPressed;
}
[[nodiscard]] bool swapRisingEdge()
{
    return getJoystickState().swapPressed && !getJoystickState().swapWasPressed;
}

[[nodiscard]] bool forceRestartPressed()
{
    return getJoystickState().forceRestartPressed;
}
[[nodiscard]] bool forceRestartRisingEdge()
{
    return getJoystickState().forceRestartPressed &&
           !getJoystickState().forceRestartWasPressed;
}

[[nodiscard]] bool restartPressed()
{
    return getJoystickState().restartPressed;
}
[[nodiscard]] bool restartRisingEdge()
{
    return getJoystickState().restartPressed &&
           !getJoystickState().restartWasPressed;
}

[[nodiscard]] bool replayPressed()
{
    return getJoystickState().replayPressed;
}
[[nodiscard]] bool replayRisingEdge()
{
    return getJoystickState().replayPressed &&
           !getJoystickState().replayWasPressed;
}

[[nodiscard]] bool screenshotPressed()
{
    return getJoystickState().screenshotPressed;
}
[[nodiscard]] bool screenshotRisingEdge()
{
    return getJoystickState().screenshotPressed &&
           !getJoystickState().screenshotWasPressed;
}

[[nodiscard]] bool nextPackPressed()
{
    return getJoystickState().nextPackPressed;
}
[[nodiscard]] bool nextPackRisingEdge()
{
    return getJoystickState().nextPackPressed &&
           !getJoystickState().nextPackWasPressed;
}

[[nodiscard]] bool previousPackPressed()
{
    return getJoystickState().nextPackPressed;
}
[[nodiscard]] bool previousPackRisingEdge()
{
    return getJoystickState().previousPackPressed &&
           !getJoystickState().previousPackWasPressed;
}

[[nodiscard]] bool addToFavoritesPressed()
{
    return getJoystickState().addToFavoritesPressed;
}
[[nodiscard]] bool addToFavoritesRisingEdge()
{
    return getJoystickState().addToFavoritesPressed &&
           !getJoystickState().addToFavoritesWasPressed;
}

[[nodiscard]] bool favoritesMenuPressed()
{
    return  getJoystickState().favoritesMenuPressed;
}
[[nodiscard]] bool favoritesMenuRisingEdge()
{
    return getJoystickState().favoritesMenuPressed &&
           !getJoystickState().favoritesMenuWasPressed;
}

} // namespace hg::Joystick
