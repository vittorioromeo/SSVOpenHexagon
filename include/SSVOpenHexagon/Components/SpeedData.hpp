// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVUtils/Core/Common/Frametime.hpp>

namespace hg
{

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

    void update(ssvu::FT mFT) noexcept
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

} // namespace hg
