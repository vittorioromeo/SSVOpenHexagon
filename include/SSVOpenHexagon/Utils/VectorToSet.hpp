// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/Vector.hpp"

#include <unordered_set>

namespace hg::Utils
{

template <typename T>
[[nodiscard]] std::unordered_set<T> toUnorderedSet(const sf::base::Vector<T>& v)
{
    return std::unordered_set<T>(v.begin(), v.end());
}

template <typename T>
[[nodiscard]] sf::base::Vector<T> toVector(const std::unordered_set<T>& s)
{
    sf::base::Vector<T> result;
    result.reserve(s.size());
    for (const auto& elem : s)
        result.pushBack(elem);
    return result;
}

} // namespace hg::Utils
