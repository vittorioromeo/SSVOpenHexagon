// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/json.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Trait/IsEnum.hpp"
#include "SFML/Base/Trait/UnderlyingType.hpp"
#include "SFML/Base/Vector.hpp"

namespace ssvuj
{
// Convert enums

template <typename T>
struct Converter
{
    static void fromObj(const Obj& mObj, T& mValue)
        requires sf::base::isEnum<T>
    {
        mValue = T(getExtr<SFML_BASE_UNDERLYING_TYPE(T)>(mObj));
    }

    static void toObj(Obj& mObj, const T& mValue)
        requires sf::base::isEnum<T>
    {
        arch<SFML_BASE_UNDERLYING_TYPE(T)>(mObj, static_cast<SFML_BASE_UNDERLYING_TYPE(T)>(mValue));
    }
};

namespace Impl
{

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
SSVUJ_IMPL_CNV_BASE(const char*, mObj.asCString());

template <>
struct Converter<sf::base::String> final
{
    using T = sf::base::String;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        mValue = sf::base::String(mObj.asString());
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        mObj = mValue.cStr();
    }
};

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

template <typename TItem>
struct Converter<sf::base::Vector<TItem>>
{
    using T = sf::base::Vector<TItem>;
    static void fromObj(const Obj& mObj, T& mValue)
    {
        const auto& size(getObjSize(mObj));
        mValue.resize(size);
        for (auto i(0u); i < size; ++i)
            extr(mObj, i, mValue[i]);
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        for (auto i(0u); i < mValue.size(); ++i)
            arch(mObj, i, mValue[i]);
    }
};

// Generic key/value-pair container converter (works with std::unordered_map, ankerl maps, ...).
// Pulled out of the std::unordered_map specialization to avoid pulling `<unordered_map>` into
// every TU that includes `BasicConverters.hpp`. Specialize this for the actual map type at the
// callsite via the SSVUJ_DEFINE_KV_CONVERTER macro below, OR include
// `BasicConverters_StdUnorderedMap.hpp` for the legacy `std::unordered_map` specialization.
template <typename TMap>
struct KeyValueMapConverter
{
    using T = TMap;
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

template <typename TItem, sf::base::SizeT TN>
struct Converter<TItem[TN]>
{
    using T = TItem[TN];
    static void fromObj(const Obj& mObj, T& mValue)
    {
        for (auto i(0u); i < TN; ++i)
            extr(mObj, i, mValue[i]);
    }
    static void toObj(Obj& mObj, const T& mValue)
    {
        for (auto i(0u); i < TN; ++i)
            arch(mObj, i, mValue[i]);
    }
};

} // namespace ssvuj
