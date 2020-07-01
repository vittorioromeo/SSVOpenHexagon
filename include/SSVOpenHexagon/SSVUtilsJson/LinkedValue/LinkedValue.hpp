// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"

#include <utility>
#include <string>

namespace ssvuj
{

template <typename T>
class LinkedValue
{
private:
    const char* name;
    T value;

public:
    constexpr explicit LinkedValue(const char* mLinkedName) : name{mLinkedName}
    {
    }

    operator T() const noexcept
    {
        return value;
    }

    auto& operator=(const T& mValue)
    {
        value = mValue;
        return *this;
    }

    void syncFrom(const Obj& mObj)
    {
        extr(mObj, name, value);
    }

    void syncTo(Obj& mObj) const
    {
        arch(mObj, name, value);
    }
};

} // namespace ssvuj
