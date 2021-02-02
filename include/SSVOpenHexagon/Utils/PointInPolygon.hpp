// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Utils
{

template <typename TC, typename T>
[[gnu::always_inline, gnu::pure, nodiscard]] inline bool pointInPolygon(
    const TC& mVertices, T x, T y) noexcept
{
    bool result{false};

    for(decltype(mVertices.size()) i{0}, j{mVertices.size() - 1};
        i < mVertices.size(); j = i++)
    {
        const auto vI = mVertices[i];
        const auto vJ = mVertices[j];

        if(((vI.y > y) != (vJ.y > y)) &&
            (x < (vJ.x - vI.x) * (y - vI.y) / (vJ.y - vI.y) + vI.x))
        {
            result = !result;
        }
    }

    return result;
}

template <typename TC, typename T>
[[gnu::always_inline, gnu::pure, nodiscard]] inline bool pointInPolygonPointers(
    const TC& mVertices, T x, T y) noexcept
{
    bool result{false};

    for(decltype(mVertices.size()) i{0}, j{mVertices.size() - 1};
        i < mVertices.size(); j = i++)
    {
        const auto vI = mVertices[i];
        const auto vJ = mVertices[j];

        if(((vI->y > y) != (vJ->y > y)) &&
            (x < (vJ->x - vI->x) * (y - vI->y) / (vJ->y - vI->y) + vI->x))
        {
            result = !result;
        }
    }

    return result;
}

} // namespace hg::Utils
