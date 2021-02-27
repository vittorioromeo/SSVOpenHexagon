// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Utils/Ticker.hpp"
#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"

#include "SSVOpenHexagon/Global/Config.hpp"

#include <SSVStart/Utils/SFML.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVUtils/Core/Utils/Math.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

using namespace hg::Utils;

namespace hg
{

inline constexpr float baseThickness{5.f};

CPlayer::CPlayer(const sf::Vector2f& mPos, const float swapCooldown) noexcept
    : startPos{mPos}, pos{mPos}, prePushPos{mPos}, lastPos{mPos},
      hue{0}, angle{0}, lastAngle{0}, size{Config::getPlayerSize()},
      speed{Config::getPlayerSpeed()}, focusSpeed{Config::getPlayerFocusSpeed()},
      dead{false}, justSwapped{false}, forcedMove{false}, swapTimer{swapCooldown},
      swapBlinkTimer{swapCooldown / 6.f}, deadEffectTimer{80.f, false}
{
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

    const float triangleWidth{mHexagonGame.getInputFocused() ? -1.5f : 3.f};

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
    const float pRadius{radius * 0.75f};

    const sf::Color colorMain{mHexagonGame.getColorMain()};

    const sf::Color colorB{Config::getBlackAndWhite()
                               ? sf::Color::Black
                               : mHexagonGame.getColor(1)};

    const sf::Color colorDarkened{Utils::getColorDarkened(colorMain, 1.4f)};

    for(auto i(0u); i < sides; ++i)
    {
        const float sAngle{div * 2.f * i};

        const sf::Vector2f p1{
            ssvs::getOrbitRad(startPos, sAngle - div, pRadius)};
        const sf::Vector2f p2{
            ssvs::getOrbitRad(startPos, sAngle + div, pRadius)};
        const sf::Vector2f p3{
            ssvs::getOrbitRad(startPos, sAngle + div, pRadius + baseThickness)};
        const sf::Vector2f p4{
            ssvs::getOrbitRad(startPos, sAngle - div, pRadius + baseThickness)};

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
    const float dRadius{hue / 8.f};
    const float thickness{hue / 20.f};

    const sf::Color colorMain{
        ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};

    for(auto i(0u); i < mHexagonGame.getSides(); ++i)
    {
        const float sAngle{div * 2.f * i};

        const sf::Vector2f p1{ssvs::getOrbitRad(pos, sAngle - div, dRadius)};
        const sf::Vector2f p2{ssvs::getOrbitRad(pos, sAngle + div, dRadius)};
        const sf::Vector2f p3{
            ssvs::getOrbitRad(pos, sAngle + div, dRadius + thickness)};
        const sf::Vector2f p4{
            ssvs::getOrbitRad(pos, sAngle - div, dRadius + thickness)};

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
}

inline constexpr float collisionPadding{0.5f};

template <typename Wall>
[[nodiscard]] bool CPlayer::checkWallCollisionEscape(const Wall& wall,
    sf::Vector2f& mPos, const float mRadiusSquared)
{
    // To find the closest wall side we intersect the circumference of the possible
    // player positions with the sides of the wall. We use the intersection closest
    // to the player's position as post collision target.
    // If an escape route could not be found player is killed.

    bool saved{false};
    sf::Vector2f vec1, vec2;
    float tempDistance, safeDistance{maxSafeDistance};
    const unsigned int vxIncrement{wall.isCustomWall() ? 1u : 2u};
    const std::array<sf::Vector2f, 4>& wVertexes{wall.getVertexPositions()};

    // This is actually useless for normal walls, but if we removed
    // getKillingSide() for CWall we would have to write a separate
    // function just to do without this variable,
    const unsigned int killingSide{wall.getKillingSide()};

    const auto assignResult = [&]() {
        tempDistance = ssvs::getMagSquared(vec1 - mPos);
        if(tempDistance < safeDistance)
        {
            mPos = vec1;
            saved = true;
            safeDistance = tempDistance;
        }
    };

    for(unsigned int i{0u}, j{3u}; i < 4u; i += vxIncrement, j = i - 1)
    {
        if(j == killingSide)
        {
            continue;
        }

        switch(getLineCircleIntersection(vec1, vec2, wVertexes[i], wVertexes[j], mRadiusSquared))
        {
            case 1u:
                assignResult();
                break;

            case 2u:
                if(ssvs::getMagSquared(vec1 - mPos) >
                    ssvs::getMagSquared(vec2 - mPos))
                {
                    vec1 = vec2;
                }
                assignResult();
                break;

            default:
                break;
        }
    }

    return saved;
}

[[nodiscard]] bool CPlayer::push(const HexagonGame& mHexagonGame,
    const CWall& wall, const sf::Vector2f& mCenterPos,
    const float mRadiusSquared, const ssvu::FT mFT)
{
    if(dead)
    {
        return false;
    }

    sf::Vector2f testPos{pos};
    sf::Vector2f pushVel{0.f, 0.f};
    const int movement{mHexagonGame.getInputMovement()};

    // If it's a rotating wall push player in the direction the
    // wall is rotating by the appropriate amount, but only if the direction
    // of the rotation is different from the direction player is moving.
    // Save the position difference in case we need to do a second attempt
    // at saving player.
    const SpeedData& curveData{wall.getCurve()};
    if(curveData.speed != 0.f && ssvu::getSign(curveData.speed) != movement)
    {
        wall.moveVertexAlongCurve(testPos, mCenterPos, mFT);
        pushVel = testPos - pos;
    }

    // If player is not moving calculate now...
    if(!movement && !forcedMove)
    {
        pos = testPos + ssvs::getNormalized(testPos - prePushPos) *
            (2.f * collisionPadding);
        angle = ssvs::getRad(pos);
        updatePosition(mHexagonGame, mFT);
        return wall.isOverlapping(pos);
    }

    // ...otherwise make testPos the position of the previous frame plus
    // the curving wall's velocity, and check an escape on that position.
    // Using the previous frame's position is essential for levels with
    // a really high amount of sides. Player might be currently positioned
    // closer to the side opposite to the one it should intuitively slide againts
    //
    //   BEFORE               AFTER
    //
    //  |       |           |       |
    //  |       |           |       |
    //  |       |   *       |  *    | <- this should be our target wall
    //  |       |  * *      | * *   |    but if we use the current position
    //  |       | *****     |*****  |    it is the other one that is closer
    //  |       |           |       |
    testPos = lastPos + pushVel;
    if(wall.isOverlapping(testPos) ||
       !checkWallCollisionEscape(wall, testPos, mRadiusSquared))
    {
        return true;
    }

    // If player survived apply test position and add a little padding.
    pos = testPos + ssvs::getNormalized(testPos - prePushPos) * collisionPadding;
    angle = ssvs::getRad(pos);
    updatePosition(mHexagonGame, mFT);
    return false;
}

[[nodiscard]] bool CPlayer::push(const HexagonGame& mHexagonGame,
    const CCustomWall& wall, const float mRadiusSquared, const ssvu::FT mFT)
{
    (void)mFT; // Currently unused.

    if(dead)
    {
        return false;
    }
    if(wall.getDeadly())
    {
        return true;
    }

    // Look for a side that is moving in a direction perpendicular to the player,
    // such side is a candidate for pushing it like a curving wall would do.
    // (lastPos is the best candidate for this check).
    std::array<const sf::Vector2f*, 4> collisionPolygon;
    const std::array<sf::Vector2f, 4>& wVertexes{wall.getVertexPositions()};
    const std::array<sf::Vector2f, 4>& wOldVertexes{wall.getOldVertexPositions()};
    sf::Vector2f pushVel{0.f, 0.f}, i1, i2;
    const unsigned int killingSide{wall.getKillingSide()};
    constexpr float pushDotThreshold{0.15f}; // 0.1 would be enough in most scenarios
                                             // but we raise it to 0.15 for really fast walls.

    for(unsigned int i{0u}, j{3u}; i < 4; j = i++)
    {
        if(j == killingSide)
        {
            continue;
        }

        collisionPolygon =
            {&wVertexes[i], &wOldVertexes[i], &wOldVertexes[j], &wVertexes[j]};

        if(pointInPolygonPointers(collisionPolygon, lastPos.x, lastPos.y))
        {
            // For a side to be an effective source of push it must have intersected
            // the player's positions circle both now and the previous frame.
            if(getLineCircleClosestIntersection(
                   i1, lastPos, wOldVertexes[i], wOldVertexes[j], mRadiusSquared) &&
               getLineCircleClosestIntersection(
                   i2, lastPos, wVertexes[i], wVertexes[j], mRadiusSquared))
            {
                pushVel = i2 - i1;
                if(std::abs(ssvs::getDotProduct(ssvs::getNormalized(pushVel),
                    ssvs::getNormalized(lastPos))) > pushDotThreshold)
                {
                    pushVel = {0.f, 0.f};
                }
            }
            break;
        }   
    }

    // Player is not moving exit now.
    if(!mHexagonGame.getInputMovement() && !forcedMove)
    {
        pos += pushVel;
        pos += ssvs::getNormalized(pos - prePushPos) *
            (2.f * collisionPadding);
        angle = ssvs::getRad(pos);
        updatePosition(mHexagonGame, mFT);
        return wall.isOverlapping(pos);
    }

    // If alive try to find a close enough safe position.
    sf::Vector2f testPos{lastPos + pushVel};
    if(wall.isOverlapping(testPos) ||
       !checkWallCollisionEscape(wall, testPos, mRadiusSquared))
    {
        return true;
    }

    // If player survived assign it the saving testPos, but displace it further out
    // the wall border, otherwise player would be lying right on top of the border.
    pos = testPos + ssvs::getNormalized(testPos - prePushPos) * collisionPadding;
    angle = ssvs::getRad(pos);
    updatePosition(mHexagonGame, mFT);
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

    lastAngle = angle;
    forcedMove = false;
}

void CPlayer::updateInput(HexagonGame& mHexagonGame, const ssvu::FT mFT)
{
    const int movement{mHexagonGame.getInputMovement()};
    currentSpeed = mHexagonGame.getPlayerSpeedMult() *
        (mHexagonGame.getInputFocused() ? focusSpeed : speed) * mFT;
    angle += ssvu::toRad(currentSpeed * movement);

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
    (void)mHexagonGame;

    radius = mHexagonGame.getRadius();

    prePushPos = pos = ssvs::getOrbitRad(startPos, angle, radius);
    lastPos = ssvs::getOrbitRad(startPos, lastAngle, radius);

    maxSafeDistance = ssvs::getMagSquared(lastPos - ssvs::getOrbitRad(
        startPos, lastAngle + ssvu::toRad(currentSpeed), radius)) + 32.f;
}

[[nodiscard]] bool CPlayer::getJustSwapped() const noexcept
{
    return justSwapped;
}

} // namespace hg
