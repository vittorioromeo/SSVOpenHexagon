// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/CustomTimelineManager.hpp"

#include "SSVOpenHexagon/Core/CustomTimeline.hpp"
#include "SSVOpenHexagon/Core/CustomTimelineHandle.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <chrono>
#include <cstddef>
#include <vector>

namespace hg {

CustomTimelineManager::CustomTimelineManager() = default;

CustomTimelineManager::~CustomTimelineManager() = default;

[[nodiscard]] bool CustomTimelineManager::isHandleValid(
    const CustomTimelineHandle h) const noexcept
{
    return h >= 0 && h < static_cast<CustomTimelineHandle>(_timelines.size());
}

void CustomTimelineManager::clear() noexcept
{
    _timelines.clear();
}

void CustomTimelineManager::updateAllTimelines(const HRTimePoint tp)
{
    for(CustomTimeline& t : _timelines)
    {
        if(const auto o = t._runner.update(t._timeline, tp);
            o == Utils::timeline2_runner::outcome::finished)
        {
            t._timeline.clear();
            t._runner = {};
        }
    }
}

[[nodiscard]] CustomTimelineHandle CustomTimelineManager::create()
{
    _timelines.emplace_back();
    const CustomTimelineHandle h = _timelines.size() - 1;

    SSVOH_ASSERT(isHandleValid(h));
    return h;
}

[[nodiscard]] CustomTimeline& CustomTimelineManager::get(
    const CustomTimelineHandle h) noexcept
{
    SSVOH_ASSERT(isHandleValid(h));
    return _timelines.at(static_cast<std::size_t>(h));
}

[[nodiscard]] const CustomTimeline& CustomTimelineManager::get(
    const CustomTimelineHandle h) const noexcept
{
    SSVOH_ASSERT(isHandleValid(h));
    return _timelines.at(static_cast<std::size_t>(h));
}

} // namespace hg
