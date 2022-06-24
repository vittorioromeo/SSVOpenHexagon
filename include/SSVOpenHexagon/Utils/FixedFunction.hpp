// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Macros.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace hg::Utils {

template <typename TSignature, std::size_t TStorageSize = 64>
class FixedFunction;

template <typename TReturn, typename... Ts, std::size_t TStorageSize>
class FixedFunction<TReturn(Ts...), TStorageSize>
{
private:
    using ret_type = TReturn;

    using fn_ptr_type = ret_type (*)(Ts...);
    using method_type = ret_type (*)(std::byte*, fn_ptr_type, Ts...);
    using alloc_type = void (*)(std::byte*, void* object_ptr);

    union
    {
        alignas(std::max_align_t) std::byte _storage[TStorageSize];
        fn_ptr_type _function_ptr;
    };

    method_type _method_ptr;
    alloc_type _alloc_ptr;

    void moveImpl(FixedFunction& o) noexcept
    {
        SSVOH_ASSERT(this != &o);

        if(_alloc_ptr)
        {
            _alloc_ptr(_storage, nullptr);
            _alloc_ptr = nullptr;
        }
        else
        {
            _function_ptr = nullptr;
        }

        _method_ptr = o._method_ptr;
        o._method_ptr = nullptr;

        if(o._alloc_ptr)
        {
            _alloc_ptr = o._alloc_ptr;
            _alloc_ptr(_storage, o._storage);
        }
        else
        {
            _function_ptr = o._function_ptr;
        }
    }

public:
    FixedFunction() noexcept
        : _function_ptr{nullptr}, _method_ptr{nullptr}, _alloc_ptr{nullptr}
    {}

    /**
     * @brief FixedFunction Constructor from functional object.
     * @param f Functor object will be stored in the internal storage
     * using move constructor. Unmovable objects are prohibited explicitly.
     */
    template <typename TFFwd>
    FixedFunction(TFFwd&& f) noexcept : FixedFunction()
    {
        using unref_type = std::remove_reference_t<TFFwd>;

        static_assert(sizeof(unref_type) < TStorageSize);
        static_assert(std::is_move_constructible_v<unref_type>);

        _method_ptr = [](std::byte* s, fn_ptr_type, Ts... xs)
        { return reinterpret_cast<unref_type*>(s)->operator()(xs...); };

        _alloc_ptr = [](std::byte* s, void* o)
        {
            if(o)
            {
                new(s) unref_type(std::move(*static_cast<unref_type*>(o)));
            }
            else
            {
                reinterpret_cast<unref_type*>(s)->~unref_type();
            }
        };

        _alloc_ptr(_storage, &f);
    }

    template <typename TFReturn, typename... TFs>
    FixedFunction(TFReturn (*f)(TFs...)) noexcept : FixedFunction()
    {
        _function_ptr = f;
        _method_ptr = [](std::byte*, fn_ptr_type xf, Ts... xs)
        { return static_cast<decltype(f)>(xf)(xs...); };
    }

    FixedFunction& operator=(const FixedFunction&) = delete;
    FixedFunction(const FixedFunction&) = delete;

    FixedFunction(FixedFunction&& rhs) noexcept : FixedFunction()
    {
        moveImpl(rhs);
    }

    FixedFunction& operator=(FixedFunction&& rhs) noexcept
    {
        moveImpl(rhs);
        return *this;
    }

    ~FixedFunction() noexcept
    {
        if(_alloc_ptr)
        {
            _alloc_ptr(_storage, nullptr);
        }
    }

    template <typename... TFwdTs>
    auto operator()(TFwdTs&&... xs) noexcept(
        noexcept(_method_ptr(_storage, _function_ptr, SSVOH_FWD(xs)...)))
    {
        SSVOH_ASSERT(_method_ptr != nullptr);
        return _method_ptr(_storage, _function_ptr, SSVOH_FWD(xs)...);
    }
};

} // namespace hg::Utils
