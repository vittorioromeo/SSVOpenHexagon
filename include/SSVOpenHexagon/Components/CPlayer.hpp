// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

namespace hg
{

class HexagonGame;

class CPlayer final
{
private:
    HexagonGame* hexagonGame;

    Vec2f pLeft;
    Vec2f pRight;
    Vec2f startPos;
    Vec2f pos;

    float hue{0};
    float angle{0};
    float size{Config::getPlayerSize()};
    float speed{Config::getPlayerSpeed()};
    float focusSpeed{Config::getPlayerFocusSpeed()};

    bool dead{false};

    Ticker swapTimer{36.f};
    Ticker swapBlinkTimer{5.f};
    Ticker deadEffectTimer{80.f, false};

    void drawPivot();
    void drawDeathEffect();

public:
    CPlayer(HexagonGame& mHexagonGame, const Vec2f& mStartPos);

    void update(FT mFT);
    void draw();
};

} // namespace hg
