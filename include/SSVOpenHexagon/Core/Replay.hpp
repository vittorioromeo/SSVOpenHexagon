// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/RandomNumberGeneratorTypes.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

#include <bitset>
#include <filesystem>
#include <string>
#include <vector>

#include <cstring>

namespace sf
{

class Packet;

}

namespace hg
{

enum class input_bit : unsigned int
{
    left  = 0,
    right = 1,
    swap  = 2,
    focus = 3,

    k_count
};

using input_bitset = std::bitset<static_cast<unsigned int>(input_bit::k_count)>;

struct serialization_result
{
    sf::base::SizeT _written_bytes{0};
    bool            _success{true};

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return _success;
    }

    [[nodiscard]] sf::base::SizeT written_bytes() const noexcept
    {
        return _written_bytes;
    }
};

struct deserialization_result
{
    sf::base::SizeT _read_bytes{0};
    bool            _success{true};

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return _success;
    }

    [[nodiscard]] sf::base::SizeT read_bytes() const noexcept
    {
        return _read_bytes;
    }
};

class replay_data
{
private:
    std::vector<input_bitset> _inputs;

public:
    void record_input(const bool left, const bool right, const bool swap, const bool focus) noexcept;

    [[nodiscard]] input_bitset    at(const sf::base::SizeT index) const noexcept;
    [[nodiscard]] sf::base::SizeT size() const noexcept;

    [[nodiscard]] bool operator==(const replay_data& rhs) const noexcept;
    [[nodiscard]] bool operator!=(const replay_data& rhs) const noexcept;

    [[nodiscard]] serialization_result serialize(std::byte* buffer, const sf::base::SizeT buffer_size) const;

    [[nodiscard]] deserialization_result deserialize(const std::byte* buffer, const sf::base::SizeT buffer_size);

    [[nodiscard]] serialization_result serialize(std::byte* buffer, const std::byte* const buffer_end) const;

    [[nodiscard]] deserialization_result deserialize(const std::byte* buffer, const std::byte* const buffer_end);
};

class replay_player
{
private:
    const replay_data& _replay_data;
    sf::base::SizeT    _current_index;

public:
    explicit replay_player(const replay_data& rd) noexcept;

    [[nodiscard]] input_bitset get_current_and_move_forward() noexcept;
    [[nodiscard]] bool         done() const noexcept;
    void                       reset() noexcept;
};

struct replay_file
{
    using seed_type = random_number_generator_seed_type;

    sf::base::U32 _version;         // Replay format version.
    std::string   _player_name;     // Name of the player.
    seed_type     _seed;            // RNG seed for the session.
    replay_data   _data;            // Input data.
    std::string   _pack_id;         // Id of the selected pack.
    std::string   _level_id;        // Id of the played level.
    bool          _first_play;      // If this was achieved on first level play.
    float         _difficulty_mult; // Played difficulty multiplier.
    double        _played_score;    // Played score (This can be an overridden score or
                                    // frametime, excluding pauses).

    [[nodiscard]] bool operator==(const replay_file& rhs) const noexcept;
    [[nodiscard]] bool operator!=(const replay_file& rhs) const noexcept;

    [[nodiscard]] serialization_result serialize(std::byte* buffer, const sf::base::SizeT buffer_size) const;

    [[nodiscard]] deserialization_result deserialize(const std::byte* buffer, const sf::base::SizeT buffer_size);

    [[nodiscard]] serialization_result serialize(std::byte* buffer, const std::byte* const buffer_end) const;

    [[nodiscard]] deserialization_result deserialize(const std::byte* buffer, const std::byte* const buffer_end);

    [[nodiscard]] bool serialize_to_file(const std::filesystem::path& p) const;
    [[nodiscard]] bool deserialize_from_file(const std::filesystem::path& p);

    [[nodiscard]] bool serialize_to_packet(sf::Packet& p) const;
    [[nodiscard]] bool deserialize_from_packet(sf::Packet& p);

    [[nodiscard]] std::string create_filename() const;

    [[nodiscard]] double played_seconds() const noexcept;
};

struct compressed_replay_file
{
    std::vector<char> _data;

    [[nodiscard]] bool serialize_to_file(const std::filesystem::path& p) const;
    [[nodiscard]] bool deserialize_from_file(const std::filesystem::path& p);

    [[nodiscard]] bool serialize_to_packet(sf::Packet& p) const;
    [[nodiscard]] bool deserialize_from_packet(sf::Packet& p);
};

[[nodiscard]] sf::base::Optional<compressed_replay_file> compress_replay_file(const replay_file& rf);

[[nodiscard]] sf::base::Optional<replay_file> decompress_replay_file(const compressed_replay_file& crf);

} // namespace hg
