// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/Timeline2.hpp"

namespace hg {

struct CustomTimeline
{
    Utils::timeline2 _timeline;
    Utils::timeline2_runner _runner;
};

} // namespace hg
