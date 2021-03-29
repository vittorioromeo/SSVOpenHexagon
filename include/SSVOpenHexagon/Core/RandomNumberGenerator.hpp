// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <SSVUtils/Internal/PCG/PCG.hpp>

#include <cassert>
#include <random>

namespace hg
{

class random_number_generator
{
public:
    using seed_type = unsigned long long;

private:
    seed_type _seed;
    pcg32_fast _rng;

public:
    explicit random_number_generator(const seed_type seed) noexcept;

    [[nodiscard]] seed_type seed() const noexcept;

    template <typename T>
    [[nodiscard]] T get_int(const T min, const T max) noexcept
    {
        SSVOH_ASSERT(min <= max);
        return std::uniform_int_distribution<T>{min, max}(_rng);
    }

    template <typename T>
    [[nodiscard]] T get_real(const T min, const T max) noexcept
    {
        SSVOH_ASSERT(min <= max);
        return std::uniform_real_distribution<T>{min, max}(_rng);
    }
};

} // namespace hg
