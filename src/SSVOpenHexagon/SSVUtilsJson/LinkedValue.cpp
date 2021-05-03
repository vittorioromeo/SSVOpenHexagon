// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/SSVUtilsJson/LinkedValue/LinkedValue.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"

#include "SSVOpenHexagon/Global/UtilsJson.hpp"

#include <SSVStart/Input/Input.hpp>

#include <string>
#include <vector>

namespace ssvuj {

template <typename T>
LinkedValue<T>::LinkedValue(const char* mLinkedName, const T& mDefault)
    : name{mLinkedName}, value{mDefault}, defValue{mDefault}
{}

template <typename T>
[[nodiscard]] LinkedValue<T>::operator T&() noexcept
{
    return value;
}

template <typename T>
[[nodiscard]] LinkedValue<T>::operator const T&() const noexcept
{
    return value;
}

template <typename T>
LinkedValue<T>& LinkedValue<T>::operator=(const T& mValue)
{
    value = mValue;
    return *this;
}

template <typename T>
void LinkedValue<T>::syncFrom(const Obj& mObj)
{
    value = getExtr(mObj, name, defValue);
}

template <typename T>
void LinkedValue<T>::syncTo(Obj& mObj) const
{
    arch(mObj, name, value);
}

template <typename T>
void LinkedValue<T>::resetToDefault()
{
    value = defValue;
}

template <typename T>
[[nodiscard]] const T& LinkedValue<T>::getDefault() const noexcept
{
    return defValue;
}

template class LinkedValue<bool>;
template class LinkedValue<int>;
template class LinkedValue<float>;
template class LinkedValue<unsigned int>;
template class LinkedValue<unsigned short>;
template class LinkedValue<std::string>;
template class LinkedValue<std::vector<std::string>>;
template class LinkedValue<ssvs::Input::Trigger>;

} // namespace ssvuj
