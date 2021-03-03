// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/Ticker.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVStart/Utils/Vector2.hpp>

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
    sf::Vector2f startPos; // Position at start of the level.

    sf::Vector2f pos; // Actual position of player.

    sf::Vector2f prePushPos; // Position before the player is pushed by a wall.
                             // Unlike `pos` it is not updated after a
                             // successful wall push.

    sf::Vector2f lastPos; // Position of the player in the previous frame,
                          // adjusted according to the current frame's radius.

    float hue;
    float angle;
    float lastAngle;
    float size;
    float speed;
    float focusSpeed;

    bool dead;
    bool justSwapped;

    bool forcedMove; // Wherever player has been forcefully moved
                     // with a setPlayerAngle() call. Essential
                     // for proper behavior of collision calculation,
                     // especially on levels that make heavy usage of it.

    float radius; // Cached value of the radius in the current frame.

    float maxSafeDistance; // The maximum distance that there can be between
                           // the current player position and the closest
                           // position safe from collision with a wall player
                           // overlaps with. If the closest position is further
                           // away player cannot be saved.

    float currentSpeed; // Cached player speed in the current frame.

    float radius;
    float maxSafeDistance;
    float currentSpeed;

    Ticker swapTimer;
    Ticker swapBlinkTimer;
    Ticker deadEffectTimer;

    void drawPivot(HexagonGame& mHexagonGame, const sf::Color& mCapColor);
    void drawDeathEffect(HexagonGame& mHexagonGame);

    template <typename Wall>
    [[nodiscard]] bool checkWallCollisionEscape(
        const Wall& wall, sf::Vector2f& mPos, const float mRadiusSquared);

public:
    CPlayer(const sf::Vector2f& mPos, const float swapCooldown) noexcept;

    [[gnu::always_inline, nodiscard]] const sf::Vector2f&
    getPosition() const noexcept
    {
        return pos;
    }

    [[gnu::always_inline, nodiscard]] float getPlayerAngle() const noexcept
    {
        return angle;
    }

    void setPlayerAngle(const float newAng) noexcept
    {
        angle = newAng;
        forcedMove = true;
    }

    void playerSwap(HexagonGame& mHexagonGame, bool mPlaySound);

    void kill(HexagonGame& mHexagonGame);

    void update(HexagonGame& mHexagonGame, const ssvu::FT mFT);
    void updateInput(HexagonGame& mHexagonGame, const ssvu::FT mFT);
    void updatePosition(const HexagonGame& mHexagonGame, const ssvu::FT mFT);
    void updateCollisionValues(
        const HexagonGame& mHexagonGame, const ssvu::FT mFT);

    void draw(HexagonGame& mHexagonGame, const sf::Color& mCapColor);

    [[nodiscard]] bool push(const HexagonGame& mHexagonGame, const CWall& wall,
        const sf::Vector2f& mCenterPos, const float mRadiusSquared,
        ssvu::FT mFT);

    [[nodiscard]] bool push(const HexagonGame& mHexagonGame,
        const hg::CCustomWall& wall, const float mRadiusSquared, ssvu::FT mFT);

    [[nodiscard]] bool getJustSwapped() const noexcept;
};

} // namespace hg
