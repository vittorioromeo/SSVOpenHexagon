// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/FixedFunction.hpp"
#include "SSVOpenHexagon/Utils/TinyVariant.hpp"

#include <chrono>
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
        Utils::FixedFunction<void(), 64> _func;
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
        Utils::FixedFunction<time_point(), 32> _time_point_fn;
    };

    using action = vittorioromeo::tinyvariant<action_do, action_wait_for,
        action_wait_until, action_wait_until_fn>;

private:
    std::vector<action> _actions;

public:
    void clear();

    template <typename F>
    void append_do(F&& func)
    {
        _actions.emplace_back(
            vittorioromeo::impl::tinyvariant_inplace_type_t<action_do>{},
            SSVOH_FWD(func));
    }

    void append_wait_for(const duration d);
    void append_wait_for_seconds(const double s);
    void append_wait_for_sixths(const double s);
    void append_wait_until(const time_point tp);

    template <typename F>
    void append_wait_until_fn(F&& tp_fn)
    {
        _actions.emplace_back(vittorioromeo::impl::tinyvariant_inplace_type_t<
                                  action_wait_until_fn>{},
            SSVOH_FWD(tp_fn));
    }

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
