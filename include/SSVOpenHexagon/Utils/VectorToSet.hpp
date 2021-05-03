// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <vector>
#include <unordered_set>

namespace hg::Utils {

template <typename T>
[[nodiscard]] std::unordered_set<T> toUnorderedSet(const std::vector<T>& v)
{
    return std::unordered_set<T>(v.begin(), v.end());
}

template <typename T>
[[nodiscard]] std::vector<T> toVector(const std::unordered_set<T>& s)
{
    return std::vector<T>(s.begin(), s.end());
}

} // namespace hg::Utils
