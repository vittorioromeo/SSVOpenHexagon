// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Impl {

[[gnu::cold]] void assertionFailure(
    const char* code, const char* file, const int line);

}

#ifndef NDEBUG

#define SSVOH_ASSERT(...)                                                  \
    do                                                                     \
    {                                                                      \
        if (!static_cast<bool>(__VA_ARGS__)) [[unlikely]]                  \
        {                                                                  \
            ::hg::Impl::assertionFailure(#__VA_ARGS__, __FILE__, __LINE__); \
        }                                                                  \
    }                                                                      \
    while (false)

#else

#define SSVOH_ASSERT(...)

#endif
