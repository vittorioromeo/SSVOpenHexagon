// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVUtils/Core/Common/Frametime.hpp>

namespace hg {

struct SpeedData
{
    float _speed;
    float _accel;
    float _min;
    float _max;
    float _pingPong;

    explicit SpeedData(float speed = 0, float accel = 0.f, float min = 0.f,
        float max = 0.f, bool pingPong = false) noexcept
        : _speed{speed},
          _accel{accel},
          _min{min},
          _max{max},
          _pingPong{pingPong ? -1.f : 1.f}
    {}

    void update(const ssvu::FT ft) noexcept
    {
        if(_accel == 0.f)
        {
            return;
        }

        _speed += _accel * ft;

        if(_speed > _max)
        {
            _speed = _max;
            _accel *= _pingPong;
        }
        else if(_speed < _min)
        {
            _speed = _min;
            _accel *= _pingPong;
        }
    }
};

} // namespace hg
