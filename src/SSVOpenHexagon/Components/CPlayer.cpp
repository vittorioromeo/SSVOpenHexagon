// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"

#include "SSVOpenHexagon/Global/Config.hpp"

#include "SSVStart/Utils/SFML.hpp"
#include "SSVStart/Utils/Vector2.hpp"

#include "SSVUtils/Ticker/Ticker.hpp"
#include "SSVUtils/Core/Common/Frametime.hpp"
#include "SSVUtils/Core/Utils/Math.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace hg
{

constexpr float baseThickness{5.f};

CPlayer::CPlayer(const sf::Vector2f& mStartPos) noexcept
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
    /*
    pTip = ssvs::getOrbitRad(pos, angle, size);
    pLeft = ssvs::getOrbitRad(pos, angle - ssvu::toRad(100.f), size + 3);
    pRight = ssvs::getOrbitRad(pos, angle + ssvu::toRad(100.f), size + 3);
    */
    const auto status{mHexagonGame.getStatus()};
    const sf::Vector2f skewEffect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D,
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D
    };
    const sf::Vector2f skew{1.f, 1.f+skewEffect.y};
    auto fieldAngle = ssvu::toRad(levelStatus.rotation);
    const auto _angle = angle + fieldAngle;
    pTip   = {_pos.x + std::cos(_angle) * size,
              _pos.y + std::sin(_angle) * (size/skew.y)};
    pLeft  = {_pos.x + std::cos(_angle - ssvu::toRad(100.f)) * (size+3),
              _pos.y + std::sin(_angle - ssvu::toRad(100.f)) * ((size+3)/skew.y)};
    pRight = {_pos.x + std::cos(_angle + ssvu::toRad(100.f)) * (size+3),
              _pos.y + std::sin(_angle + ssvu::toRad(100.f)) * ((size+3)/skew.y)};

    pDTip   = {pos.x + std::cos(angle) * size,
               pos.y + std::sin(angle) * size};
    pDLeft  = {pos.x + std::cos(angle - ssvu::toRad(100.f)) * (size+3),
               pos.y + std::sin(angle - ssvu::toRad(100.f)) * (size+3)};
    pDRight = {pos.x + std::cos(angle + ssvu::toRad(100.f)) * (size+3),
               pos.y + std::sin(angle + ssvu::toRad(100.f)) * (size+3)};

    //Debug Player itself
    if (Config::getDebug()) {
        sf::Color colorDebug(0, 0, 255, 150);
        mHexagonGame.playerDebugTris.reserve_more(3);
        mHexagonGame.playerDebugTris.batch_unsafe_emplace_back(
            colorDebug,
            pDTip,
            pDLeft,
            pDRight);
    }

    //Player itself
    mHexagonGame.playerTris.reserve_more(3);
    mHexagonGame.playerTris.batch_unsafe_emplace_back(
        colorMain,
        pTip,
        pLeft,
        pRight);
}

void CPlayer::drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor,
                        const LevelStatus& levelStatus, const StyleData& styleData)
{
    const auto status{mHexagonGame.getStatus()};
    const auto sides(mHexagonGame.getSides());
    const float div{ssvu::tau / sides * 0.5f};
    const float radius{mHexagonGame.getRadius() * 0.75f};

    const sf::Color colorMain{mHexagonGame.getColorMain()};

    const sf::Color colorB{Config::getBlackAndWhite()
                               ? sf::Color::Black
                               : mHexagonGame.getColor(1)};

    const sf::Color colorDarkened{Utils::getColorDarkened(colorMain, 1.4f)};


    auto fieldAngle = ssvu::toRad(levelStatus.rotation);
    const sf::Vector2f skewEffect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D,
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D
    };
    const sf::Vector2f skew{1.f, 1.f+skewEffect.y};
    //Cap Borders
    for(auto i(0u); i < sides; ++i)
    {
        const float sAngle{fieldAngle + div * 2.f * i};

        const sf::Vector2f p1{mHexagonGame.getFieldPos().x + std::cos(sAngle - div) * radius,
                              mHexagonGame.getFieldPos().y + std::sin(sAngle - div) * (radius/skew.y)};
        const sf::Vector2f p2{mHexagonGame.getFieldPos().x + std::cos(sAngle + div) * radius,
                              mHexagonGame.getFieldPos().y + std::sin(sAngle + div) * (radius/skew.y)};
        const sf::Vector2f p3{mHexagonGame.getFieldPos().x + std::cos(sAngle + div) * (radius + baseThickness),
                              mHexagonGame.getFieldPos().y + std::sin(sAngle + div) * ((radius + baseThickness)/skew.y)};
        const sf::Vector2f p4{mHexagonGame.getFieldPos().x + std::cos(sAngle - div) * (radius + baseThickness),
                              mHexagonGame.getFieldPos().y + std::sin(sAngle - div) * ((radius + baseThickness)/skew.y)};

        mHexagonGame.wallQuads.reserve_more(4);
        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain,
            p1,
            p2,
            p3,
            p4);

        mHexagonGame.capTris.reserve_more(3);
        mHexagonGame.capTris.batch_unsafe_emplace_back(
            mCapColor,
            mHexagonGame.getFieldPos(),
            p1,
            p2);
    }
}

void CPlayer::drawDeathEffect(HexagonGame& mHexagonGame, const StyleData& styleData)
{
    const float div{ssvu::tau / mHexagonGame.getSides() * 0.5f};
    const float radius{hue / 8.f};
    const float thickness{hue / 20.f};

    const sf::Color colorMain{
        ssvs::getColorFromHSV((360.f - hue) / 360.f, 1.f, 1.f)};

    if(++hue > 360.f)
    {
        hue = 0.f;
    }

    const auto status{mHexagonGame.getStatus()};
    const sf::Vector2f skewEffect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D,
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D
    };
    const sf::Vector2f skew{1.f, 1.f+skewEffect.y};

    for(auto i(0u); i < mHexagonGame.getSides(); ++i)
    {
        const float sAngle{div * 2.f * i};

        sf::Vector2f p1{_pos.x + std::cos(sAngle - div) * radius,
                        _pos.y + std::sin(sAngle - div) * (radius/skew.y)};
        sf::Vector2f p2{_pos.x + std::cos(sAngle + div) * radius,
                        _pos.y + std::sin(sAngle + div) * (radius/skew.y)};
        sf::Vector2f p3{_pos.x + std::cos(sAngle + div) * (radius + thickness),
                        _pos.y + std::sin(sAngle + div) * ((radius + thickness)/skew.y)};
        sf::Vector2f p4{_pos.x + std::cos(sAngle - div) * (radius + thickness),
                        _pos.y + std::sin(sAngle - div) * ((radius + thickness)/skew.y)};

        mHexagonGame.wallQuads.reserve_more(4);
        mHexagonGame.wallQuads.batch_unsafe_emplace_back(
            colorMain, p1, p2, p3, p4);
    }
}

void CPlayer::swap(HexagonGame& mHexagonGame, bool mSoundTog){
    angle += ssvu::pi;
    mHexagonGame.runLuaFunctionIfExists<void>("onCursorSwap");
    if (mSoundTog) {mHexagonGame.getAssets().playSound("swap.ogg");}
}

void CPlayer::update(HexagonGame& mHexagonGame, FT mFT)
{
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

    sf::Vector2f lastPos{pos};
    float currentSpeed{speed};

    const float lastAngle{angle};
    const float radius{mHexagonGame.getRadius()};
    const int movement{mHexagonGame.getInputMovement()};

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

    const sf::Vector2f tempPos{ssvs::getOrbitRad(startPos, angle, radius)};
    const sf::Vector2f pLeftCheck{ssvs::getOrbitRad(tempPos, angle - ssvu::piHalf, 0.01f)};
    const sf::Vector2f pRightCheck{ssvs::getOrbitRad(tempPos, angle + ssvu::piHalf, 0.01f)};

    //Collisions
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

            ssvs::moveTowards(lastPos, ssvs::zeroVec2f, 5 * mHexagonGame.getSpeedMultDM());

            pos = lastPos;
            mHexagonGame.death();

            return;
        }
    }

    const auto status{mHexagonGame.getStatus()};
    const auto levelStatus{mHexagonGame.getLevelStatus()};
    const auto styleData{mHexagonGame.getStyleData()};

    const sf::Vector2f skewEffect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D,
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D
    };
    const sf::Vector2f skew{1.f, 1.f+skewEffect.y};
    auto fieldAngle = ssvu::toRad(levelStatus.rotation);

    //For collisions check
    pos = ssvs::getOrbitRad(mHexagonGame.getFieldPos(), angle, radius);

    //For Drawing
    _pos = {mHexagonGame.getFieldPos().x + std::cos(fieldAngle + angle) * radius,
            mHexagonGame.getFieldPos().y + std::sin(fieldAngle + angle) * (radius/skew.y)};

}

} // namespace hg
