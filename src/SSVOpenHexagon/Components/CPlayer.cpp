// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
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

CPlayer::CPlayer(const sf::Vector2f& mStartPos) noexcept
    : startPos{mStartPos}, pos{startPos}, hue{0}, angle{0},
      size{Config::getPlayerSize()}, speed{Config::getPlayerSpeed()},
      focusSpeed{Config::getPlayerFocusSpeed()}, dead{false}, swapTimer{36.f},
      swapBlinkTimer{5.f}, deadEffectTimer{80.f, false}
{
}

[[nodiscard]] float CPlayer::getPlayerAngle() const
{
    return angle;
}

void CPlayer::setPlayerAngle(const float newAng)
{
    angle = newAng;
}

void CPlayer::draw(HexagonGame& mHexagonGame, const sf::Color& mCapColor,
    const LevelStatus& levelStatus, const StyleData& styleData)
{
    drawPivot(mHexagonGame, mCapColor, levelStatus, styleData);

    if(deadEffectTimer.isRunning())
    {
        drawDeathEffect(mHexagonGame, styleData);
    }

    sf::Color colorMain{!dead ? mHexagonGame.getColorMain()
                              : ssvs::getColorFromHSV(hue / 360.f, 1.f, 1.f)};

    if(!swapTimer.isRunning())
    {
        colorMain = ssvs::getColorFromHSV(
            (swapBlinkTimer.getCurrent() * 15) / 360.f, 1, 1);
    }

    const auto fieldPos{mHexagonGame.getFieldPos()};
    const auto status{mHexagonGame.getStatus()};
    const float playerRadius{mHexagonGame.getRadius()};

    const sf::Vector2f& skew{styleData.skew};
    auto fieldAngle = ssvu::toRad(levelStatus.rotation);
    const auto _angle = angle + fieldAngle;

    // For Drawing
    _pos = Utils::getSkewedOrbitRad(
        fieldPos, fieldAngle + angle, playerRadius, skew);
    pTip = Utils::getSkewedOrbitRad(_pos, _angle, size*ssvu::getSign(playerRadius), skew);
    pLeft = Utils::getSkewedOrbitRad(
        _pos, _angle - ssvu::toRad(100.f), (size + 3)*ssvu::getSign(playerRadius), skew);
    pRight = Utils::getSkewedOrbitRad(
        _pos, _angle + ssvu::toRad(100.f), (size + 3)*ssvu::getSign(playerRadius), skew);

    // For collisions check
    pos = ssvs::getOrbitRad(mHexagonGame.getFieldPos(), angle, playerRadius);
    pDTip = ssvs::getOrbitRad(pos, angle, size*ssvu::getSign(playerRadius));
    pDLeft = ssvs::getOrbitRad(pos, angle - ssvu::toRad(100.f), (size + 3)*ssvu::getSign(playerRadius));
    pDRight = ssvs::getOrbitRad(pos, angle + ssvu::toRad(100.f), (size + 3)*ssvu::getSign(playerRadius));

    // Debug Player itself
    if(Config::getDebug())
    {
        sf::Color colorDebug(0, 0, 255, 150);
        mHexagonGame.playerDebugTris.reserve_more(3);
        mHexagonGame.playerDebugTris.batch_unsafe_emplace_back(
            colorDebug, pDTip, pDLeft, pDRight);
    }

    // Player itself
    mHexagonGame.playerTris.reserve_more(3);
    mHexagonGame.playerTris.batch_unsafe_emplace_back(
        colorMain, pTip, pLeft, pRight);
}

void CPlayer::drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor,
    const LevelStatus& levelStatus, const StyleData& styleData)
{
    const auto status{mHexagonGame.getStatus()};
    const auto sides(mHexagonGame.getSides());
    const float div{ssvu::tau / sides * 0.5f};
    const float radius{abs(mHexagonGame.getRadius() * 0.75f)};

    const sf::Color colorMain{mHexagonGame.getColorMain()};

    const sf::Color colorB{Config::getBlackAndWhite()
                               ? sf::Color::Black
                               : mHexagonGame.getColor(1)};

    const sf::Color colorDarkened{Utils::getColorDarkened(colorMain, 1.4f)};

    const sf::Vector2f& skew{styleData.skew};
    auto fieldAngle = ssvu::toRad(levelStatus.rotation);

    // Cap Graphics
    for(auto i(0u); i < sides; ++i)
    {
        const float sAngle{fieldAngle + div * 2.f * i};

        const sf::Vector2f p1{Utils::getSkewedOrbitRad(
            mHexagonGame.getFieldPos(), sAngle - div, radius, skew)};
        const sf::Vector2f p2{Utils::getSkewedOrbitRad(
            mHexagonGame.getFieldPos(), sAngle + div, radius, skew)};
        const sf::Vector2f p3{
            Utils::getSkewedOrbitRad(mHexagonGame.getFieldPos(), sAngle + div,
                radius + baseThickness, skew)};
        const sf::Vector2f p4{
            Utils::getSkewedOrbitRad(mHexagonGame.getFieldPos(), sAngle - div,
                radius + baseThickness, skew)};

        // Cap's border
        mHexagonGame.wallQuads.reserve_more(4);
        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);

        // Cap
        mHexagonGame.capTris.reserve_more(3);
        mHexagonGame.capTris.batch_unsafe_emplace_back(
            mCapColor, mHexagonGame.getFieldPos(), p1, p2);
    }
}

void CPlayer::drawDeathEffect(
    HexagonGame& mHexagonGame, const StyleData& styleData)
{
    const float div{ssvu::tau / mHexagonGame.getSides() * 0.5f};
    const float radius{hue / 8.f};
    const float thickness{hue / 20.f};

    const auto status{mHexagonGame.getStatus()};

    const sf::Vector2f& skew{styleData.skew};

    const auto fieldAngle = ssvu::toRad(mHexagonGame.getLevelStatus().rotation);

    const sf::Color colorMain{
        ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};

    if(++hue > 360.f)
    {
        hue = 0.f;
    }

    for(auto i(0u); i < mHexagonGame.getSides(); ++i)
    {
        const float sAngle{fieldAngle + angle + div * 2.f * i};

        const sf::Vector2f p1{
            Utils::getSkewedOrbitRad(_pos, sAngle - div, radius, skew)};
        const sf::Vector2f p2{
            Utils::getSkewedOrbitRad(_pos, sAngle + div, radius, skew)};
        const sf::Vector2f p3{Utils::getSkewedOrbitRad(
            _pos, sAngle + div, radius + thickness, skew)};
        const sf::Vector2f p4{Utils::getSkewedOrbitRad(
            _pos, sAngle - div, radius + thickness, skew)};

        mHexagonGame.wallQuads.reserve_more(4);
        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);
    }
}

void CPlayer::swap(HexagonGame& mHexagonGame, bool mSoundTog)
{
    angle += ssvu::pi;
    mHexagonGame.runLuaFunctionIfExists<void>("onCursorSwap");
    if(mSoundTog)
    {
        mHexagonGame.getAssets().playSound("swap.ogg");
    }
}

void CPlayer::update(HexagonGame& mHexagonGame, ssvu::FT mFT)
{
    const auto startPos = mHexagonGame.getFieldPos();
    const sf::Vector2f lastPos{pos};
    float currentSpeed{speed};

    const float lastAngle{angle};
    const float radius{abs(mHexagonGame.getRadius())};
    const int movement{mHexagonGame.getInputMovement()};



    swapBlinkTimer.update(mFT);

    if(deadEffectTimer.update(mFT) &&
        mHexagonGame.getLevelStatus().tutorialMode)
    {
        deadEffectTimer.stop();
    }

    if(mHexagonGame.getLevelStatus().swapEnabled && swapTimer.update(mFT))
    {
        swapTimer.stop();
    }

    if(mHexagonGame.getInputFocused())
    {
        currentSpeed = focusSpeed;
    }

    angle += ssvu::toRad(currentSpeed * movement * mFT);

    if(mHexagonGame.getLevelStatus().swapEnabled &&
        mHexagonGame.getInputSwap() && !swapTimer.isRunning())
    {
        swap(mHexagonGame, true);
        swapTimer.restart();
    }

    // Collisions
    const sf::Vector2f tempPos{ssvs::getOrbitRad(startPos, angle, radius)};
    const sf::Vector2f pLeftCheck{
        ssvs::getOrbitRad(tempPos, angle - ssvu::piHalf, 0.01f)};
    const sf::Vector2f pRightCheck{
        ssvs::getOrbitRad(tempPos, angle + ssvu::piHalf, 0.01f)};

    const auto doCollision = [&](const auto& wall) {
        if((movement == -1 && wall.isOverlapping(pLeftCheck)) ||
            (movement == 1 && wall.isOverlapping(pRightCheck)))
        {
            angle = lastAngle;
        }

        if(wall.isOverlapping(pos))
        {
            deadEffectTimer.restart();

            if(!Config::getInvincible())
            {
                dead = true;
            }

            // TODO:
            // ssvs::moveTowards(lastPos, ssvs::zeroVec2f, 5 *
            // mHexagonGame.getSpeedMultDM());

            pos = lastPos;
            mHexagonGame.death();

            return true;
        }

        return false;
    };

    for(const CWall& wall : mHexagonGame.walls)
    {
        if(doCollision(wall))
        {
            return;
        }
    }

    const bool customWallCollision =
        mHexagonGame.anyCustomWall([&](const CCustomWall& customWall) {
            if(doCollision(customWall))
            {
                return true;
            }

            return false;
        });

    if(customWallCollision)
    {
        return;
    }


    pos = ssvs::getOrbitRad(startPos, angle, radius);
}

} // namespace hg
