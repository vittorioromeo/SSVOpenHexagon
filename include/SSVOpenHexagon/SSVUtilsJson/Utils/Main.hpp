// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/TypeChecker.hpp"

#include <utility>

namespace ssvuj {
/// @brief Gets a JSON Obj from another JSON Obj.
/// @param mObj Source JSON Obj.
inline Obj& getObj(Obj& mObj) noexcept
{
    return mObj;
}
inline const Obj& getObj(const Obj& mObj) noexcept
{
    return mObj;
}

/// @brief Gets a JSON Obj from another JSON Obj.
/// @param mObj Source JSON Obj.
/// @param mKey Key of the child.
inline Obj& getObj(Obj& mObj, const Key& mKey) noexcept
{
    return mObj[mKey];
}
inline const Obj& getObj(const Obj& mObj, const Key& mKey) noexcept
{
    return mObj[mKey];
}

/// @brief Gets a JSON Obj from another JSON Obj.
/// @param mArray Source JSON Obj array.
/// @param mIdx Index of the child.
inline Obj& getObj(Obj& mArray, Idx mIdx) noexcept
{
    return mArray[mIdx];
}
inline const Obj& getObj(const Obj& mArray, Idx mIdx) noexcept
{
    return mArray[mIdx];
}

/// @brief Gets the size of a JSON Obj.
/// @param mObj Target JSON Obj.
inline auto getObjSize(const Obj& mObj) noexcept
{
    return mObj.size();
}

/// @brief Gets the size of a JSON Obj.
/// @param mObj Source Obj.
/// @param mKey Key of the target child Obj.
inline auto getObjSize(const Obj& mObj, const Key& mKey) noexcept
{
    return getObjSize(getObj(mObj, mKey));
}

/// @brief Gets the size of a JSON Obj.
/// @param mArray Source Obj.
/// @param mIdx Index of the target child Obj.
inline auto getObjSize(const Obj& mArray, Idx mIdx) noexcept
{
    return getObjSize(getObj(mArray, mIdx));
}

/// @brief Checks whether a JSON Obj has a certain member.
/// @param mObj Obj to check.
/// @param mKey Key of the child.
inline bool hasObj(const Obj& mObj, const Key& mKey) noexcept
{
    return mObj.isObject() && mObj.isMember(mKey);
}

/// @brief Checks whether a JSON Obj array has a certain member.
/// @param mArray Obj to check.
/// @param mIdx Index of the child.
inline bool hasObj(const Obj& mArray, Idx mIdx) noexcept
{
    return mArray.isArray() && mArray.isValidIndex(mIdx);
}

/// @brief Checks if a JSON Obj is an array.
/// @param mObj Obj to check.
inline bool isObjArray(const Obj& mObj) noexcept
{
    return mObj.isArray();
}

/// @brief Checks if a JSON Obj is an Obj.
/// @param mObj Obj to check.
inline bool isObj(const Obj& mObj) noexcept
{
    return mObj.isObject();
}

/// @brief Checks the type of a JSON Obj's value.
/// @param mObj Obj to check.
template <typename T>
inline bool isObjType(const Obj& mObj) noexcept
{
    return Impl::isObjType<T>(mObj);
}

/// @brief Sets a JSON Obj to a certain value.
/// @param mObj Target Obj.
/// @param mValue Value to set.
template <typename T>
inline void arch(Obj& mObj, const T& mValue)
{
    Converter<T>::toObj(mObj, mValue);
}

/// @brief Sets a JSON Obj to a certain value.
/// @param mObj Source Obj.
/// @param mKey Key of the target child Obj.
/// @param mValue Value to set.
template <typename T>
inline void arch(Obj& mObj, const Key& mKey, const T& mValue)
{
    arch(getObj(mObj, mKey), mValue);
}

/// @brief Sets a JSON Obj to a certain value.
/// @param mArray Source Obj array.
/// @param mIdx Index of the target child Obj.
/// @param mValue Value to set.
template <typename T>
inline void arch(Obj& mArray, Idx mIdx, const T& mValue)
{
    arch(getObj(mArray, mIdx), mValue);
}

/// @brief Sets a value from a JSON Obj.
/// @details By default, this does not call the value's constructor.
/// @param mObj Source Obj.
/// @param mValue Value to be set.
template <typename T>
inline void extr(const Obj& mObj, T& mValue)
{
    Converter<T>::fromObj(mObj, mValue);
}

/// @brief Sets a value from a JSON Obj.
/// @details By default, this does not call the value's constructor.
/// @param mObj Source Obj.
/// @param mKey Key of the target child Obj.
/// @param mValue Value to be set.
template <typename T>
inline void extr(const Obj& mObj, const Key& mKey, T& mValue)
{
    extr(getObj(mObj, mKey), mValue);
}

/// @brief Sets a value from a JSON Obj.
/// @details By default, this does not call the value's constructor.
/// @param mArray Source Obj array.
/// @param mIdx Index of the target child Obj.
/// @param mValue Value to be set.
template <typename T>
inline void extr(const Obj& mArray, Idx mIdx, T& mValue)
{
    extr(getObj(mArray, mIdx), mValue);
}

/// @brief Converts and return an instance of a JSON Obj's value.
/// @param mObj Obj to convert.
template <typename T>
inline T getExtr(const Obj& mObj)
{
    T result;
    extr(mObj, result);
    return result;
}

/// @brief Converts and return an instance of a JSON Obj's value.
/// @param mObj Parent Obj.
/// @param mKey Key of the child Obj to convert.
template <typename T>
inline T getExtr(const Obj& mObj, const Key& mKey)
{
    return getExtr<T>(getObj(mObj, mKey));
}

/// @brief Converts and return an instance of a JSON Obj's value.
/// @param mArray Parent Obj array.
/// @param mIdx Index of the child Obj to convert.
template <typename T>
inline T getExtr(const Obj& mArray, Idx mIdx)
{
    return getExtr<T>(getObj(mArray, mIdx));
}

/// @brief Converts and return an instance of a JSON Obj's value.
/// @param mObj Parent Obj.
/// @param mKey Key of the child Obj to convert.
/// @param mDefault Default value to use, in case the Obj hasn't got the
/// desired
/// value.
template <typename T>
inline T getExtr(const Obj& mObj, const Key& mKey, const T& mDefault)
{
    return hasObj(mObj, mKey) ? getExtr<T>(mObj, mKey) : mDefault;
}

/// @brief Converts and return an instance of a JSON Obj's value.
/// @param mArray Parent Obj array.
/// @param mIdx Index of the child Obj to convert.
/// @param mDefault Default value to use, in case the Obj hasn't got the
/// desired
/// value.
template <typename T>
inline T getExtr(const Obj& mArray, Idx mIdx, const T& mDefault)
{
    return hasObj(mArray, mIdx) ? getExtr<T>(mArray, mIdx) : mDefault;
}

/// @brief Converts a value to a new JSON Obj instance and returns it.
/// @param mValue Value to archive.
template <typename T>
inline Obj getArch(const T& mValue)
{
    Obj result;
    arch(result, mValue);
    return result;
}

/// @brief Gets the key of a JSON Obj from an iterator.
/// @param mItr JSON iterator.
inline Key getKey(const Iterator& mItr) noexcept
{
    return getExtr<Key>(mItr.key());
}

/// @brief Gets the key of a JSON Obj from an iterator.
/// @param mItr JSON iterator.
inline Key getKey(const ConstIterator& mItr) noexcept
{
    return getExtr<Key>(mItr.key());
}

// Iterator support
inline auto begin(Obj& mObj) noexcept
{
    return mObj.begin();
}
inline auto end(Obj& mObj) noexcept
{
    return mObj.end();
}
inline auto begin(const Obj& mObj) noexcept
{
    return mObj.begin();
}
inline auto end(const Obj& mObj) noexcept
{
    return mObj.end();
}

} // namespace ssvuj
