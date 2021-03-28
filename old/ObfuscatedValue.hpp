// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVUtils/Encoding/Encoding.hpp>
#include <SSVUtils/Core/String/Utils.hpp>
#include <SSVUtils/Core/Assert/Assert.hpp>

#include <type_traits>
#include <string>
#include <sstream>

namespace hg
{

template <typename T,
    ssvu::Encoding::Type TCrypto = ssvu::Encoding::Type::Base64,
    typename TEnable = void>
class ObfuscatedValue;

/// @brief Base64 "obfuscated" value
/// @details Quick (but not really effective) way to protect a value against
/// memory scanners (such as Cheat Engine).
/// Obviously introduces a runtime cost to get/set the internal value.
/// @tparam T Type of the underlying arithmetic value.
template <typename T, ssvu::Encoding::Type TCrypto>
class ObfuscatedValue<T, TCrypto, std::enable_if_t<std::is_arithmetic_v<T>>>
{
private:
    /// @brief Dummy value used to "fool" memory scanners.
    T dummy;

    /// @brief The "real" value, under the form of a Base64 string.
    std::string encodedValue;

    /// @brief Converts the encodedValue to the arithmetic type.
    /// @details Used internally.
    /// @param mStr String to convert to arithmetic type.
    T fromStr(const std::string& mStr) const
    {
        std::istringstream stream(mStr);
        T t;
        stream >> t;
        return t;
    }

public:
    /// @brief Constructs an ObfuscatedValue from an arithmetic value.
    /// @param mValue The arithmetic value that will be stored internally.
    ObfuscatedValue(const T& mValue)
    {
        set(mValue);
    }

    /// @brief Sets the internal Base64 string.
    /// @param mValue Value to use.
    void set(const T& mValue)
    {
        dummy = mValue;
        encodedValue = ssvu::Encoding::encode<TCrypto>(ssvu::toStr(mValue));
    }

    /// @brief Converts the internal Base64 string and returns the
    /// unobfuscated value.
    [[nodiscard]] T get() const
    {
        return fromStr(ssvu::Encoding::decode<TCrypto>(encodedValue));
    }

    /// @brief Implicit conversion to the obfuscated type.
    [[nodiscard]] operator T() const
    {
        return get();
    }

    /// @brief Adds a value to the internal one.
    /// @param mValue Value to add.
    T operator+=(const T& mValue)
    {
        set(get() + mValue);
        return get();
    }

    /// @brief Subtracts a value from the internal one.
    /// @param mValue Value to subtract.
    T operator-=(const T& mValue)
    {
        set(get() - mValue);
        return get();
    }

    /// @brief Multiplies the internal value by another value.
    /// @param mValue Value to multiply with.
    T operator*=(const T& mValue)
    {
        set(get() * mValue);
        return get();
    }

    /// @brief Divides the internal value by another value.
    /// @param mValue Value to divide with.
    T operator/=(const T& mValue)
    {
        SSVU_ASSERT(mValue != 0);
        set(get() / mValue);
        return get();
    }

    /// @brief Shortcut operator for set.
    /// @param mValue Value to use.
    void operator=(const T& mValue)
    {
        set(mValue);
    }
};

} // namespace hg
