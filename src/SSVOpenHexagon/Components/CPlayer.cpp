// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"

using namespace std;
using namespace sf;
using namespace ssvu;
using namespace ssvs;
using namespace hg::Utils;

namespace hg
{

constexpr float baseThickness{5.f};

CPlayer::CPlayer(const Vec2f& mStartPos) noexcept
    : startPos{mStartPos}, pos{startPos}
{
}

float CPlayer::getPlayerAngle()
{
    return angle;
}

void CPlayer::setPlayerAngle(float newAng)
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

    Color colorMain{!dead ? mHexagonGame.getColorMain()
                          : ssvs::getColorFromHSV(hue / 360.f, 1.f, 1.f)};

    pLeft = getOrbitRad(pos, angle - toRad(100.f), size + 3);
    pRight = getOrbitRad(pos, angle + toRad(100.f), size + 3);

    if(!swapTimer.isRunning())
    {
        colorMain = ssvs::getColorFromHSV(
            (swapBlinkTimer.getCurrent() * 15) / 360.f, 1, 1);
    }

    mHexagonGame.playerTris.reserve_more(3);
    mHexagonGame.playerTris.batch_unsafe_emplace_back(
        colorMain, getOrbitRad(pos, angle, size), pLeft, pRight);
}

void CPlayer::drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor)
{
    const auto sides(mHexagonGame.getSides());
    const float div{ssvu::tau / sides * 0.5f};
    const float radius{mHexagonGame.getRadius() * 0.75f};
    const Color colorMain{mHexagonGame.getColorMain()};
    const Color colorB{
        Config::getBlackAndWhite() ? Color::Black : mHexagonGame.getColor(1)};
    const Color colorDarkened{getColorDarkened(colorMain, 1.4f)};

    for(auto i(0u); i < sides; ++i)
    {
        const float sAngle{div * 2.f * i};

        const Vec2f p1{getOrbitRad(startPos, sAngle - div, radius)};
        const Vec2f p2{getOrbitRad(startPos, sAngle + div, radius)};
        const Vec2f p3{
            getOrbitRad(startPos, sAngle + div, radius + baseThickness)};
        const Vec2f p4{
            getOrbitRad(startPos, sAngle - div, radius + baseThickness)};

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
    const Color colorMain{
        ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};

    if(++hue > 360.f)
    {
        hue = 0.f;
    }

    for(auto i(0u); i < mHexagonGame.getSides(); ++i)
    {
        const float sAngle{div * 2.f * i};

        Vec2f p1{getOrbitRad(pos, sAngle - div, radius)};
        Vec2f p2{getOrbitRad(pos, sAngle + div, radius)};
        Vec2f p3{getOrbitRad(pos, sAngle + div, radius + thickness)};
        Vec2f p4{getOrbitRad(pos, sAngle - div, radius + thickness)};

        mHexagonGame.wallQuads.reserve_more(4);
        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);
    }
}

void CPlayer::update(HexagonGame& mHexagonGame, FT mFT)
{
    swapBlinkTimer.update(mFT);

    if(deadEffectTimer.update(mFT) &&
        mHexagonGame.getLevelStatus().tutorialMode)
    {
        deadEffectTimer.stop();
    }

    if(mHexagonGame.getLevelStatus().swapEnabled)
    {
        if(swapTimer.update(mFT))
        {
            swapTimer.stop();
        }
    }

    Vec2f lastPos{pos};
    float currentSpeed{speed};

    const float lastAngle{angle};
    const float radius{mHexagonGame.getRadius()};
    const int movement{mHexagonGame.getInputMovement()};

    if(mHexagonGame.getInputFocused())
    {
        currentSpeed = focusSpeed;
    }

    angle += toRad(currentSpeed * movement * mFT);

    if(mHexagonGame.getLevelStatus().swapEnabled &&
        mHexagonGame.getInputSwap() && !swapTimer.isRunning())
    {
        mHexagonGame.getAssets().playSound("swap.ogg");
        swapTimer.restart();
        angle += ssvu::pi;
        mHexagonGame.runLuaFunctionIfExists<void>("onCursorSwap");
    }

    const Vec2f tempPos{getOrbitRad(startPos, angle, radius)};
    const Vec2f pLeftCheck{getOrbitRad(tempPos, angle - ssvu::piHalf, 0.01f)};
    const Vec2f pRightCheck{getOrbitRad(tempPos, angle + ssvu::piHalf, 0.01f)};

    for(const auto& wall : mHexagonGame.walls)
    {
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

            moveTowards(
                lastPos, ssvs::zeroVec2f, 5 * mHexagonGame.getSpeedMultDM());

            pos = lastPos;
            mHexagonGame.death();

            return;
        }
    }

    pos = getOrbitRad(startPos, angle, radius);
}

} // namespace hg
