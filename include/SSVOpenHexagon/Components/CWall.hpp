// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{

class HexagonGame;

struct SpeedData
{
    float speed;
    float accel;
    float min;
    float max;

    bool pingPong;

    SpeedData(float mSpeed = 0, float mAccel = 0.f, float mMin = 0.f,
        float mMax = 0.f, bool mPingPong = false) noexcept
        : speed{mSpeed}, accel{mAccel}, min{mMin}, max{mMax}, pingPong{
                                                                  mPingPong}
    {
    }

    void update(FT mFT) noexcept
    {
        if(accel == 0)
        {
            return;
        }

        speed += accel * mFT;
        if(speed > max)
        {
            speed = max;
            if(pingPong)
            {
                accel *= -1;
            }
        }
        else if(speed < min)
        {
            speed = min;
            if(pingPong)
            {
                accel *= -1;
            }
        }
    }
};

class CWall
{
private:
    std::array<sf::Vector2f, 4> Collisions_vertexPositions;
    std::array<sf::Vector2f, 4> vertexPositions;

    unsigned int initialSides;
    unsigned int side;
    float distance;
    float thickness;

    SpeedData speed;
    SpeedData curve;

    float curveOffset{0.f};

    float hueMod{0};

public:
    bool killed{false};

    CWall(HexagonGame& mHexagonGame, unsigned int mSide,
          float mThickness, float mDistance,
          const SpeedData& mSpeed, const SpeedData& mCurve);

    void update(
        HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos, FT mFT);
    void draw(HexagonGame& mHexagonGame);

    void setHueMod(float mHueMod) noexcept
    {
        hueMod = mHueMod;
    }

    [[nodiscard]] SpeedData& getSpeed() noexcept
    {
        return speed;
    }

    [[nodiscard]] SpeedData& getCurve() noexcept
    {
        return curve;
    }

    [[nodiscard]] bool isOverlapping(const sf::Vector2f& mPoint) const noexcept
    {
        return ssvs::isPointInPolygon(Collisions_vertexPositions, mPoint);
    }
};

} // namespace hg
