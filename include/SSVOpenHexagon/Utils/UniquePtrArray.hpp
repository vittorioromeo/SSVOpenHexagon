// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"

namespace hg::Utils {

template <typename T>
class UniquePtrArray
{
private:
    using SizeT = decltype(sizeof(int));

    T* _ptr;

public:
    [[nodiscard, gnu::always_inline]] explicit UniquePtrArray() noexcept
        : _ptr{nullptr}
    {}

    [[nodiscard, gnu::always_inline]] explicit UniquePtrArray(T* ptr) noexcept
        : _ptr{ptr}
    {}

    [[gnu::always_inline]] ~UniquePtrArray() noexcept
    {
        delete[] _ptr;
    }

    UniquePtrArray(const UniquePtrArray&) = delete;
    UniquePtrArray& operator=(const UniquePtrArray&) = delete;

    [[nodiscard, gnu::always_inline]] UniquePtrArray(
        UniquePtrArray&& rhs) noexcept
        : _ptr{rhs._ptr}
    {
        rhs._ptr = nullptr;
    }

    UniquePtrArray& operator=(UniquePtrArray&& rhs) noexcept
    {
        delete[] _ptr;

        _ptr = rhs._ptr;
        rhs._ptr = nullptr;

        return *this;
    }

    [[nodiscard, gnu::always_inline]] T* get() noexcept
    {
        return _ptr;
    }

    [[nodiscard, gnu::always_inline]] const T* get() const noexcept
    {
        return _ptr;
    }

    [[nodiscard, gnu::always_inline]] T& operator[](const SizeT i) noexcept
    {
        SSVOH_ASSERT(_ptr != nullptr);
        return _ptr[i];
    }

    [[nodiscard, gnu::always_inline]] const T& operator[](
        const SizeT i) const noexcept
    {
        SSVOH_ASSERT(_ptr != nullptr);
        return _ptr[i];
    }

    [[nodiscard, gnu::always_inline]] bool operator==(
        const T* ptr) const noexcept
    {
        return _ptr == ptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator!=(
        const T* ptr) const noexcept
    {
        return _ptr != ptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator==(
        decltype(nullptr)) const noexcept
    {
        return _ptr == nullptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator!=(
        decltype(nullptr)) const noexcept
    {
        return _ptr != nullptr;
    }
};

template <typename T>
[[nodiscard, gnu::always_inline]] inline UniquePtrArray<T> makeUniqueArray(
    decltype(sizeof(int)) capacity)
{
    return UniquePtrArray<T>{new T[capacity]};
}

} // namespace hg::Utils
