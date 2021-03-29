// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <algorithm>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace hg::Utils
{

template <typename T>
class sparse_integer_set
{
public:
    using dense_container_type = std::vector<T>;
    using sparse_container_type = std::vector<T*>;

    using value_type = T;
    using size_type = std::size_t;
    using iterator = dense_container_type::iterator;
    using const_iterator = dense_container_type::const_iterator;
    using reverse_iterator = dense_container_type::reverse_iterator;
    using const_reverse_iterator = dense_container_type::const_reverse_iterator;

private:
    dense_container_type _dense;
    sparse_container_type _sparse;

    [[gnu::always_inline, nodiscard]] bool valid_index(
        const size_type i) const noexcept
    {
        return _sparse.size() > i;
    }

    void reserve(const size_type new_capacity)
    {
        _dense.reserve(new_capacity);
        _sparse.reserve(new_capacity);
    }

public:
    void grow(const size_type new_size)
    {
        reserve(new_size);
        _sparse.resize(new_size);
    }

    [[gnu::always_inline]] void clear() noexcept
    {
        _dense.clear();
        std::fill(_sparse.begin(), _sparse.end(), nullptr);
    }

    [[gnu::always_inline, nodiscard]] bool has(const size_type i) const noexcept
    {
        SSVOH_ASSERT(valid_index(i));
        return _sparse[i] != nullptr;
    }

    [[gnu::always_inline]] void insert_unchecked(const size_type i)
    {
        SSVOH_ASSERT(valid_index(i));
        SSVOH_ASSERT(!has(i));

        _dense.push_back(i);
        _sparse[i] = &_dense.back();
    }

    [[gnu::always_inline, nodiscard]] bool insert(const size_type i)
    {
        SSVOH_ASSERT(valid_index(i));

        if(has(i))
        {
            return false;
        }

        insert_unchecked(i);
        return true;
    }

    [[gnu::always_inline]] void unchecked_erase(const size_type i)
    {
        SSVOH_ASSERT(valid_index(i));
        SSVOH_ASSERT(has(i));

        auto* const ptr = _sparse[i];

        const auto last_dense = _dense.back();
        *ptr = last_dense;
        _sparse[last_dense] = ptr;

        _sparse[i] = nullptr;
        _dense.pop_back();
    }

    [[gnu::always_inline, nodiscard]] bool erase(const size_type i)
    {
        SSVOH_ASSERT(valid_index(i));

        if(!has(i))
        {
            return false;
        }

        unchecked_erase(i);
        return true;
    }

    [[gnu::always_inline, nodiscard]] size_type dense_size() const noexcept
    {
        return _dense.size();
    }

    [[gnu::always_inline, nodiscard]] size_type sparse_size() const noexcept
    {
        return _sparse.size();
    }

    [[gnu::always_inline, nodiscard]] bool dense_empty() const noexcept
    {
        return _dense.empty();
    }

    [[gnu::always_inline, nodiscard]] const_iterator begin() const noexcept
    {
        return _dense.begin();
    }

    [[gnu::always_inline, nodiscard]] const_iterator end() const noexcept
    {
        return _dense.end();
    }

    [[gnu::always_inline, nodiscard]] const_reverse_iterator
    rbegin() const noexcept
    {
        return _dense.rbegin();
    }

    [[gnu::always_inline, nodiscard]] const_reverse_iterator
    rend() const noexcept
    {
        return _dense.rend();
    }
};

} // namespace hg::Utils
