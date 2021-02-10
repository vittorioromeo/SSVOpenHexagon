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
    : startPos{mPos}, pos{mPos}, hue{0}, angle{0}, lastAngle{0},
      size{Config::getPlayerSize()}, speed{Config::getPlayerSpeed()},
      focusSpeed{Config::getPlayerFocusSpeed()}, dead{false},
      justSwapped{false}, swapTimer{swapCooldown},
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

inline constexpr float collisionPadding{0.1f};

template <typename Wall>
[[nodiscard]] bool CPlayer::checkWallCollisionEscape(const Wall& wall, sf::Vector2f& mPos)
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

    const auto getLineCircleIntersection = [this](sf::Vector2f& i1,
        sf::Vector2f& i2, const sf::Vector2f& p1,
        const sf::Vector2f& p2) -> unsigned int
    {
        const float dx{p2.x - p1.x};
        const float dy{p2.y - p1.y};
        const float a{dx * dx + dy * dy};
        const float b{2.f * (dx * p1.x + dy * p1.y)};
        const float c{p1.x * p1.x + p1.y * p1.y - radius * radius};
        const float delta{b * b - 4.f * a * c};

        // No intersections.
        if(delta < 0.f)
        {
            return 0u;
        }

        float t;
        const float twoA{2.f * a};
        
        // One intersection.
        if(delta < hg::Utils::epsilon)
        {
            t = -b / twoA;
            i1 = {p1.x + t * dx, p1.y + t * dy};
            return 1u;
        }

        // Two intersections.
        const float sqrtDelta{hg::Utils::fastSqrt(delta)};
        t = (-b + sqrtDelta) / twoA;
        i1 = {p1.x + t * dx, p1.y + t * dy};
        t = (-b - sqrtDelta) / twoA;
        i2 = {p1.x + t * dx, p1.y + t * dy};
        return 2u;
    };

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
        switch(getLineCircleIntersection(vec1, vec2, wVertexes[i], wVertexes[j]))
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
    const CWall& wall, const sf::Vector2f& mCenterPos, const ssvu::FT mFT)
{
    if(dead)
    {
        return false;
    }

    sf::Vector2f testPos{pos};
    sf::Vector2f velocity{0.f, 0.f};
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
        testPos = ssvs::getOrbitRad(startPos, ssvs::getRad(testPos), radius);
        velocity = testPos - pos;
    }

    // If player is not moving calculate now...
    if(!movement)
    {
        pos = testPos + ssvs::getNormalized(testPos - pos) * collisionPadding;
        lastAngle = angle = ssvs::getRad(pos);
        updatePosition(mHexagonGame, mFT);
        return wall.isOverlapping(pos);
    }

    // ...otherwise make testPos the position of the previous frame plus
    // the curving wall's velocity, and check an escape on that position
    // Using the previous frame's position is essential for levels with
    // a really high amount of sides.
    testPos = ssvs::getOrbitRad(startPos, lastAngle, radius) + velocity;
    if(!checkWallCollisionEscape<CWall>(wall, testPos))
    {
        return true;
    }

    // If player survived apply test position and add a little padding.
    pos = testPos + ssvs::getNormalized(testPos - pos) * collisionPadding;
    lastAngle = angle = ssvs::getRad(pos);
    updatePosition(mHexagonGame, mFT);
    return false;
}

inline constexpr float sideSlideThreshold{0.8f};

[[nodiscard]] bool CPlayer::push(const HexagonGame& mHexagonGame,
    const CCustomWall& wall, const ssvu::FT mFT)
{
    (void)mFT; // Currently unused.

    if(dead)
    {
        return false;
    }

    sf::Vector2f lastPos{ssvs::getOrbitRad(startPos, lastAngle, radius)};
    sf::Vector2f testPos{(pos + lastPos) / 2.f}; // I found this to give best results.
    sf::Vector2f velocity{0.f, 0.f};

    // A custom wall can take any shape and its sides can move however
    // they want. To check if player should be moved like it occurs with
    // rotating standard walls see if testPos fits within the trapezoid
    // delimited by the wall's old and new positions of the vertexes.
    std::array<const sf::Vector2f*, 4> collisionPolygon;
    const std::array<sf::Vector2f, 4>& wVertexes{wall.getVertexPositions()};
    const std::array<sf::Vector2f, 4>& wOldVertexes{wall.getOldVertexPositions()};

    for(unsigned int i{0u}, j{3u}; i < 4u; j = i++)
    {
        collisionPolygon =
            {&wVertexes[i], &wOldVertexes[i], &wOldVertexes[j], &wVertexes[j]};
        if(!pointInPolygonPointers(collisionPolygon, testPos.x, testPos.y))
        {
            continue;
        }

        // Average velocity of the vertexes.
        velocity = 
            ((wVertexes[i] - wOldVertexes[i]) + (wVertexes[j] - wOldVertexes[j])) / 2.f;

        // Not a side player can slide against.
        if(std::abs(ssvs::getDotProduct(ssvs::getNormalized(velocity),
            ssvs::getNormalized(pos))) < sideSlideThreshold)
        {
            return true;
        }

        break;
    }

    // If player is not moving calculate now...
    if(!mHexagonGame.getInputMovement())
    {
        pos += velocity + ssvs::getNormalized(testPos - pos) * collisionPadding;
        lastAngle = angle = ssvs::getRad(pos);
        updatePosition(mHexagonGame, mFT);
        return wall.isOverlapping(pos);
    }

    // ...otherwise try to find a close enough safe position.
    testPos = lastPos + velocity;
    if(!checkWallCollisionEscape<CCustomWall>(wall, testPos))
    {
        return true;
    }

    // If player survived apply test position and add a little padding.
    pos = testPos + ssvs::getNormalized(testPos - pos) * collisionPadding;
    lastAngle = angle = ssvs::getRad(pos);
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
}

void CPlayer::updateInput(HexagonGame& mHexagonGame, const ssvu::FT mFT)
{
    const int movement{mHexagonGame.getInputMovement()};
    currentSpeed = mHexagonGame.getPlayerSpeedMult() *
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
    (void)mHexagonGame;

    pos = ssvs::getOrbitRad(startPos, angle, radius);
}

void CPlayer::updateCollisionValues(
    const HexagonGame& mHexagonGame, const ssvu::FT mFT)
{
    radius = mHexagonGame.getRadius();
    maxSafeDistance = currentSpeed * (2.f * mFT);
    maxSafeDistance = maxSafeDistance * maxSafeDistance + 32.f;
}

[[nodiscard]] bool CPlayer::getJustSwapped() const noexcept
{
    return justSwapped;
}

} // namespace hg
