// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg::Utils
{

namespace Impl
{
[[nodiscard, gnu::always_inline]] inline const char* findFirstOf(
    const char* first,
    const char* last,
    const char* delimsBegin,
    const char* delimsEnd) noexcept
{
    for (; first != last; ++first)
        for (const char* d = delimsBegin; d != delimsEnd; ++d)
            if (*first == *d)
                return first;
    return last;
}
} // namespace Impl

template <typename TSplitType = sf::base::StringView, typename F>
void withSplit(F&& f, const sf::base::StringView str, const sf::base::StringView delims = " ")
{
    for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last;
         first = second + 1)
    {
        second = Impl::findFirstOf(first, last, delims.data(), delims.data() + delims.size());

        if (first != second)
        {
            f(TSplitType{first, static_cast<sf::base::SizeT>(second - first)});
        }
    }
}

template <typename TSplitType = sf::base::StringView>
[[nodiscard]] inline sf::base::Vector<TSplitType> split(const sf::base::StringView str,
                                                        const sf::base::StringView delims = " ")
{
    sf::base::Vector<TSplitType> result;

    withSplit<TSplitType>([&](TSplitType&& piece) { result.emplaceBack(SFML_BASE_MOVE(piece)); }, str, delims);

    return result;
}

} // namespace hg::Utils
