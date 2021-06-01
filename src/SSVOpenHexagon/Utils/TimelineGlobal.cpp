// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/TimelineGlobal.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <type_traits>
#include <variant>
#include <utility>
#include <chrono>
#include <optional>
#include <functional>

namespace hg::Utils {

[[nodiscard]] int timelineGlobal::hasTimepoint(time_point tp) const noexcept
{
    return _actions.contains(tp);
}

[[nodiscard]] timelineGlobal::timeline_G::iterator
timelineGlobal::begin() noexcept
{
    return _actions.begin();
}

[[nodiscard]] timelineGlobal::timeline_G::iterator
timelineGlobal::end() noexcept
{
    return _actions.end();
}

void timelineGlobal::clear()
{
    _actions.clear();
}

void timelineGlobal::clearTimepoint(time_point tp)
{
    if(!hasTimepoint(tp))
    {
        return;
    }
    _actions[tp].clear();
}

void timelineGlobal::append_to(time_point tp, const std::function<void()>& func)
{
    if(!hasTimepoint(tp))
    {
        _actions[tp] = std::vector<action>{};
    }
    _actions[tp].emplace_back(action{func});
}

[[nodiscard]] std::size_t timelineGlobal::size() const noexcept
{
    return _actions.size();
}

[[nodiscard]] std::vector<timelineGlobal::action>& timelineGlobal::actions_at(
    const time_point tp)
{
    return _actions[tp];
}

[[nodiscard]] std::vector<timelineGlobal::time_point>
timelineGlobal_runner::getRunnableTimepoints(
    timelineGlobal& timeline, const time_point tp)
{
    std::vector<time_point> result{};
    long prev_tp_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        prev_tp.value().time_since_epoch())
                            .count();
    long tp_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch())
                       .count();
    for(auto iter = timeline.begin(); iter != timeline.end(); iter++)
    {
        long iter_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            iter->first.time_since_epoch())
                             .count();
        if(iter_time > prev_tp_time && iter_time <= tp_time)
        {
            result.emplace_back(iter->first);
        }
    }
    return result;
}

void timelineGlobal_runner::update(timelineGlobal& timeline, time_point tp)
{
    if(!prev_tp.has_value())
    {
        // Just simply grab the time point
        if(!timeline.hasTimepoint(tp))
        {
            return;
        }
        std::vector<action> actions = timeline.actions_at(tp);
        for(std::size_t i = 0; i < actions.size(); i++)
        {
            actions[i]._func();
        }
    }
    else
    {
        // Grab all timepoints in between the previous and current timepoint and
        // run all of the functions contained.
        std::vector<time_point> timepoints =
            getRunnableTimepoints(timeline, tp);
        for(std::size_t t = 0; t < timepoints.size(); t++)
        {
            std::vector<timelineGlobal::action> actions =
                timeline.actions_at(timepoints[t]);
            for(std::size_t i = 0; i < actions.size(); i++)
            {
                actions[i]._func();
            }
        }
    }
    // Update the previous timepoint with the new timepoint.
    prev_tp = tp;
}

void timelineGlobal_runner::clearLastTp()
{
    prev_tp.reset();
}

} // namespace hg::Utils