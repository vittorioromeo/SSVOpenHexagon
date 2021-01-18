// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Utils/Ticker.hpp"

#include "SSVOpenHexagon/Global/Config.hpp"

#include <SSVStart/Utils/SFML.hpp>
#include <SSVStart/Utils/Vector2.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVUtils/Core/Utils/Math.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace hg
{

inline constexpr float baseThickness{5.f};

CPlayer::CPlayer(const sf::Vector2f& mPos, const float swapCooldown) noexcept
    : startPos{mPos}, pos{mPos}, lastPos{mPos}, hue{0}, angle{0}, lastAngle{0},
      size{Config::getPlayerSize()}, speed{Config::getPlayerSpeed()},
      focusSpeed{Config::getPlayerFocusSpeed()}, dead{false},
      justSwapped{false}, swapTimer{swapCooldown},
      swapBlinkTimer{swapCooldown / 6.f}, deadEffectTimer{80.f, false}
{
}

[[nodiscard]] float CPlayer::getPlayerAngle() const noexcept
{
    return angle;
}

void CPlayer::setPlayerAngle(const float newAng) noexcept
{
    angle = newAng;
}

void CPlayer::draw(HexagonGame& mHexagonGame, const sf::Color& mCapColor)
{
    drawPivot(mHexagonGame, mCapColor);

    if(deadEffectTimer.isRunning())
    {
        drawDeathEffect(mHexagonGame);
    }

    sf::Color colorMain{!deadEffectTimer.isRunning()
                            ? mHexagonGame.getColorPlayer()
                            : ssvs::getColorFromHSV(hue / 360.f, 1.f, 1.f)};

    const float triangleWidth = mHexagonGame.getInputFocused() ? -1.5f : 3.f;

    const sf::Vector2f pLeft = ssvs::getOrbitRad(
        pos, angle - ssvu::toRad(100.f), size + triangleWidth);

    const sf::Vector2f pRight = ssvs::getOrbitRad(
        pos, angle + ssvu::toRad(100.f), size + triangleWidth);

    if(!swapTimer.isRunning() && !dead)
    {
        colorMain = ssvs::getColorFromHSV(
            (swapBlinkTimer.getCurrent() * 15) / 360.f, 1, 1);
    }

    mHexagonGame.playerTris.reserve_more(3);
    mHexagonGame.playerTris.batch_unsafe_emplace_back(
        colorMain, ssvs::getOrbitRad(pos, angle, size), pLeft, pRight);
}

void CPlayer::drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor)
{
    const auto sides(mHexagonGame.getSides());
    const float div{ssvu::tau / sides * 0.5f};
    const float radius{mHexagonGame.getRadius() * 0.75f};

    const sf::Color colorMain{mHexagonGame.getColorMain()};

    const sf::Color colorB{Config::getBlackAndWhite()
                               ? sf::Color::Black
                               : mHexagonGame.getColor(1)};

    const sf::Color colorDarkened{Utils::getColorDarkened(colorMain, 1.4f)};

    for(auto i(0u); i < sides; ++i)
    {
        const float sAngle{div * 2.f * i};

        const sf::Vector2f p1{
            ssvs::getOrbitRad(startPos, sAngle - div, radius)};
        const sf::Vector2f p2{
            ssvs::getOrbitRad(startPos, sAngle + div, radius)};
        const sf::Vector2f p3{
            ssvs::getOrbitRad(startPos, sAngle + div, radius + baseThickness)};
        const sf::Vector2f p4{
            ssvs::getOrbitRad(startPos, sAngle - div, radius + baseThickness)};

        mHexagonGame.wallQuads.reserve_more(4);
        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);

        mHexagonGame.capTris.reserve_more(3);
        mHexagonGame.capTris.batch_unsafe_emplace_back(
            mCapColor, p1, p2, startPos);
    }
}

void CPlayer::drawDeathEffect(HexagonGame& mHexagonGame)
{
    const float div{ssvu::tau / mHexagonGame.getSides() * 0.5f};
    const float radius{hue / 8.f};
    const float thickness{hue / 20.f};

    const sf::Color colorMain{
        ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};

    for(auto i(0u); i < mHexagonGame.getSides(); ++i)
    {
        const float sAngle{div * 2.f * i};

        const sf::Vector2f p1{ssvs::getOrbitRad(pos, sAngle - div, radius)};
        const sf::Vector2f p2{ssvs::getOrbitRad(pos, sAngle + div, radius)};
        const sf::Vector2f p3{
            ssvs::getOrbitRad(pos, sAngle + div, radius + thickness)};
        const sf::Vector2f p4{
            ssvs::getOrbitRad(pos, sAngle - div, radius + thickness)};

        mHexagonGame.wallQuads.reserve_more(4);
        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);
    }
}

void CPlayer::playerSwap(HexagonGame& mHexagonGame, bool mPlaySound)
{
    angle += ssvu::pi;
    mHexagonGame.runLuaFunctionIfExists<void>("onCursorSwap");

    if(mPlaySound)
    {
        mHexagonGame.getAssets().playSound(
            mHexagonGame.getLevelStatus().swapSound);
    }
}

void CPlayer::kill(HexagonGame& mHexagonGame)
{
    deadEffectTimer.restart();

    if(!Config::getInvincible() && !mHexagonGame.getLevelStatus().tutorialMode)
    {
        dead = true;
    }

    mHexagonGame.death();

    if(!getJustSwapped())
    {
        pos = lastPos;
    }
}

[[nodiscard]] bool CPlayer::push(const HexagonGame& mHexagonGame,
    const CWall& wall, const sf::Vector2f& mCenterPos, const ssvu::FT mFT)
{
    if(dead)
    {
        return false;
    }

    constexpr float padding{0.025f};
    int movement{mHexagonGame.getInputMovement()};

    // First of all, if it's a rotating wall push player in the direction the
    // wall is rotating by the appropriate amount, but only if the direction
    // of the rotation is different from the direction player is moving.
    const SpeedData& curveData{wall.getCurve()};
    const int speedSign{ssvu::getSign(curveData.speed)};
    if(curveData.speed != 0.f && speedSign != movement)
    {
        // This is a copy paste of CWall::moveCurve()
        ssvs::rotateRadAround(pos, mCenterPos, curveData.speed / 60.f * mFT);

        // Calculate angle, add a little padding, and readjust the position.
        angle = ssvs::getRad(pos) + speedSign * padding;
        updatePosition(mHexagonGame, mFT);
    }

    // If player is not moving calculate now.
    if(!movement)
    {
        return wall.isOverlapping(pos);
    }

    // Compensate for the player movement to make it slide along the side.
    movement = - movement;
    const float currentSpeed{mHexagonGame.getPlayerSpeedMult() *
        (mHexagonGame.getInputFocused() ? focusSpeed : speed)};
    lastAngle = angle + ssvu::toRad(currentSpeed * movement * mFT) + movement * padding;
    lastPos = ssvs::getOrbitRad(startPos, lastAngle, mHexagonGame.getRadius());

    // If there is overlap even after compensation kill without updating
    // position, as there is not benefit in doing it.
    if(wall.isOverlapping(lastPos))
    {
        return true;
    }

    // If still alive position player right against the wall to give the illusion
    // it is sliding along it. Since this is a standard wall we can assume the required
    // angle is the angle of vertex 0 or 1 depending on which one is close to the player.
    const std::array<sf::Vector2f, 4>& wVertexes{wall.getVertexes()};
    const float radZero{ssvs::getRad(wVertexes[0])}, radOne{ssvs::getRad(wVertexes[1])};
    angle = ssvu::getDistRad(angle, radOne) > ssvu::getDistRad(angle, radZero) ?
                radZero : radOne;
    angle += movement * padding;
    updatePosition(mHexagonGame, mFT);
    return false;
}

[[nodiscard]] bool CPlayer::push(const HexagonGame& mHexagonGame,
    const CCustomWall& wall, const ssvu::FT mFT)
{
    (void)mFT; // Currently unused.

    if(dead)
    {
        return false;
    }

    const int movement{mHexagonGame.getInputMovement()};
    const unsigned int maxAttempts = 5 + speed;
    const float pushDir = -movement;

    const float pushAngle = ssvu::toRad(1.f) * pushDir;

    unsigned int attempt = 0;
    const float radius{mHexagonGame.getRadius()};

    while(wall.isOverlapping(pos))
    {
        angle += pushAngle;
        pos = ssvs::getOrbitRad(startPos, angle, radius);

        if(++attempt >= maxAttempts)
        {
            pos = lastPos;
            angle = lastAngle;

            return true;
        }
    }

    return false;
}

void CPlayer::update(HexagonGame& mHexagonGame, const ssvu::FT mFT)
{
    if(deadEffectTimer.isRunning())
    {
        deadEffectTimer.update(mFT);

        if(++hue > 360.f)
        {
            hue = 0.f;
        }
        if(dead)
        {
            return;
        }
    }

    swapBlinkTimer.update(mFT);

    if(mHexagonGame.getLevelStatus().swapEnabled && swapTimer.update(mFT))
    {
        swapTimer.stop();
    }

    lastPos = pos;
    lastAngle = angle;
}

void CPlayer::updateInput(HexagonGame& mHexagonGame, const ssvu::FT mFT)
{
    const int movement{mHexagonGame.getInputMovement()};

    const float currentSpeed =
        mHexagonGame.getPlayerSpeedMult() *
        (mHexagonGame.getInputFocused() ? focusSpeed : speed);

    angle += ssvu::toRad(currentSpeed * movement * mFT);

    if(mHexagonGame.getLevelStatus().swapEnabled &&
        mHexagonGame.getInputSwap() && !swapTimer.isRunning())
    {
        playerSwap(mHexagonGame, true /* mPlaySound */);
        swapTimer.restart(mHexagonGame.getSwapCooldown());
        swapBlinkTimer.restart(mHexagonGame.getSwapCooldown() / 6.f);
        justSwapped = true;
    }
    else
    {
        justSwapped = false;
    }
}

void CPlayer::updatePosition(const HexagonGame& mHexagonGame, const ssvu::FT mFT)
{
    (void)mFT; // Currently unused.

    pos = ssvs::getOrbitRad(startPos, angle, mHexagonGame.getRadius());
}

[[nodiscard]] bool CPlayer::getJustSwapped() const noexcept
{
    return justSwapped;
}

} // namespace hg
