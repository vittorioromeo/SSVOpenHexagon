// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Core/RandomNumberGeneratorTypes.hpp"

#include <SSVUtils/Internal/PCG/PCG.hpp>

#include <random>

namespace hg {

class random_number_generator
{
public:
    using engine_type = pcg32_fast;
    using seed_type = random_number_generator_seed_type;
    using state_type = engine_type::state_type;

private:
    seed_type _seed;
    engine_type _rng;

public:
    explicit random_number_generator(const seed_type seed) noexcept;

    [[nodiscard]] seed_type seed() const noexcept;

    template <typename T>
    [[nodiscard, gnu::always_inline]] inline T get_int(
        const T min, const T max) noexcept
    {
        SSVOH_ASSERT(min <= max);
        return std::uniform_int_distribution<T>{min, max}(_rng);
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] inline T get_real(
        const T min, const T max) noexcept
    {
        SSVOH_ASSERT(min <= max);
        return std::uniform_real_distribution<T>{min, max}(_rng);
    }

    [[gnu::always_inline]] inline void advance(const state_type delta) noexcept
    {
        _rng.advance(delta);
    }
};

} // namespace hg
