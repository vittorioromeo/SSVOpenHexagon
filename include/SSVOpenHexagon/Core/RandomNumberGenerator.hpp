// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/RandomNumberGeneratorTypes.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"

#include "SFML/Base/IntTypes.hpp"

#include <pcg/pcg_random.hpp>
#include <random>


namespace hg
{

class random_number_generator
{
public:
    using seed_type  = random_number_generator_seed_type;
    using state_type = sf::base::U64;

private:
    seed_type  _seed;
    pcg32_fast _rng;

public:
    explicit random_number_generator(const seed_type seed) noexcept;

    [[nodiscard]] seed_type seed() const noexcept;

    template <typename T>
    [[nodiscard, gnu::always_inline]] inline T get_int(const T min, const T max) noexcept
    {
        SSVOH_ASSERT(min <= max);
        return std::uniform_int_distribution<T>{min, max}(_rng);
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] inline T get_real(const T min, const T max) noexcept
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
