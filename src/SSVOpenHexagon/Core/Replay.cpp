// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Replay.hpp"

#include <cassert>
#include <fstream>

namespace hg
{

#define SSVOH_TRY(...)                                                        \
    do                                                                        \
    {                                                                         \
        __VA_ARGS__;                                                          \
        if(!result._success)                                                  \
        {                                                                     \
            ::std::cerr << "Failed [de]serialization operation'" #__VA_ARGS__ \
                           "'\n";                                             \
                                                                              \
            return result;                                                    \
        }                                                                     \
    } while(false)

static auto make_write(serialization_result& result, std::byte*& buffer,
    const std::byte* const buffer_end)
{
    return [&result, &buffer, buffer_end](const auto& datum) {
        if(buffer + sizeof(datum) > buffer_end)
        {
            result._success = false;
            return;
        }

        std::memcpy(buffer, &datum, sizeof(datum));
        buffer += sizeof(datum);
        result._written_bytes += sizeof(datum);
    };
}

static auto make_read(deserialization_result& result, const std::byte*& buffer,
    const std::byte* const buffer_end)
{
    return [&result, &buffer, buffer_end](auto& target) {
        if(buffer + sizeof(target) > buffer_end)
        {
            result._success = false;
            return;
        }

        std::memcpy(&target, buffer, sizeof(target));
        buffer += sizeof(target);
        result._read_bytes += sizeof(target);
    };
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


[[nodiscard]] bool replay_data::operator==(
    const replay_data& rhs) const noexcept
{
    return _inputs == rhs._inputs;
}

[[nodiscard]] bool replay_data::operator!=(
    const replay_data& rhs) const noexcept
{
    return !(*this == rhs);
}

[[nodiscard]] serialization_result replay_data::serialize(
    std::byte* buffer, const std::size_t buffer_size)
{
    return serialize(buffer, buffer + buffer_size);
}

[[nodiscard]] deserialization_result replay_data::deserialize(
    const std::byte* buffer, const std::size_t buffer_size)
{
    return deserialize(buffer, buffer + buffer_size);
}

[[nodiscard]] serialization_result replay_data::serialize(
    std::byte* buffer, const std::byte* const buffer_end)
{
    serialization_result result;
    const auto write = make_write(result, buffer, buffer_end);

    const std::size_t n_inputs = _inputs.size();
    SSVOH_TRY(write(n_inputs));

    for(const input_bitset& ib : _inputs)
    {
        const std::uint8_t ib_byte = ib.to_ulong();
        SSVOH_TRY(write(ib_byte));
    }

    return result;
}

[[nodiscard]] deserialization_result replay_data::deserialize(
    const std::byte* buffer, const std::byte* const buffer_end)
{
    deserialization_result result;
    const auto read = make_read(result, buffer, buffer_end);

    std::size_t n_inputs;
    SSVOH_TRY(read(n_inputs));

    _inputs.resize(n_inputs);

    for(std::size_t i = 0; i < n_inputs; ++i)
    {
        std::uint8_t ib_byte;
        SSVOH_TRY(read(ib_byte));

        _inputs[i] = input_bitset{static_cast<unsigned long>(ib_byte)};
    }

    return result;
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

[[nodiscard]] bool replay_player::done() const noexcept
{
    return _current_index == _replay_data.size();
}


[[nodiscard]] bool replay_file::operator==(
    const replay_file& rhs) const noexcept
{
    return _version == rhs._version &&                 //
           _player_name == rhs._player_name &&         //
           _seed == rhs._seed &&                       //
           _data == rhs._data &&                       //
           _pack_id == rhs._pack_id &&                 //
           _level_id == rhs._level_id &&               //
           _difficulty_mult == rhs._difficulty_mult && //
           _played_frametime == rhs._played_frametime;
}

[[nodiscard]] bool replay_file::operator!=(
    const replay_file& rhs) const noexcept
{
    return !(*this == rhs);
}

[[nodiscard]] serialization_result replay_file::serialize(
    std::byte* buffer, const std::size_t buffer_size)
{
    return serialize(buffer, buffer + buffer_size);
}

[[nodiscard]] deserialization_result replay_file::deserialize(
    const std::byte* buffer, const std::size_t buffer_size)
{
    return deserialize(buffer, buffer + buffer_size);
}

[[nodiscard]] serialization_result replay_file::serialize(
    std::byte* buffer, const std::byte* const buffer_end)
{
    serialization_result result;
    const auto write = make_write(result, buffer, buffer_end);

    const auto write_str = [&](const std::string& s) {
        SSVOH_TRY(write(static_cast<std::uint32_t>(s.size())));

        for(const char c : s)
        {
            SSVOH_TRY(write(c));
        }

        return result;
    };

    SSVOH_TRY(write(_version));
    SSVOH_TRY(write_str(_player_name));
    SSVOH_TRY(write(_seed));

    const serialization_result data_result =
        _data.serialize(buffer, buffer_end);

    if(!data_result._success)
    {
        result._success = false;
        return result;
    }

    buffer += data_result._written_bytes;
    result._written_bytes += data_result._written_bytes;

    SSVOH_TRY(write_str(_pack_id));
    SSVOH_TRY(write_str(_level_id));
    SSVOH_TRY(write(_difficulty_mult));
    SSVOH_TRY(write(_played_frametime));

    return result;
}

[[nodiscard]] deserialization_result replay_file::deserialize(
    const std::byte* buffer, const std::byte* const buffer_end)
{
    deserialization_result result;
    const auto read = make_read(result, buffer, buffer_end);

    const auto read_str = [&](std::string& s) {
        std::uint32_t s_size;
        SSVOH_TRY(read(s_size));

        s.resize(s_size);

        for(std::uint32_t i = 0; i < s_size; ++i)
        {
            char c;
            SSVOH_TRY(read(c));

            s[i] = c;
        }

        return result;
    };

    SSVOH_TRY(read(_version));
    SSVOH_TRY(read_str(_player_name));
    SSVOH_TRY(read(_seed));

    const deserialization_result data_result =
        _data.deserialize(buffer, buffer_end);

    if(!data_result._success)
    {
        result._success = false;
        return result;
    }

    buffer += data_result._read_bytes;

    SSVOH_TRY(read_str(_pack_id));
    SSVOH_TRY(read_str(_level_id));
    SSVOH_TRY(read(_difficulty_mult));
    SSVOH_TRY(read(_played_frametime));

    return result;
}

[[nodiscard]] bool replay_file::serialize_to_file(const std::filesystem::path p)
{
    constexpr std::size_t buf_size{2048};
    std::byte buf[buf_size];

    const serialization_result sr = serialize(buf, buf_size);
    if(!sr)
    {
        return false;
    }

    const std::size_t written_bytes = sr.written_bytes();

    std::ofstream os(p, std::ios::binary | std::ios::out);
    os.write(reinterpret_cast<const char*>(buf), written_bytes);
    os.flush();

    return static_cast<bool>(os);
}

[[nodiscard]] bool replay_file::deserialize_from_file(
    const std::filesystem::path p)
{
    constexpr std::size_t buf_size{2048};
    std::byte buf[buf_size];

    std::ifstream is(p, std::ios::binary | std::ios::in);

    is.seekg(0, std::ios::end);
    const std::size_t bytes_to_read = is.tellg();
    is.seekg(0, std::ios::beg);

    is.read(reinterpret_cast<char*>(buf), bytes_to_read);

    if(!static_cast<bool>(is))
    {
        return false;
    }

    const deserialization_result dr = deserialize(buf, bytes_to_read);
    return static_cast<bool>(dr);
}

} // namespace hg
