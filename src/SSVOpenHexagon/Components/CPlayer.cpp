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

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVUtils/Core/Utils/Math.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace hg
{

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

inline constexpr unsigned int playerSides{3u};

void CPlayer::drawCommon(HexagonGame& mHexagonGame)
{
    sf::Color colorMain{!deadEffectTimer.isRunning()
                            ? mHexagonGame.getColorPlayer()
                            : ssvs::getColorFromHSV(hue / 360.f, 1.f, 1.f)};

    const float triangleWidth = mHexagonGame.getInputFocused() ? -1.5f : 3.f;

    vertexPositions[0] = ssvs::getOrbitRad(pos, angle, size);
    vertexPositions[1] = ssvs::getOrbitRad(
        pos, angle - ssvu::toRad(100.f), size + triangleWidth);
    vertexPositions[2] = ssvs::getOrbitRad(
        pos, angle + ssvu::toRad(100.f), size + triangleWidth);

    if(!swapTimer.isRunning() && !dead)
    {
        colorMain = ssvs::getColorFromHSV(
            (swapBlinkTimer.getCurrent() * 15) / 360.f, 1, 1);
    }

    mHexagonGame.playerTris.reserve_more(playerSides);
    mHexagonGame.playerTris.batch_unsafe_emplace_back(
        colorMain, vertexPositions[0], vertexPositions[1], vertexPositions[2]);
}

void CPlayer::draw(HexagonGame& mHexagonGame, const sf::Color& mCapColor)
{
    drawPivot(mHexagonGame, mCapColor);
    if(deadEffectTimer.isRunning())
    {
        drawDeathEffect(mHexagonGame);
    }
    drawCommon(mHexagonGame);
}

void CPlayer::draw3D(HexagonGame& mHexagonGame, const sf::Color& mWallColor,
    const sf::Color& mCapColor)
{
    drawPivot3D(mHexagonGame, mWallColor, mCapColor);
    if(deadEffectTimer.isRunning())
    {
        drawDeathEffect3D(mHexagonGame, mWallColor);
    }
    drawCommon(mHexagonGame);

    mHexagonGame.wallQuads3D.reserve_more(playerSides * 4);
    const sf::Vector2f& offset3D{mHexagonGame.get3DOffset()};

    for(unsigned int i{0}, j{2}; i < playerSides; j = i++)
    {
        mHexagonGame.wallQuads3D.batch_unsafe_emplace_back(mWallColor,
            vertexPositions[i], vertexPositions[j],
            sf::Vector2f{vertexPositions[j].x + offset3D.x,
                vertexPositions[j].y + offset3D.y},
            sf::Vector2f{vertexPositions[i].x + offset3D.x,
                vertexPositions[i].y + offset3D.y});
    }
}

void CPlayer::drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor)
{
    const unsigned int sides(mHexagonGame.getSides());
    const float div{ssvu::tau / sides * 0.5f};
    const sf::Color colorMain{mHexagonGame.getColorMain()};

    // This value is needed by all walls in the 3D drawing mode, so add
    // the thickness value even if it needs to be subtracted in the next lines.
    pivotRadius = mHexagonGame.getRadius() * 0.75f + pivotBorderThickness;

    /*  Unused
        const sf::Color colorB{Config::getBlackAndWhite()
                                   ? sf::Color::Black
                                   : mHexagonGame.getColor(1)};
        const sf::Color colorDarkened{Utils::getColorDarkened(colorMain, 1.4f)};
    */

    float sAngle;
    sf::Vector2f p1, p2, p3, p4;
    mHexagonGame.capQuads.reserve(4 * sides);
    mHexagonGame.capTris.reserve(3 * sides);

    for(unsigned int i{0}; i < sides; ++i)
    {
        sAngle = div * 2.f * i;
        p1 = ssvs::getOrbitRad(
            startPos, sAngle - div, pivotRadius - pivotBorderThickness);
        p2 = ssvs::getOrbitRad(
            startPos, sAngle + div, pivotRadius - pivotBorderThickness);
        p3 = ssvs::getOrbitRad(startPos, sAngle + div, pivotRadius);
        p4 = ssvs::getOrbitRad(startPos, sAngle - div, pivotRadius);

        mHexagonGame.capQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);
        mHexagonGame.capTris.batch_unsafe_emplace_back(
            mCapColor, p1, p2, startPos);
    }
}

void CPlayer::drawPivot3D(HexagonGame& mHexagonGame,
    const sf::Color& mWallColor, const sf::Color& mCapColor)
{
    const unsigned int sides(mHexagonGame.getSides());
    const float div{ssvu::tau / sides * 0.5f};
    pivotRadius = mHexagonGame.getRadius() * 0.75f + pivotBorderThickness;
    const sf::Color colorMain{mHexagonGame.getColorMain()};

    float sAngle;
    sf::Vector2f p1, p2, p3;
    mHexagonGame.capQuads.reserve(4 * sides);
    mHexagonGame.capTris.reserve(3 * sides);
    mHexagonGame.wallQuads3D.reserve_more(4 * sides);
    const sf::Vector2f& offset3D{mHexagonGame.get3DOffset()};

    for(unsigned int i{0}; i < sides; ++i)
    {
        sAngle = div * 2.f * i;
        p1 = ssvs::getOrbitRad(
            startPos, sAngle - div, pivotRadius - pivotBorderThickness);
        p2 = ssvs::getOrbitRad(
            startPos, sAngle + div, pivotRadius - pivotBorderThickness);
        p3 = ssvs::getOrbitRad(startPos, sAngle + div, pivotRadius);
        pivotVertexes[i] =
            ssvs::getOrbitRad(startPos, sAngle - div, pivotRadius);

        mHexagonGame.capQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, pivotVertexes[i]);
        mHexagonGame.capTris.batch_unsafe_emplace_back(
            mCapColor, p1, p2, startPos);
        mHexagonGame.wallQuads3D.batch_unsafe_emplace_back(mWallColor, p3,
            pivotVertexes[i],
            sf::Vector2f{pivotVertexes[i].x + offset3D.x,
                pivotVertexes[i].y + offset3D.y},
            sf::Vector2f{p3.x + offset3D.x, p3.y + offset3D.y});
    }
}

void CPlayer::drawDeathEffect(HexagonGame& mHexagonGame)
{
    const unsigned int sides(mHexagonGame.getSides());
    const float div{ssvu::tau / mHexagonGame.getSides() * 0.5f};
    const float radius{hue / 8.f};
    const float thickness{hue / 20.f};
    const sf::Color colorMain{
        ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};

    float sAngle;
    sf::Vector2f p1, p2, p3, p4;
    mHexagonGame.wallQuads.reserve_more(4 * sides);

    for(unsigned int i{0}; i < sides; ++i)
    {
        sAngle = div * 2.f * i;
        p1 = ssvs::getOrbitRad(pos, sAngle - div, radius);
        p2 = ssvs::getOrbitRad(pos, sAngle + div, radius);
        p3 = ssvs::getOrbitRad(pos, sAngle + div, radius + thickness);
        p4 = ssvs::getOrbitRad(pos, sAngle - div, radius + thickness);

        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);
    }
}

void CPlayer::drawDeathEffect3D(
    HexagonGame& mHexagonGame, const sf::Color& mWallColors)
{
    const unsigned int sides(mHexagonGame.getSides());
    const float div{ssvu::tau / mHexagonGame.getSides() * 0.5f};
    const float radius{hue / 8.f};
    const float thickness{hue / 20.f};
    const sf::Color colorMain{
        ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};

    float sAngle;
    sf::Vector2f p1, p2, p3, p4;
    mHexagonGame.wallQuads.reserve_more(4 * sides);
    mHexagonGame.wallQuads3D.reserve_more(4 * 2 * sides);
    const sf::Vector2f& offset3D{mHexagonGame.get3DOffset()};

    for(unsigned int i{0}; i < sides; ++i)
    {
        sAngle = div * 2.f * i;
        p1 = ssvs::getOrbitRad(pos, sAngle - div, radius);
        p2 = ssvs::getOrbitRad(pos, sAngle + div, radius);
        p3 = ssvs::getOrbitRad(pos, sAngle + div, radius + thickness);
        p4 = ssvs::getOrbitRad(pos, sAngle - div, radius + thickness);

        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);
        mHexagonGame.wallQuads3D.batch_unsafe_emplace_back(mWallColors, p1, p2,
            sf::Vector2f{p2.x + offset3D.x, p2.y + offset3D.y},
            sf::Vector2f{p1.x + offset3D.x, p1.y + offset3D.y});
        mHexagonGame.wallQuads3D.batch_unsafe_emplace_back(mWallColors, p3, p4,
            sf::Vector2f{p4.x + offset3D.x, p4.y + offset3D.y},
            sf::Vector2f{p3.x + offset3D.x, p3.y + offset3D.y});
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

    int movement{mHexagonGame.getInputMovement()};

    // First of all, if it's a rotating wall push player in the direction the
    // wall is rotating by the appropriate amount, but only if the direction
    // of the rotation is different from the direction player is moving.
    constexpr float padding{0.025f};
    const SpeedData& curveData{wall.getCurve()};
    const int speedSign{ssvu::getSign(curveData.speed)};

    if(curveData.speed != 0.f && speedSign != movement)
    {
        wall.moveVertexAlongCurve(pos, mCenterPos, mFT);

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
    movement = -movement;
    const float currentSpeed{
        mHexagonGame.getPlayerSpeedMult() *
        (mHexagonGame.getInputFocused() ? focusSpeed : speed)};

    const float testAngle{angle + ssvu::toRad(currentSpeed * movement * mFT) +
                          movement * padding};
    const sf::Vector2f testPos{
        ssvs::getOrbitRad(startPos, testAngle, mHexagonGame.getRadius())};

    // If there is overlap even after compensation kill without updating
    // position, as there is no benefit in doing it.
    if(wall.isOverlapping(testPos))
    {
        return true;
    }

    // If still alive position player right against the wall to give the
    // illusion it is sliding along it. Since this is a standard wall we can
    // assume the required angle is the angle of vertex 0 or 1 depending on
    // which one is closer to the player.
    const std::array<sf::Vector2f, 4>& wVertexes{wall.getVertexes()};
    const float radZero{ssvs::getRad(wVertexes[0])},
        radOne{ssvs::getRad(wVertexes[1])};
    angle = ssvu::getDistRad(lastAngle, radOne) >
                    ssvu::getDistRad(lastAngle, radZero)
                ? radZero
                : radOne;
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

void CPlayer::updatePosition(
    const HexagonGame& mHexagonGame, const ssvu::FT mFT)
{
    (void)mFT; // Currently unused.

    pos = ssvs::getOrbitRad(startPos, angle, mHexagonGame.getRadius());
}

[[nodiscard]] bool CPlayer::getJustSwapped() const noexcept
{
    return justSwapped;
}

} // namespace hg
