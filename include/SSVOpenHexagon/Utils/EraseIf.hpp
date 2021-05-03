// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <unordered_set>

namespace hg::Utils {

// From:
// https://en.cppreference.com/w/cpp/container/unordered_set/erase_if

template <class Key, class Hash, class KeyEqual, class Alloc, class Pred>
typename std::unordered_set<Key, Hash, KeyEqual, Alloc>::size_type erase_if(
    std::unordered_set<Key, Hash, KeyEqual, Alloc>& c, Pred pred)
{
    auto old_size = c.size();
    for(auto i = c.begin(), last = c.end(); i != last;)
    {
        if(pred(*i))
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
