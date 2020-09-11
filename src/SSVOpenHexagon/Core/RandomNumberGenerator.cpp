// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0


#include "SSVOpenHexagon/Core/RandomNumberGenerator.hpp"

#include <SSVUtils/Internal/PCG/PCG.hpp>

#include <random>

namespace hg
{

random_number_generator::random_number_generator(const seed_type seed) noexcept
    : _seed{seed}, _rng{seed}
{
}

[[nodiscard]] random_number_generator::seed_type
random_number_generator::seed() const noexcept
{
    return _seed;
}

} // namespace hg
