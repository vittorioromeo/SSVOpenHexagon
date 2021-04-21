// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace Json {
class Value;
}

namespace ssvuj {
using Obj = Json::Value;
}

namespace ssvuj {

template <typename T>
class LinkedValue
{
private:
    const char* const name;
    T value;
    const T defValue;

public:
    explicit LinkedValue(const char* mLinkedName, const T& mDefault);

    [[nodiscard]] operator T&() noexcept;
    [[nodiscard]] operator const T&() const noexcept;

    LinkedValue& operator=(const T& mValue);

    void syncFrom(const Obj& mObj);
    void syncTo(Obj& mObj) const;

    void resetToDefault();

    [[nodiscard]] const T& getDefault() const noexcept;
};

} // namespace ssvuj
