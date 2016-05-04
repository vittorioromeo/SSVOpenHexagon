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
using namespace sses;
using namespace ssvs;
using namespace hg::Utils;

namespace hg
{
    constexpr float baseThickness{5.f};

    CPlayer::CPlayer(
        Entity& mE, HexagonGame& mHexagonGame, const Vec2f& mStartPos)
        : Component{mE}, hexagonGame(mHexagonGame), startPos{mStartPos},
          pos{startPos}
    {
    }

    void CPlayer::draw()
    {
        drawPivot();

        if(deadEffectTimer.isRunning()) drawDeathEffect();

        Color colorMain{!dead ? hexagonGame.getColorMain()
                              : ssvs::getColorFromHSV(hue / 360.f, 1.f, 1.f)};

        pLeft = getOrbitRad(pos, angle - toRad(100.f), size + 3);
        pRight = getOrbitRad(pos, angle + toRad(100.f), size + 3);

        if(!swapTimer.isRunning())
            colorMain = ssvs::getColorFromHSV(
                (swapBlinkTimer.getCurrent() * 15) / 360.f, 1, 1);

        hexagonGame.playerTris.emplace_back(
            getOrbitRad(pos, angle, size), colorMain);
        hexagonGame.playerTris.emplace_back(pLeft, colorMain);
        hexagonGame.playerTris.emplace_back(pRight, colorMain);
    }
    void CPlayer::drawPivot()
    {
        auto sides(hexagonGame.getSides());
        float div{ssvu::tau / sides * 0.5f},
            radius{hexagonGame.getRadius() * 0.75f};
        Color colorMain{hexagonGame.getColorMain()},
            colorB{hexagonGame.getColor(1)};
        if(Config::getBlackAndWhite()) colorB = Color::Black;

        for(auto i(0u); i < sides; ++i)
        {
            float sAngle{div * 2.f * i};

            Vec2f p1{getOrbitRad(startPos, sAngle - div, radius)};
            Vec2f p2{getOrbitRad(startPos, sAngle + div, radius)};
            Vec2f p3{
                getOrbitRad(startPos, sAngle + div, radius + baseThickness)};
            Vec2f p4{
                getOrbitRad(startPos, sAngle - div, radius + baseThickness)};

            hexagonGame.wallQuads.emplace_back(p1, colorMain);
            hexagonGame.wallQuads.emplace_back(p2, colorMain);
            hexagonGame.wallQuads.emplace_back(p3, colorMain);
            hexagonGame.wallQuads.emplace_back(p4, colorMain);

            hexagonGame.playerTris.emplace_back(p1, colorB);
            hexagonGame.playerTris.emplace_back(p2, colorB);
            hexagonGame.playerTris.emplace_back(startPos, colorB);
        }
    }
    void CPlayer::drawDeathEffect()
    {
        float div{ssvu::tau / hexagonGame.getSides() * 0.5f}, radius{hue / 8.f},
            thickness{hue / 20.f};
        Color colorMain{ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};
        if(++hue > 360.f) hue = 0.f;

        for(auto i(0u); i < hexagonGame.getSides(); ++i)
        {
            float sAngle{div * 2.f * i};

            Vec2f p1{getOrbitRad(pos, sAngle - div, radius)};
            Vec2f p2{getOrbitRad(pos, sAngle + div, radius)};
            Vec2f p3{getOrbitRad(pos, sAngle + div, radius + thickness)};
            Vec2f p4{getOrbitRad(pos, sAngle - div, radius + thickness)};

            hexagonGame.wallQuads.emplace_back(p1, colorMain);
            hexagonGame.wallQuads.emplace_back(p2, colorMain);
            hexagonGame.wallQuads.emplace_back(p3, colorMain);
            hexagonGame.wallQuads.emplace_back(p4, colorMain);
        }
    }

    void CPlayer::update(FT mFT)
    {
        swapBlinkTimer.update(mFT);
        if(deadEffectTimer.update(mFT) &&
            hexagonGame.getLevelStatus().tutorialMode)
            deadEffectTimer.stop();
        if(hexagonGame.getLevelStatus().swapEnabled)
            if(swapTimer.update(mFT)) swapTimer.stop();

        Vec2f lastPos{pos};
        float currentSpeed{speed}, lastAngle{angle},
            radius{hexagonGame.getRadius()};
        int movement{hexagonGame.getInputMovement()};
        if(hexagonGame.getInputFocused()) currentSpeed = focusSpeed;

        angle += toRad(currentSpeed * movement * mFT);

        if(hexagonGame.getLevelStatus().swapEnabled &&
            hexagonGame.getInputSwap() && !swapTimer.isRunning())
        {
            hexagonGame.getAssets().playSound("swap.ogg");
            swapTimer.restart();
            angle += ssvu::pi;
            hexagonGame.runLuaFunctionIfExists<void>("onCursorSwap");
        }

        Vec2f tempPos{getOrbitRad(startPos, angle, radius)};
        Vec2f pLeftCheck{getOrbitRad(tempPos, angle - ssvu::piHalf, 0.01f)};
        Vec2f pRightCheck{getOrbitRad(tempPos, angle + ssvu::piHalf, 0.01f)};

        for(const auto& wall : getManager().getEntities(HGGroup::Wall))
        {
            const auto& cwall(wall->getComponent<CWall>());
            if((movement == -1 && cwall.isOverlapping(pLeftCheck)) ||
                (movement == 1 && cwall.isOverlapping(pRightCheck)))
                angle = lastAngle;
            if(cwall.isOverlapping(pos))
            {
                deadEffectTimer.restart();
                if(!Config::getInvincible()) dead = true;
                moveTowards(
                    lastPos, ssvs::zeroVec2f, 5 * hexagonGame.getSpeedMultDM());
                pos = lastPos;
                hexagonGame.death();
                return;
            }
        }

        pos = getOrbitRad(startPos, angle, radius);
    }
}
