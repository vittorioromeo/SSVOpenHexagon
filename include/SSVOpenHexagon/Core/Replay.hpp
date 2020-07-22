// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/RandomNumberGenerator.hpp"

#include <bitset>
#include <vector>
#include <cstddef>

namespace hg
{

enum class input_bit : unsigned int
{
    left = 0,
    right = 1,
    swap = 2,
    focus = 3,

    k_count
};

using input_bitset = std::bitset<static_cast<unsigned int>(input_bit::k_count)>;

class replay_data
{
public:
    using seed_type = random_number_generator::seed_type;

private:
    seed_type _seed;
    std::vector<input_bitset> _inputs;

public:
    explicit replay_data(const seed_type seed) noexcept;

    void record_input(const bool left, const bool right, const bool swap,
        const bool focus) noexcept;

    [[nodiscard]] seed_type get_seed() const noexcept;
    [[nodiscard]] input_bitset at(const std::size_t index) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
};

class replay_player
{
private:
    const replay_data& _replay_data;
    std::size_t _current_index;

public:
    explicit replay_player(const replay_data& rd) noexcept;

    [[nodiscard]] input_bitset get_current_and_move_forward() noexcept;
};

} // namespace hg
