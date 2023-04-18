// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/Ticker.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVStart/Utils/Vector2.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace hg {

class CWall;
class CCustomWall;

class CPlayer
{
private:
    sf::Vector2f _startPos;   // Position at start of the level.

    sf::Vector2f _pos;        // Actual position of player.

    sf::Vector2f _prePushPos; // Position before the player is pushed by a wall.
                              // Unlike `pos` it is not updated after a
                              // successful wall push.

    sf::Vector2f _lastPos;    // Position of the player in the previous frame,
                           // adjusted according to the current frame's radius.

    float _hue;
    float _angle;
    float _lastAngle;
    float _size;
    float _speed;
    float _focusSpeed;

    bool _dead;
    bool _justSwapped;

    bool _forcedMove;       // Wherever player has been forcefully moved
                            // with a setPlayerAngle() call. Essential
                            // for proper behavior of collision calculation,
                            // especially on levels that make heavy usage of it.

    float _radius;          // Cached value of the radius in the current frame.

    float _maxSafeDistance; // The maximum distance that there can be between
                            // the current player position and the closest
                            // position safe from collision with a wall player
                            // overlaps with. If the closest position is further
                            // away player cannot be saved.

    float _currentSpeed;    // Cached player speed in the current frame.

    float _triangleWidth; // Visual width of the triangle, varies when focusing.
    float _triangleWidthTransitionTime; // From 0 to 1, when transitioning

    Ticker _swapTimer;
    Ticker _swapBlinkTimer;
    Ticker _deadEffectTimer;

    float _currTiltedAngle;

    void drawPivot(const unsigned int sides, const sf::Color& colorMain,
        Utils::FastVertexVectorTris& wallQuads,
        Utils::FastVertexVectorTris& capTris, const sf::Color& capColor);

    void drawDeathEffect(Utils::FastVertexVectorTris& wallQuads);

    template <typename Wall>
    [[nodiscard]] bool checkWallCollisionEscape(
        const Wall& wall, sf::Vector2f& pos, const float radiusSquared);

    void updateTriangleWidthTransition(const bool focused, const ssvu::FT ft);

public:
    explicit CPlayer(const sf::Vector2f& pos, const float swapCooldown,
        const float size, const float speed, const float focusSpeed) noexcept;

    [[nodiscard, gnu::always_inline]] const sf::Vector2f&
    getPosition() const noexcept
    {
        return _pos;
    }

    [[nodiscard, gnu::always_inline]] float getPlayerAngle() const noexcept
    {
        return _angle;
    }

    void setPlayerAngle(const float newAng) noexcept
    {
        _angle = newAng;
        _forcedMove = true;
    }

    void playerSwap();

    void kill(const bool fatal);

    void update(const bool focused, const bool swapEnabled, const ssvu::FT ft);

    void updateInputMovement(const float movementDir,
        const float playerSpeedMult, const bool focused, const ssvu::FT ft);

    void resetSwap(const float swapCooldown);

    void setJustSwapped(const bool value);

    void updatePosition(const float radius);

    [[nodiscard]] sf::Color getColor(const sf::Color& colorPlayer) const;

    [[nodiscard]] sf::Color getColorAdjustedForSwap(
        const sf::Color& colorPlayer) const;

    void draw(const unsigned int sides, const sf::Color& colorMain,
        const sf::Color& colorPlayer, Utils::FastVertexVectorTris& wallQuads,
        Utils::FastVertexVectorTris& capTris,
        Utils::FastVertexVectorTris& playerTris, const sf::Color& capColor,
        const float angleTiltIntensity, const bool swapBlinkingEffect);

    [[nodiscard]] bool push(const int movementDir, const float radius,
        const CWall& wall, const sf::Vector2f& mCenterPos,
        const float radiusSquared, ssvu::FT ft);

    [[nodiscard]] bool push(const int movementDir, const float radius,
        const hg::CCustomWall& wall, const float radiusSquared, ssvu::FT ft);

    [[nodiscard]] bool getJustSwapped() const noexcept;

    [[nodiscard, gnu::always_inline]] bool isReadyToSwap() const noexcept
    {
        return !_swapTimer.isRunning();
    }

    [[nodiscard, gnu::always_inline]] bool hasChangedAngle() const noexcept
    {
        return _angle != _lastAngle;
    }
};

} // namespace hg
