// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <vrm/pp/sep_to_str.hpp>

namespace hg::Impl {

[[gnu::cold]] void assertionFailure(
    const char* code, const char* file, const int line);

}

#ifndef NDEBUG

#define SSVOH_ASSERT(...)                                                  \
    do                                                                     \
    {                                                                      \
        constexpr const char* assert_code =                                \
            VRM_PP_SEP_TOSTR(" ", VRM_PP_EMPTY(), __VA_ARGS__);            \
                                                                           \
        if(!static_cast<bool>(__VA_ARGS__)) [[unlikely]]                   \
        {                                                                  \
            ::hg::Impl::assertionFailure(assert_code, __FILE__, __LINE__); \
        }                                                                  \
    }                                                                      \
    while(false)

#else

#define SSVOH_ASSERT(...)

#endif
