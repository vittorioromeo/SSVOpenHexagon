// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{

class HexagonGame;

struct SpeedData
{
    float speed, accel, min, max;
    bool pingPong;

    SpeedData(float mSpeed = 0, float mAccel = 0.f, float mMin = 0.f,
        float mMax = 0.f, bool mPingPong = false) noexcept
        : speed{mSpeed}, accel{mAccel}, min{mMin}, max{mMax}, pingPong{
                                                                  mPingPong}
    {
    }

    void update(FT mFT) noexcept
    {
        if(accel == 0) return;
        speed += accel * mFT;
        if(speed > max)
        {
            speed = max;
            if(pingPong) accel *= -1;
        }
        else if(speed < min)
        {
            speed = min;
            if(pingPong) accel *= -1;
        }
    }
};

class CWall final
{
private:
    HexagonGame* hexagonGame;
    Vec2f centerPos;
    std::array<Vec2f, 4> vertexPositions;
    SpeedData speed, curve;
    float distance{0}, thickness{0}, hueMod{0};
    int side{0};

public:
    bool killed{false};

    CWall(HexagonGame& mHexagonGame, const Vec2f& mCenterPos, int mSide,
        float mThickness, float mDistance, const SpeedData& mSpeed,
        const SpeedData& mCurve);

    void update(FT mFT);
    void draw();

    void setHueMod(float mHueMod) noexcept
    {
        hueMod = mHueMod;
    }

    SpeedData& getSpeed() noexcept
    {
        return speed;
    }
    SpeedData& getCurve() noexcept
    {
        return curve;
    }
    bool isOverlapping(const Vec2f& mPoint) const noexcept
    {
        return ssvs::isPointInPolygon(vertexPositions, mPoint);
    }
};

} // namespace hg
