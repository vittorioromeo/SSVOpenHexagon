// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Utils
{

// Generic `erase_if` over any associative container with `begin/end/erase`.
// Works for `std::unordered_set` / `std::unordered_map` and the
// `ankerl::unordered_dense` equivalents.
template <typename Container, typename Pred>
auto erase_if(Container& c, Pred pred) -> decltype(c.size())
{
    auto old_size = c.size();
    for (auto i = c.begin(), last = c.end(); i != last;)
    {
        if (pred(*i))
        {
            i = c.erase(i);
        }
        else
        {
            ++i;
        }
    }

    return old_size - c.size();
}

} // namespace hg::Utils
