// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>

/// @macro Class mixin that allows SSVUJ converters to access the current
/// class's private members.
#define SSVUJ_CNV_FRIEND() \
    template <typename>    \
    friend struct ssvuj::Converter

/// @macro Shortcut to serialize a class member as an object with the same name
/// as the member.
#define SSVUJ_CNV_OBJ_AUTO(mValue, mVar) #mVar, mValue.mVar

/// @macro Defines a simple converter template specialization to convert classes
/// that do not require special behavior.
#define SSVUJ_CNV_SIMPLE(mType, mObjName, mValueName)                       \
    struct Converter<mType> final : ssvuj::Impl::ConverterSimpleImpl<mType> \
    {                                                                       \
        template <typename TObj, typename TValue>                           \
        static void impl(TObj mObjName, TValue mValueName)

/// @macro End macro, required after defining a simple converter.
#define SSVUJ_CNV_SIMPLE_END() }

namespace ssvuj
{
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

namespace Impl
{
template <typename T>
struct ConverterSimpleImpl
{
    static void fromObj(const Obj& mObjName, T& mValueName)
    {
        Converter<T>::template impl<const Obj&, T&>(mObjName, mValueName);
    }
    static void toObj(Obj& mObjName, const T& mValueName)
    {
        Converter<T>::template impl<Obj&, const T&>(mObjName, mValueName);
    }
};

template <typename T>
struct ConverterBaseImpl
{
    static void toObj(Obj& mObj, const T& mValue)
    {
        mObj = mValue;
    }
};

template <std::size_t I, typename TTpl>
using TplArg =
    std::tuple_element_t<I, std::remove_const_t<std::remove_reference_t<TTpl>>>;

template <std::size_t I = 0, typename... TArgs>
std::enable_if_t<I == sizeof...(TArgs)> toTpl(const Obj&, std::tuple<TArgs...>&)
{
}
template <std::size_t I = 0, typename... TArgs>
    std::enable_if_t <
    I<sizeof...(TArgs)> toTpl(const Obj& mObj, std::tuple<TArgs...>& mTpl)
{
    Converter<TplArg<I, decltype(mTpl)>>::fromObj(
        mObj[Idx(I)], std::get<I>(mTpl));
    toTpl<I + 1, TArgs...>(mObj, mTpl);
}

template <std::size_t I = 0, typename... TArgs>
std::enable_if_t<I == sizeof...(TArgs)> fromTpl(
    Obj&, const std::tuple<TArgs...>&)
{
}
template <std::size_t I = 0, typename... TArgs>
    std::enable_if_t <
    I<sizeof...(TArgs)> fromTpl(Obj& mObj, const std::tuple<TArgs...>& mTpl)
{
    Converter<TplArg<I, decltype(mTpl)>>::toObj(
        mObj[Idx(I)], std::get<I>(mTpl));
    fromTpl<I + 1, TArgs...>(mObj, mTpl);
}
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
template <typename TKey, typename TValue, typename TComp, typename TAlloc>
struct Converter<std::map<TKey, TValue, TComp, TAlloc>>
{
    using T = std::map<TKey, TValue, TComp, TAlloc>;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        for(auto& p : mObj)
        {
            const auto& valueKey(getExtr<TKey>(p, 0));
            extr(p, 1, mValue[valueKey]);
        }
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        Idx idx{0};
        for(const auto& p : mValue)
            arch<std::pair<TKey, TValue>>(getObj(mObj, idx++), p);
    }
};
template <typename TKey, typename TValue, typename THash, typename TKeyEqual,
    typename TAlloc>
struct Converter<std::unordered_map<TKey, TValue, THash, TKeyEqual, TAlloc>>
{
    using T = std::unordered_map<TKey, TValue, THash, TKeyEqual, TAlloc>;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        for(auto& p : mObj)
        {
            const auto& valueKey(getExtr<TKey>(p, 0));
            extr(p, 1, mValue[valueKey]);
        }
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        Idx idx{0};
        for(const auto& p : mValue)
            arch<std::pair<TKey, TValue>>(getObj(mObj, idx++), p);
    }
};

template <typename T1, typename T2>
struct Converter<std::pair<T1, T2>>
{
    using T = std::pair<T1, T2>;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        extr<Impl::TplArg<0, T>>(mObj, 0, std::get<0>(mValue));
        extr<Impl::TplArg<1, T>>(mObj, 1, std::get<1>(mValue));
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        arch<Impl::TplArg<0, T>>(mObj, 0, std::get<0>(mValue));
        arch<Impl::TplArg<1, T>>(mObj, 1, std::get<1>(mValue));
    }
};

template <typename... TArgs>
struct Converter<std::tuple<TArgs...>>
{
    using T = std::tuple<TArgs...>;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        Impl::toTpl(mObj, mValue);
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        Impl::fromTpl(mObj, mValue);
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
