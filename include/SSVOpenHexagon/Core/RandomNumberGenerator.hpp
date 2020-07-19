// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVUtils/Internal/PCG/PCG.hpp>

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

    [[nodiscard]] seed_type get_seed() const noexcept;
};

} // namespace hg
