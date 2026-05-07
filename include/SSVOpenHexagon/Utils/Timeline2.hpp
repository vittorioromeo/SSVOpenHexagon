// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/FixedFunction.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg::Utils
{

class timeline2
{
public:
    using clock      = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using duration   = clock::duration;

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

    using action = sf::base::Variant<action_do, action_wait_for, action_wait_until, action_wait_until_fn>;

private:
    sf::base::Vector<action> _actions;

public:
    void clear();

    template <typename F>
    void append_do(F&& func)
    {
        _actions.emplaceBack(sf::base::inPlaceType<action_do>, SFML_BASE_FORWARD(func));
    }

    void append_wait_for(const duration d);
    void append_wait_for_seconds(const double s);
    void append_wait_for_sixths(const double s);
    void append_wait_until(const time_point tp);

    template <typename F>
    void append_wait_until_fn(F&& tp_fn)
    {
        _actions.emplaceBack(sf::base::inPlaceType<action_wait_until_fn>, SFML_BASE_FORWARD(tp_fn));
    }

    [[nodiscard]] sf::base::SizeT size() const noexcept;
    [[nodiscard]] action&         action_at(const sf::base::SizeT i) noexcept;
};

class timeline2_runner
{
public:
    using time_point = timeline2::time_point;
    using duration   = timeline2::duration;

    enum class outcome
    {
        proceed,
        waiting,
        finished
    };

private:
    sf::base::SizeT                _current_idx{0};
    sf::base::Optional<time_point> _wait_start_tp;

public:
    outcome update(timeline2& timeline, const time_point tp);
};

} // namespace hg::Utils
