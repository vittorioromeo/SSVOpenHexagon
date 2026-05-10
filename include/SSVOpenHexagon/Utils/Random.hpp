// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "pcg/pcg_extras.hpp"
#include "pcg/pcg_random.hpp"

#include <random>

// Tiny random helpers used by graphics / particles / picking. These are
// drop-in replacements for the previously-used `ssvu::getRndR` /
// `ssvu::getRndI`: same thread-local PCG seeded from `std::random_device`,
// same upper-bound semantics (inclusive for real, exclusive for int).
//
// The simulation RNG used by replays / scoring lives elsewhere
// (`hg::random_number_generator`); these helpers are only for incidental
// non-deterministic effects (camera shake, decorative particles, music
// segment picks, ...).

namespace hg::Utils
{

[[nodiscard]] inline pcg32_fast& getRndEngine() noexcept
{
    thread_local pcg32_fast rng{pcg_extras::seed_seq_from<std::random_device>{}};
    return rng;
}

// Uniform real in `[mMin, mMax]`. Inclusive on both ends.
template <typename T>
[[nodiscard]] inline T getRndR(const T mMin, const T mMax) noexcept
{
    return std::uniform_real_distribution<T>{mMin, mMax}(getRndEngine());
}

// Uniform integer in `[mMin, mMax)`. Upper bound is **exclusive** -- this
// matches `ssvu::getRndI` (which subtracts 1 internally) and is what the
// call sites pass `container.size()` to.
template <typename T>
[[nodiscard]] inline T getRndI(const T mMin, const T mMax) noexcept
{
    return std::uniform_int_distribution<T>{mMin, static_cast<T>(mMax - 1)}(getRndEngine());
}

} // namespace hg::Utils
