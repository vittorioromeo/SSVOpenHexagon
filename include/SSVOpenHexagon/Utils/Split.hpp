// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string_view>
#include <utility>
#include <algorithm>
#include <vector>

namespace hg::Utils {

template <typename TSplitType = std::string_view, typename F>
void withSplit(
    F&& f, const std::string_view str, const std::string_view delims = " ")
{
    for(auto first = str.data(), second = str.data(), last = first + str.size();
        second != last && first != last; first = second + 1)
    {
        second = std::find_first_of(
            first, last, std::cbegin(delims), std::cend(delims));

        if(first != second)
        {
            f(TSplitType{first,
                static_cast<typename TSplitType::size_type>(second - first)});
        }
    }
}

template <typename TSplitType = std::string_view>
[[nodiscard]] inline std::vector<TSplitType> split(
    const std::string_view str, const std::string_view delims = " ")
{
    std::vector<TSplitType> result;

    withSplit<TSplitType>([&](TSplitType&& piece)
        { result.emplace_back(std::move(piece)); },
        str, delims);

    return result;
}

} // namespace hg::Utils
