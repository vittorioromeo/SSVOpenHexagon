// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Macros.hpp"
#include "SSVOpenHexagon/Utils/Timeline2.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StdChrono.hpp"

namespace hg::Utils
{

void timeline2::clear()
{
    _actions.clear();
}


void timeline2::append_wait_for(const duration d)
{
    _actions.emplaceBack(sf::base::inPlaceType<action_wait_for>, d);
}

void timeline2::append_wait_for_seconds(const double s)
{
    append_wait_for(std::chrono::milliseconds(static_cast<int>(s * 1000.0)));
}

void timeline2::append_wait_for_sixths(const double s)
{
    append_wait_for_seconds(s / 60.0);
}

void timeline2::append_wait_until(const time_point tp)
{
    _actions.emplaceBack(sf::base::inPlaceType<action_wait_until>, tp);
}

[[nodiscard]] sf::base::SizeT timeline2::size() const noexcept
{
    return _actions.size();
}

[[nodiscard]] timeline2::action& timeline2::action_at(const sf::base::SizeT i) noexcept
{
    SSVOH_ASSERT(i < size());
    return _actions[i];
}

timeline2_runner::outcome timeline2_runner::update(timeline2& timeline, const time_point tp)
{
    if (_current_idx >= timeline.size())
    {
        // Empty timeline or reached the end.
        return outcome::finished;
    }

    while (_current_idx < timeline.size())
    {
        timeline2::action& a = timeline.action_at(_current_idx);

        // `action_do` is handled out-of-band because its callback can run
        // user code (Lua) that appends to the same timeline. The append
        // may reallocate `_actions`, freeing the variant we'd otherwise
        // still be visiting through `linearMatch` -- `linearMatch`'s fold
        // re-reads the variant's discriminator after the visitor returns,
        // which would be a heap-use-after-free on the reallocated buffer.
        // Move the function out, advance the index, drop the reference,
        // *then* invoke.
        if (a.is<timeline2::action_do>())
        {
            auto fn = SSVOH_MOVE(a.as<timeline2::action_do>()._func);
            ++_current_idx;
            fn();
            continue;
        }

        const outcome o = a.linearMatch(
            [&](timeline2::action_do&)
        {
            // Already handled above; unreachable. Kept to satisfy the
            // exhaustive-match requirement of `linearMatch`.
            SSVOH_ASSERT(false);
            return outcome::proceed;
        },
            [&](timeline2::action_wait_for& x)
        {
            if (!_wait_start_tp.hasValue())
            {
                // Just started waiting.
                _wait_start_tp.emplace(tp);
            }

            const auto elapsed = tp - _wait_start_tp.value();
            if (elapsed < x._duration)
            {
                // Still waiting.
                return outcome::waiting;
            }

            // Finished waiting.
            _wait_start_tp.reset();
            return outcome::proceed;
        },
            [&](timeline2::action_wait_until& x)
        {
            if (tp < x._time_point)
            {
                // Still waiting.
                return outcome::waiting;
            }

            // Finished waiting.
            return outcome::proceed;
        }, //
            [&](timeline2::action_wait_until_fn& x)
        {
            if (tp < x._time_point_fn())
            {
                // Still waiting.
                return outcome::waiting;
            }

            // Finished waiting.
            return outcome::proceed;
        } //
        );

        if (o == outcome::proceed)
        {
            ++_current_idx;
            continue;
        }

        if (o == outcome::waiting)
        {
            return o;
        }

        SSVOH_ASSERT(false);
    }

    return outcome::finished;
}

} // namespace hg::Utils
