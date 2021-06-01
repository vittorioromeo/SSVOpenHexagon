// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// This file, unlike Timeline2.hpp, is built for a global timeline instead of a
// relative timeline.

#pragma once

#include <chrono>
#include <functional>
#include <cstddef>
#include <optional>

#include <map>
#include <vector> // We won't use the vector for the timeline itself, but a container

namespace hg::Utils {

class timelineGlobal
{
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;


    // Variants don't need to be used here, because waiting functions aren't
    // needed in a global timeline
    struct action
    {
        std::function<void()> _func;
    };

    using timeline_G = std::map<time_point, std::vector<action>>;

private:
    timeline_G _actions;

public:
    [[nodiscard]] int hasTimepoint(time_point tp) const noexcept;
    [[nodiscard]] timeline_G::iterator begin() noexcept;
    [[nodiscard]] timeline_G::iterator end() noexcept;
    void clear();
    void clearTimepoint(time_point tp);

    void append_to(time_point tp, const std::function<void()>& func);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::vector<action>& actions_at(const time_point tp);
};

class timelineGlobal_runner
{
public:
    using time_point = timelineGlobal::time_point;
    using action = timelineGlobal::action;

private:
    std::optional<time_point> prev_tp;

public:
    [[nodiscard]] std::vector<time_point> getRunnableTimepoints(
        timelineGlobal& timeline, const time_point tp);
    void update(timelineGlobal& timeline, const time_point tp);
    void clearLastTp();
};

} // namespace hg::Utils