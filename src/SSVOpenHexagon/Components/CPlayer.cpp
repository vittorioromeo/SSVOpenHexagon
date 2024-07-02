// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CPlayer.hpp"

#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"

#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Utils/Easing.hpp"
#include "SSVOpenHexagon/Utils/Geometry.hpp"
#include "SSVOpenHexagon/Utils/Math.hpp"
#include "SSVOpenHexagon/Utils/MoveTowards.hpp"
#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"
#include "SSVOpenHexagon/Utils/Ticker.hpp"

#include <SFML/System/Angle.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <cmath>

namespace hg {

inline constexpr float baseThickness{5.f};
inline constexpr float unfocusedTriangleWidth{3.f};
inline constexpr float focusedTriangleWidth{-1.5f};
inline constexpr float triangleWidthRange{
    unfocusedTriangleWidth - focusedTriangleWidth};

CPlayer::CPlayer(const sf::Vector2f& pos, const float swapCooldown,
    const float size, const float speed, const float focusSpeed) noexcept
    : _startPos{pos},
      _pos{pos},
      _prePushPos{pos},
      _lastPos{pos},
      _hue{0},
      _angle{0},
      _lastAngle{0},
      _size{size},
      _speed{speed},
      _focusSpeed{focusSpeed},
      _dead{false},
      _justSwapped{false},
      _forcedMove{false},
      _radius{0.f},
      _maxSafeDistance{0.f},
      _currentSpeed{0.f},
      _triangleWidth{unfocusedTriangleWidth},
      _triangleWidthTransitionTime{0.f},
      _swapTimer{swapCooldown},
      _swapBlinkTimer{6.f},
      _deadEffectTimer{80.f, false},
      _currTiltedAngle{0}
{}

[[nodiscard]] sf::Color CPlayer::getColor(const sf::Color& colorPlayer) const
{
    return !_deadEffectTimer.isRunning() ? colorPlayer
                                         : Utils::getColorFromHue(_hue / 360.f);
}

[[nodiscard]] sf::Color CPlayer::getColorAdjustedForSwap(
    const sf::Color& colorPlayer) const
{
    if (!_swapTimer.isRunning() && !_dead)
    {
        return Utils::getColorFromHue(
            std::fmod(_swapBlinkTimer.getCurrent() / 12.f, 0.2f));
    }

    return getColor(colorPlayer);
}

void CPlayer::draw(const unsigned int sides, const sf::Color& colorMain,
    const sf::Color& colorPlayer, Utils::FastVertexVectorTris& wallQuads,
    Utils::FastVertexVectorTris& capTris,
    Utils::FastVertexVectorTris& playerTris, const sf::Color& capColor,
    const float angleTiltIntensity, const bool swapBlinkingEffect)
{
    drawPivot(sides, colorMain, wallQuads, capTris, capColor);

    if (_deadEffectTimer.isRunning())
    {
        drawDeathEffect(wallQuads);
    }

    const float tiltedAngle =
        _angle + (_currTiltedAngle * Utils::toRad(24.f) * angleTiltIntensity);

    const sf::Vector2f pLeft = _pos.movedTowards(
        _size + _triangleWidth, sf::radians(tiltedAngle - Utils::toRad(100.f)));

    const sf::Vector2f pRight = _pos.movedTowards(
        _size + _triangleWidth, sf::radians(tiltedAngle + Utils::toRad(100.f)));

    playerTris.reserve_more(3);
    playerTris.batch_unsafe_emplace_back(
        swapBlinkingEffect ? getColorAdjustedForSwap(colorPlayer)
                           : getColor(colorPlayer),
        _pos.movedTowards(_size, sf::radians(tiltedAngle)), pLeft, pRight);
}

void CPlayer::drawPivot(const unsigned int sides, const sf::Color& colorMain,
    Utils::FastVertexVectorTris& wallQuads,
    Utils::FastVertexVectorTris& capTris, const sf::Color& capColor)
{
    const float div{Utils::tau / sides * 0.5f};
    const float pRadius{_radius * 0.75f};

    wallQuads.reserve_more_quad(sides * 1);
    capTris.reserve_more(sides * 3);

    for (auto i(0u); i < sides; ++i)
    {
        const float sAngle{div * 2.f * i};

        const sf::Vector2f p1{
            _startPos.movedTowards(pRadius, sf::radians(sAngle - div))};
        const sf::Vector2f p2{
            _startPos.movedTowards(pRadius, sf::radians(sAngle + div))};
        const sf::Vector2f p3{_startPos.movedTowards(
            pRadius + baseThickness, sf::radians(sAngle + div))};
        const sf::Vector2f p4{_startPos.movedTowards(
            pRadius + baseThickness, sf::radians(sAngle - div))};

        wallQuads.batch_unsafe_emplace_back_quad(colorMain, p1, p2, p3, p4);
        capTris.batch_unsafe_emplace_back(capColor, p1, p2, _startPos);
    }
}

void CPlayer::drawDeathEffect(Utils::FastVertexVectorTris& wallQuads)
{
    const float div{Utils::tau / 6 * 0.5f};
    const float dRadius{_hue / 8.f};
    const float thickness{_hue / 20.f};

    const sf::Color colorMain{Utils::getColorFromHue((360.f - _hue) / 360.f)};

    wallQuads.reserve_more_quad(6);

    for (auto i(0u); i < 6; ++i)
    {
        const float sAngle{div * 2.f * i};

        const sf::Vector2f p1{
            _pos.movedTowards(dRadius, sf::radians(sAngle - div))};
        const sf::Vector2f p2{
            _pos.movedTowards(dRadius, sf::radians(sAngle + div))};
        const sf::Vector2f p3{
            _pos.movedTowards(dRadius + thickness, sf::radians(sAngle + div))};
        const sf::Vector2f p4{
            _pos.movedTowards(dRadius + thickness, sf::radians(sAngle - div))};

        wallQuads.batch_unsafe_emplace_back_quad(colorMain, p1, p2, p3, p4);
    }
}

void CPlayer::playerSwap()
{
    _angle += Utils::pi;
}

void CPlayer::kill(const bool fatal)
{
    _deadEffectTimer.restart();

    if (fatal)
    {
        _dead = true;

        // Avoid moving back position if the player had just swapped or the
        // player was forcibly moved by a lot via Lua scripting.
        if (!_justSwapped && (_lastPos - _pos).length() < 24.f)
        {
            // Move back position to graphically show the tip of the triangle
            // hitting the wall rather than the center of the triangle.
            _pos = _lastPos.movedTowards(-_size, sf::radians(_angle));
        }
    }
}

inline constexpr float collisionPadding{0.5f};

template <typename Wall>
[[nodiscard]] bool CPlayer::checkWallCollisionEscape(
    const Wall& wall, sf::Vector2f& pos, const float radiusSquared)
{
    // To find the closest wall side we intersect the circumference of the
    // possible player positions with the sides of the wall. We use the
    // intersection closest to the player's position as post collision target.
    // If an escape route could not be found player is killed.

    bool saved{false};
    sf::Vector2f vec1, vec2;
    float tempDistance, safeDistance{_maxSafeDistance};
    const unsigned int vxIncrement{wall.isCustomWall() ? 1u : 2u};
    const std::array<sf::Vector2f, 4>& wVertexes{wall.getVertexPositions()};

    // This is actually useless for normal walls, but if we removed
    // getKillingSide() for CWall we would have to write a separate
    // function just to do without this variable,
    const unsigned int killingSide{wall.getKillingSide()};

    const auto assignResult = [&]()
    {
        tempDistance = (vec1 - pos).lengthSq();
        if (tempDistance < safeDistance)
        {
            pos = vec1;
            saved = true;
            safeDistance = tempDistance;
        }
    };

    for (unsigned int i{0u}, j{3u}; i < 4u; i += vxIncrement, j = i - 1)
    {
        if (j == killingSide)
        {
            continue;
        }

        switch (Utils::getLineCircleIntersection(
            vec1, vec2, wVertexes[i], wVertexes[j], radiusSquared))
        {
            case 1u:
            {
                assignResult();
                break;
            }

            case 2u:
            {
                if ((vec1 - pos).lengthSq() > (vec2 - pos).lengthSq())
                {
                    vec1 = vec2;
                }

                assignResult();
                break;
            }

            default:
            {
                break;
            }
        }
    }

    return saved;
}

[[nodiscard]] bool CPlayer::push(const int movementDir, const float radius,
    const CWall& wall, const sf::Vector2f& centerPos, const float radiusSquared,
    const float ft)
{
    if (_dead)
    {
        return false;
    }

    sf::Vector2f testPos{_pos};
    sf::Vector2f pushVel{0.f, 0.f};

    // If it's a rotating wall push player in the direction the
    // wall is rotating by the appropriate amount, but only if the direction
    // of the rotation is different from the direction player is moving.
    // Save the position difference in case we need to do a second attempt
    // at saving player.
    const SpeedData& curveData{wall.getCurve()};
    if (curveData._speed != 0.f &&
        Utils::getSign(curveData._speed) != movementDir)
    {
        wall.moveVertexAlongCurve(testPos, centerPos, ft);
        pushVel = testPos - _pos;
    }

    // If player is not moving calculate now...
    if (!movementDir && !_forcedMove)
    {
        const sf::Vector2f posDiff = testPos - _prePushPos;
        const sf::Vector2f posDiffNormalized =
            posDiff == sf::Vector2f::Zero ? posDiff : posDiff.normalized();

        _pos = testPos + posDiffNormalized * (2.f * collisionPadding);
        _angle = _pos.angle().asRadians();
        updatePosition(radius);
        return wall.isOverlapping(_pos);
    }

    // ...otherwise make testPos the position of the previous frame plus
    // the curving wall's velocity, and check an escape on that position.
    // Using the previous frame's position is essential for levels with
    // a really high amount of sides. Player might be currently positioned
    // closer to the side opposite to the one it should intuitively slide
    // againts
    //
    //   BEFORE               AFTER
    //
    //  |       |           |       |
    //  |       |           |       |
    //  |       |   *       |  *    | <- this should be our target wall
    //  |       |  * *      | * *   |    but if we use the current position
    //  |       | *****     |*****  |    it is the other one that is closer
    //  |       |           |       |
    testPos = _lastPos + pushVel;
    if (wall.isOverlapping(testPos) ||
        !checkWallCollisionEscape(wall, testPos, radiusSquared))
    {
        return true;
    }

    // If player survived assign it the saving testPos, but displace it further
    // out the wall border, otherwise player would be lying right on top of the
    // border.
    _pos = testPos + (testPos - _prePushPos).normalized() * collisionPadding;
    _angle = _pos.angle().asRadians();
    updatePosition(radius);
    return false;
}

[[nodiscard]] bool CPlayer::push(const int movementDir, const float radius,
    const CCustomWall& wall, const float radiusSquared, const float ft)
{
    (void)ft; // Currently unused.

    if (_dead)
    {
        return false;
    }

    // Look for a side that is moving in a direction perpendicular to the
    // player, such side is a candidate for pushing it like a curving wall would
    // do. (_lastPos is the best candidate for this check).

    const std::array<sf::Vector2f, 4>& wVertexes{wall.getVertexPositions()};
    const std::array<sf::Vector2f, 4>& wOldVertexes{
        wall.getOldVertexPositions()};
    sf::Vector2f pushVel{0.f, 0.f}, i1, i2;
    const unsigned int killingSide{wall.getKillingSide()};
    constexpr float pushDotThreshold{
        0.15f}; // 0.1 would be enough in most scenarios
                // but we raise it to 0.15 for really fast walls.

    for (unsigned int i{0u}, j{3u}; i < 4; j = i++)
    {
        if (j == killingSide)
        {
            continue;
        }

        const std::array<sf::Vector2f, 4> collisionPolygon{
            wVertexes[i], wOldVertexes[i], wOldVertexes[j], wVertexes[j]};

        if (Utils::pointInPolygon<4>(collisionPolygon, _lastPos.x, _lastPos.y))
        {
            // For a side to be an effective source of push it must have
            // intersected the player's positions circle both now and the
            // previous frame.
            if (Utils::getLineCircleClosestIntersection(i1, _lastPos,
                    wOldVertexes[i], wOldVertexes[j], radiusSquared) &&
                Utils::getLineCircleClosestIntersection(
                    i2, _lastPos, wVertexes[i], wVertexes[j], radiusSquared))
            {
                pushVel = i2 - i1;
                if (std::abs(pushVel.normalized().dot(_lastPos.normalized())) >
                    pushDotThreshold)
                {
                    pushVel = {0.f, 0.f};
                }
            }
            break;
        }
    }

    // Player is not moving exit now.
    if (!movementDir && !_forcedMove)
    {
        _pos += pushVel;
        _pos += (_pos - _prePushPos).normalized() * (2.f * collisionPadding);
        _angle = _pos.angle().asRadians();
        updatePosition(radius);
        return wall.isOverlapping(_pos);
    }

    // If alive try to find a close enough safe position.
    sf::Vector2f testPos{_lastPos + pushVel};
    if (wall.isOverlapping(testPos) ||
        !checkWallCollisionEscape(wall, testPos, radiusSquared))
    {
        return true;
    }

    // If player survived assign it the saving testPos, but displace it further
    // out the wall border, otherwise player would be lying right on top of the
    // border.
    _pos = testPos + (testPos - _prePushPos).normalized() * collisionPadding;
    _angle = _pos.angle().asRadians();
    updatePosition(radius);

    return false;
}

void CPlayer::updateTriangleWidthTransition(const bool focused, const float ft)
{
    if (focused && _triangleWidthTransitionTime < 1.f)
    {
        Utils::moveTowards(_triangleWidthTransitionTime, 1.f, ft * 0.1f);
    }
    else if (!focused && _triangleWidthTransitionTime > 0.f)
    {
        Utils::moveTowardsZero(_triangleWidthTransitionTime, ft * 0.1f);
    }

    _triangleWidth =
        triangleWidthRange *
        (1.f - Utils::getSmoothStep(0.f, 1.f, _triangleWidthTransitionTime));
}

void CPlayer::update(const bool focused, const bool swapEnabled, const float ft)
{
    updateTriangleWidthTransition(focused, ft);

    if (_deadEffectTimer.isRunning())
    {
        _deadEffectTimer.update(ft);

        _hue += 18.f * ft;

        if (_hue > 360.f)
        {
            _hue = 0.f;
        }

        if (_dead)
        {
            return;
        }

        if (_deadEffectTimer.getTotal() >= 100)
        {
            _deadEffectTimer.stop();
            _deadEffectTimer.resetAll();
        }
    }

    _swapBlinkTimer.update(ft / 3.f);

    if (swapEnabled && _swapTimer.update(ft))
    {
        _swapTimer.stop();
    }

    _lastAngle = _angle;
    _forcedMove = false;
}

void CPlayer::updateInputMovement(const float movementDir,
    const float playerSpeedMult, const bool focused, const float ft)
{
    _currentSpeed = playerSpeedMult * (focused ? _focusSpeed : _speed) * ft;
    _angle += Utils::toRad(_currentSpeed * movementDir);

    const float inc = ft / 10.f;

    _currTiltedAngle =
        (movementDir == 0.f)
            ? Utils::getMoveTowardsZero(_currTiltedAngle, inc)
            : Utils::getMoveTowards(_currTiltedAngle, movementDir, inc * 2.f);
}

void CPlayer::resetSwap(const float swapCooldown)
{
    _swapTimer.restart(swapCooldown);
    _swapBlinkTimer.restart(6.f);
}

void CPlayer::setJustSwapped(const bool value)
{
    _justSwapped = value;
}

void CPlayer::updatePosition(const float radius)
{
    _radius = radius;

    _prePushPos = _pos = _startPos.movedTowards(_radius, sf::radians(_angle));
    _lastPos = _startPos.movedTowards(_radius, sf::radians(_lastAngle));

    _maxSafeDistance =
        (_lastPos - _startPos.movedTowards(_radius,
                        sf::radians(_lastAngle + Utils::toRad(_currentSpeed))))
            .lengthSq() +
        32.f;
}

[[nodiscard]] bool CPlayer::getJustSwapped() const noexcept
{
    return _justSwapped;
}

} // namespace hg
