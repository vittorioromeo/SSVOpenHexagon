// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/Replay.hpp"

#include <cassert>

namespace hg
{

replay_data::replay_data(const seed_type seed) noexcept : _seed{seed}
{
}

void replay_data::record_input(const bool left, const bool right,
    const bool swap, const bool focus) noexcept
{
    input_bitset& ib = _inputs.emplace_back();
    ib[static_cast<unsigned int>(input_bit::left)] = left;
    ib[static_cast<unsigned int>(input_bit::right)] = right;
    ib[static_cast<unsigned int>(input_bit::swap)] = swap;
    ib[static_cast<unsigned int>(input_bit::focus)] = focus;
}

[[nodiscard]] replay_data::seed_type replay_data::get_seed() const noexcept
{
    return _seed;
}

[[nodiscard]] input_bitset replay_data::at(
    const std::size_t index) const noexcept
{
    assert(index < size());
    return _inputs[index];
}

[[nodiscard]] std::size_t replay_data::size() const noexcept
{
    return _inputs.size();
}

replay_player::replay_player(const replay_data& rd) noexcept
    : _replay_data{rd}, _current_index{0}
{
}

[[nodiscard]] input_bitset
replay_player::get_current_and_move_forward() noexcept
{
    if(_replay_data.size() <= _current_index)
    {
        return {};
    }

    return _replay_data.at(_current_index++);
}

} // namespace hg
