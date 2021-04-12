// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <cstdio>
#include <cstdlib>

namespace hg::Impl {

[[gnu::cold]] void assertionFailure(
    const char* code, const char* file, const int line)
{
    std::printf("ASSERTION FAILED\n    %s:%d\n    %s\n", file, line, code);
    std::abort();
}

} // namespace hg::Impl
