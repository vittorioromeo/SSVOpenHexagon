// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Macros.hpp"

#include "SFML/Base/StringView.hpp"

#include <algorithm>
#include <vector>

namespace hg::Utils
{

template <typename TSplitType = sf::base::StringView, typename F>
void withSplit(F&& f, const sf::base::StringView str, const sf::base::StringView delims = " ")
{
    for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last;
         first = second + 1)
    {
        second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));

        if (first != second)
        {
            f(TSplitType{first, static_cast<typename TSplitType::size_type>(second - first)});
        }
    }
}

template <typename TSplitType = sf::base::StringView>
[[nodiscard]] inline std::vector<TSplitType> split(const sf::base::StringView str,
                                                   const sf::base::StringView delims = " ")
{
    std::vector<TSplitType> result;

    withSplit<TSplitType>([&](TSplitType&& piece) { result.emplace_back(SSVOH_MOVE(piece)); }, str, delims);

    return result;
}

} // namespace hg::Utils
