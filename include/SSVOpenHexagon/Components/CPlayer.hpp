// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Config.hpp"

#include "SSVUtils/Ticker/Ticker.hpp"
#include "SSVUtils/Core/Common/Frametime.hpp"

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

    float hue{0};
    float angle{0};
    float size{Config::getPlayerSize()};
    float speed{Config::getPlayerSpeed()};
    float focusSpeed{Config::getPlayerFocusSpeed()};

    bool dead{false};

    ssvu::Ticker swapTimer{36.f};
    ssvu::Ticker swapBlinkTimer{5.f};
    ssvu::Ticker deadEffectTimer{80.f, false};

    void drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor,
                    const LevelStatus& levelStatus, const StyleData& styleData);
    void drawDeathEffect(HexagonGame& mHexagonGame, const StyleData& styleData);

public:
    CPlayer() noexcept;

    float getPlayerAngle();
    void setPlayerAngle(float newAng);
    void swap(HexagonGame& mHexagonGame, bool mSoundTog);

    void update(HexagonGame& mHexagonGame, FT mFT);
    void draw(HexagonGame& mHexagonGame, const sf::Color& mCapColor,
              const LevelStatus& levelStatus, const StyleData& styleData);
};

} // namespace hg
