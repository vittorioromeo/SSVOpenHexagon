// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/SizeT.hpp"

namespace ssvuj
{

template <typename TKey, typename TValue, typename THash, typename TKeyEqual, typename TAllocOrContainer, typename TBucket>
struct Converter<ankerl::unordered_dense::map<TKey, TValue, THash, TKeyEqual, TAllocOrContainer, TBucket>>
{
    using T = ankerl::unordered_dense::map<TKey, TValue, THash, TKeyEqual, TAllocOrContainer, TBucket>;

    static void fromObj(const Obj& mObj, T& mValue)
    {
        for (const auto& id : mObj.getMemberNames())
        {
            mValue.emplace(id, getExtr<typename T::mapped_type>(mObj[id]));
        }
    }

    static void toObj(Obj& mObj, const T& mValue)
    {
        for (const auto& [k, v] : mValue)
        {
            arch(mObj, k, v);
        }
    }
};

template <typename TKey, typename THash, typename TKeyEqual, typename TAllocOrContainer, typename TBucket>
struct Converter<ankerl::unordered_dense::set<TKey, THash, TKeyEqual, TAllocOrContainer, TBucket>>
{
    using T = ankerl::unordered_dense::set<TKey, THash, TKeyEqual, TAllocOrContainer, TBucket>;

    static void fromObj(const Obj& mObj, T& mValue)
    {
        for (sf::base::SizeT i = 0; i < getObjSize(mObj); ++i)
        {
            mValue.emplace(getExtr<TKey>(mObj, i));
        }
    }

    static void toObj(Obj& mObj, const T& mValue)
    {
        sf::base::SizeT idx = 0;
        for (const auto& k : mValue)
        {
            arch(mObj, idx++, k);
        }
    }
};

} // namespace ssvuj
