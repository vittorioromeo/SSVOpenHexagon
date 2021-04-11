// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <type_traits>
#include <variant>
#include <utility>
#include <chrono>
#include <functional>
#include <optional>
#include <cstddef>
#include <vector>

namespace hg::Utils {

class timeline2
{
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using duration = clock::duration;

    struct action_do
    {
        std::function<void()> _func;
    };

    struct action_wait_for
    {
        duration _duration;
    };

    struct action_wait_until
    {
        time_point _time_point;
    };

    struct action_wait_until_fn
    {
        std::function<time_point()> _time_point_fn;
    };

    struct action
    {
        std::variant<action_do, action_wait_for, action_wait_until,
            action_wait_until_fn>
            _inner;
    };

private:
    std::vector<action> _actions;

public:
    void clear();

    void append_do(const std::function<void()>& func);
    void append_wait_for(const duration d);
    void append_wait_for_seconds(const double s);
    void append_wait_for_sixths(const double s);
    void append_wait_until(const time_point tp);
    void append_wait_until_fn(const std::function<time_point()>& tp_fn);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] action& action_at(const std::size_t i) noexcept;
};

class timeline2_runner
{
public:
    using time_point = timeline2::time_point;
    using duration = timeline2::duration;

    enum class outcome
    {
        proceed,
        waiting,
        finished
    };

private:
    std::size_t _current_idx{0};
    std::optional<time_point> _wait_start_tp;

public:
    outcome update(timeline2& timeline, const time_point tp);
};

} // namespace hg::Utils
