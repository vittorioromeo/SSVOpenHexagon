// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>

namespace ssvuj::Impl
{

template <typename T>
inline bool isObjType(const Obj& mObj) noexcept;

template <>
inline bool isObjType<Obj>(const Obj&) noexcept
{
    return true;
}
template <>
inline bool isObjType<char>(const Obj& mObj) noexcept
{
    return mObj.isInt();
}
template <>
inline bool isObjType<unsigned char>(const Obj& mObj) noexcept
{
    return mObj.isUInt();
}
template <>
inline bool isObjType<int>(const Obj& mObj) noexcept
{
    return mObj.isInt();
}
template <>
inline bool isObjType<float>(const Obj& mObj) noexcept
{
    return mObj.isDouble();
}
template <>
inline bool isObjType<double>(const Obj& mObj) noexcept
{
    return mObj.isDouble();
}
template <>
inline bool isObjType<bool>(const Obj& mObj) noexcept
{
    return mObj.isBool();
}
template <>
inline bool isObjType<const char*>(const Obj& mObj) noexcept
{
    return mObj.isString();
}
template <>
inline bool isObjType<std::string>(const Obj& mObj) noexcept
{
    return mObj.isString();
}
template <>
inline bool isObjType<long>(const Obj& mObj) noexcept
{
    return mObj.isInt64();
}
template <>
inline bool isObjType<unsigned int>(const Obj& mObj) noexcept
{
    return mObj.isUInt();
}
template <>
inline bool isObjType<unsigned long>(const Obj& mObj) noexcept
{
    return mObj.isUInt64();
}

} // namespace ssvuj::Impl
