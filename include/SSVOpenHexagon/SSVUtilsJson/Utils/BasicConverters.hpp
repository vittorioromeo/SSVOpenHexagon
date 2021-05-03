// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"

#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ssvuj {
// Convert enums

template <typename T>
struct Converter
{
    static void fromObj(const Obj& mObj, T& mValue,
        std::enable_if_t<std::is_enum_v<T>>* = nullptr)
    {
        mValue = T(getExtr<std::underlying_type_t<T>>(mObj));
    }

    static void toObj(Obj& mObj, const T& mValue,
        std::enable_if_t<std::is_enum_v<T>>* = nullptr)
    {
        arch<std::underlying_type_t<T>>(
            mObj, std::underlying_type_t<T>(mValue));
    }
};

namespace Impl {

template <typename T>
struct ConverterBaseImpl
{
    static void toObj(Obj& mObj, const T& mValue)
    {
        mObj = mValue;
    }
};

} // namespace Impl

#define SSVUJ_IMPL_CNV_BASE(mType, ...)                                   \
    template <>                                                           \
    struct Converter<mType> final : ssvuj::Impl::ConverterBaseImpl<mType> \
    {                                                                     \
        using T = mType;                                                  \
        static void fromObj(const Obj& mObj, T& mValue)                   \
        {                                                                 \
            mValue = __VA_ARGS__;                                         \
        }                                                                 \
    }

SSVUJ_IMPL_CNV_BASE(Obj, mObj);
SSVUJ_IMPL_CNV_BASE(char, T(mObj.asInt()));
SSVUJ_IMPL_CNV_BASE(unsigned char, T(mObj.asInt()));
SSVUJ_IMPL_CNV_BASE(int, mObj.asInt());
SSVUJ_IMPL_CNV_BASE(float, mObj.asFloat());
SSVUJ_IMPL_CNV_BASE(double, mObj.asDouble());
SSVUJ_IMPL_CNV_BASE(bool, mObj.asBool());
SSVUJ_IMPL_CNV_BASE(std::string, mObj.asString());
SSVUJ_IMPL_CNV_BASE(const char*, mObj.asCString());

#undef SSVUJ_IMPL_CNV_BASE

template <>
struct Converter<long>
{
    using T = long;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        mValue = mObj.asLargestInt();
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        mObj = Json::Int64(mValue);
    }
};

template <>
struct Converter<unsigned int>
{
    using T = unsigned int;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        mValue = mObj.asUInt();
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        mObj = Json::UInt(mValue);
    }
};

template <>
struct Converter<unsigned short>
{
    using T = unsigned short;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        mValue = mObj.asUInt();
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        mObj = Json::UInt(mValue);
    }
};

template <>
struct Converter<unsigned long>
{
    using T = unsigned long;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        mValue = mObj.asLargestUInt();
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        mObj = Json::UInt64(mValue);
    }
};

template <typename TItem, typename TAlloc>
struct Converter<std::vector<TItem, TAlloc>>
{
    using T = std::vector<TItem, TAlloc>;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        const auto& size(getObjSize(mObj));
        mValue.resize(size);
        for(auto i(0u); i < size; ++i) extr(mObj, i, mValue[i]);
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        for(auto i(0u); i < mValue.size(); ++i) arch(mObj, i, mValue[i]);
    }
};

template <typename TKey, typename TValue, typename THash, typename TKeyEqual,
    typename TAlloc>
struct Converter<std::unordered_map<TKey, TValue, THash, TKeyEqual, TAlloc>>
{
    using T = std::unordered_map<TKey, TValue, THash, TKeyEqual, TAlloc>;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        for(const auto& id : mObj.getMemberNames())
        {
            mValue.emplace(id, getExtr<TValue>(mObj[id]));
        }
    }

    static void toObj(Obj& mObj, const T& mValue)
    {
        for(const auto& [k, v] : mValue)
        {
            arch(mObj, k, v);
        }
    }
};

template <typename TItem, std::size_t TN>
struct Converter<TItem[TN]>
{
    using T = TItem[TN];
    static void fromObj(const Obj& mObj, T& mValue)
    {
        for(auto i(0u); i < TN; ++i) extr(mObj, i, mValue[i]);
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        for(auto i(0u); i < TN; ++i) arch(mObj, i, mValue[i]);
    }
};

} // namespace ssvuj
