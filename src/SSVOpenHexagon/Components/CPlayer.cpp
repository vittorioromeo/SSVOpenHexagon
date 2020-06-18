// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"

using namespace std;
using namespace sf;
using namespace ssvu;
using namespace ssvs;
using namespace hg::Utils;

namespace hg
{

constexpr float baseThickness{5.f};

CPlayer::CPlayer(HexagonGame& mHexagonGame, const Vec2f& mStartPos)
    : hexagonGame(&mHexagonGame), startPos{mStartPos}, pos{startPos}
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

void CPlayer::draw()
{
    drawPivot();

    if(deadEffectTimer.isRunning())
    {
        drawDeathEffect();
    }

    Color colorMain{!dead ? hexagonGame->getColorMain()
                          : ssvs::getColorFromHSV(hue / 360.f, 1.f, 1.f)};

    pLeft = getOrbitRad(pos, angle - toRad(100.f), size + 3);
    pRight = getOrbitRad(pos, angle + toRad(100.f), size + 3);

    if(!swapTimer.isRunning())
    {
        colorMain = ssvs::getColorFromHSV(
            (swapBlinkTimer.getCurrent() * 15) / 360.f, 1, 1);
    }

    hexagonGame->playerTris.emplace_back(
        getOrbitRad(pos, angle, size), colorMain);
    hexagonGame->playerTris.emplace_back(pLeft, colorMain);
    hexagonGame->playerTris.emplace_back(pRight, colorMain);
}

void CPlayer::drawPivot()
{
    auto sides(hexagonGame->getSides());
    const float div{ssvu::tau / sides * 0.5f};
    const float radius{hexagonGame->getRadius() * 0.75f};
    const Color colorMain{hexagonGame->getColorMain()};
    const Color colorB{
        Config::getBlackAndWhite() ? Color::Black : hexagonGame->getColor(1)};
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

        hexagonGame->wallQuads.emplace_back(p1, colorMain);
        hexagonGame->wallQuads.emplace_back(p2, colorMain);
        hexagonGame->wallQuads.emplace_back(p3, colorMain);
        hexagonGame->wallQuads.emplace_back(p4, colorMain);

        hexagonGame->capTris.emplace_back(p1, colorDarkened);
        hexagonGame->capTris.emplace_back(p2, colorDarkened);
        hexagonGame->capTris.emplace_back(startPos, colorDarkened);
    }
}

void CPlayer::drawDeathEffect()
{
    float div{ssvu::tau / hexagonGame->getSides() * 0.5f};
    float radius{hue / 8.f};
    float thickness{hue / 20.f};
    Color colorMain{ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};
    if(++hue > 360.f)
    {
        hue = 0.f;
    }

    for(auto i(0u); i < hexagonGame->getSides(); ++i)
    {
        float sAngle{div * 2.f * i};

        Vec2f p1{getOrbitRad(pos, sAngle - div, radius)};
        Vec2f p2{getOrbitRad(pos, sAngle + div, radius)};
        Vec2f p3{getOrbitRad(pos, sAngle + div, radius + thickness)};
        Vec2f p4{getOrbitRad(pos, sAngle - div, radius + thickness)};

        hexagonGame->wallQuads.emplace_back(p1, colorMain);
        hexagonGame->wallQuads.emplace_back(p2, colorMain);
        hexagonGame->wallQuads.emplace_back(p3, colorMain);
        hexagonGame->wallQuads.emplace_back(p4, colorMain);
    }
}

void CPlayer::update(FT mFT)
{
    swapBlinkTimer.update(mFT);
    if(deadEffectTimer.update(mFT) &&
        hexagonGame->getLevelStatus().tutorialMode)
    {
        deadEffectTimer.stop();
    }
    if(hexagonGame->getLevelStatus().swapEnabled)
    {
        if(swapTimer.update(mFT))
        {
            swapTimer.stop();
        }
    }

    Vec2f lastPos{pos};
    float currentSpeed{speed};
    float lastAngle{angle};
    float radius{hexagonGame->getRadius()};
    int movement{hexagonGame->getInputMovement()};
    if(hexagonGame->getInputFocused())
    {
        currentSpeed = focusSpeed;
    }

    angle += toRad(currentSpeed * movement * mFT);

    if(hexagonGame->getLevelStatus().swapEnabled &&
        hexagonGame->getInputSwap() && !swapTimer.isRunning())
    {
        hexagonGame->getAssets().playSound("swap.ogg");
        swapTimer.restart();
        angle += ssvu::pi;
        hexagonGame->runLuaFunctionIfExists<void>("onCursorSwap");
    }

    Vec2f tempPos{getOrbitRad(startPos, angle, radius)};
    Vec2f pLeftCheck{getOrbitRad(tempPos, angle - ssvu::piHalf, 0.01f)};
    Vec2f pRightCheck{getOrbitRad(tempPos, angle + ssvu::piHalf, 0.01f)};

    for(const auto& wall : hexagonGame->walls)
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
                lastPos, ssvs::zeroVec2f, 5 * hexagonGame->getSpeedMultDM());
            pos = lastPos;
            hexagonGame->death();

            return;
        }
    }

    pos = getOrbitRad(startPos, angle, radius);
}

} // namespace hg
