// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/CustomTimelineHandle.hpp"

#include "SSVOpenHexagon/Utils/Clock.hpp"

#include <chrono>
#include <vector>

namespace hg {

struct CustomTimeline;

class CustomTimelineManager
{
private:
    std::vector<CustomTimeline> _timelines;

public:
    CustomTimelineManager();
    ~CustomTimelineManager();

    [[nodiscard]] bool isHandleValid(
        const CustomTimelineHandle h) const noexcept;

    void clear() noexcept;

    void updateAllTimelines(const HRTimePoint tp);

    [[nodiscard]] CustomTimelineHandle create();

    [[nodiscard]] CustomTimeline& get(const CustomTimelineHandle h) noexcept;

    [[nodiscard]] const CustomTimeline& get(
        const CustomTimelineHandle h) const noexcept;
};

} // namespace hg
