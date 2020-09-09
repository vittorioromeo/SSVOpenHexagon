// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/Ticker.hpp"

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace hg
{

class HexagonGame;
class CWall;
class CCustomWall;

class CPlayer
{
private:
    sf::Vector2f startPos;
    sf::Vector2f pos;
    sf::Vector2f lastPos;

    float hue;
    float angle;
    float lastAngle;
    float size;
    float speed;
    float focusSpeed;

    bool dead;
    bool justSwapped;

    Ticker swapTimer;
    Ticker swapBlinkTimer;
    Ticker deadEffectTimer;

    void drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor);
    void drawDeathEffect(HexagonGame& mHexagonGame);

public:
    CPlayer(const sf::Vector2f& mPos, const float swapCooldown) noexcept;

    [[gnu::always_inline, nodiscard]] const sf::Vector2f&
    getPosition() const noexcept
    {
        return pos;
    }

    [[nodiscard]] float getPlayerAngle() const noexcept;

    void setPlayerAngle(const float newAng) noexcept;
    void playerSwap(HexagonGame& mHexagonGame, bool mPlaySound);

    void kill(HexagonGame& mHexagonGame);

    void update(HexagonGame& mHexagonGame, ssvu::FT mFT);
    void updateInput(HexagonGame& mHexagonGame, ssvu::FT mFT);
    void updatePosition(HexagonGame& mHexagonGame, ssvu::FT mFT);

    void draw(HexagonGame& mHexagonGame, const sf::Color& mCapColor);

    [[nodiscard]] bool push(
        HexagonGame& mHexagonGame, const hg::CWall& wall, ssvu::FT mFT);

    [[nodiscard]] bool push(
        HexagonGame& mHexagonGame, const hg::CCustomWall& wall, ssvu::FT mFT);

    [[nodiscard]] bool getJustSwapped() const noexcept;
};

} // namespace hg
