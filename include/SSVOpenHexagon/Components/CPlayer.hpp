// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/Ticker.hpp"

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace hg
{

class HexagonGame;

class CPlayer
{
private:
    sf::Vector2f pTip;
    sf::Vector2f pLeft;
    sf::Vector2f pRight;

    sf::Vector2f pDTip;
    sf::Vector2f pDLeft;
    sf::Vector2f pDRight;

    sf::Vector2f startPos;
    sf::Vector2f pos;
    sf::Vector2f _pos;

    float hue;
    float angle;
    float size;
    float speed;
    float focusSpeed;

    bool dead;

    Ticker swapTimer;
    Ticker swapBlinkTimer;
    Ticker deadEffectTimer;

    void drawPivot(HexagonGame& mHexagonGame);
    void drawDeathEffect(HexagonGame& mHexagonGame);

public:
    CPlayer(const sf::Vector2f& mStartPos) noexcept;

    [[nodiscard]] float getPlayerAngle() const;
    void setPlayerAngle(const float newAng);
    void swap(HexagonGame& mHexagonGame, bool mSoundTog);

    void update(HexagonGame& mHexagonGame, ssvs::FT mFT);
    void draw(HexagonGame& mHexagonGame);
};

} // namespace hg
