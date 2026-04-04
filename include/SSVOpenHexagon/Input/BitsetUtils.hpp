// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include <bitset>
#include <type_traits>

#include <cstddef>

namespace ssvs
{

inline constexpr std::size_t inputBitOffset{1};
inline constexpr std::size_t fingerCount{16};

using FingerID     = unsigned int;
using FingerBitset = std::bitset<fingerCount>;
using KeyBitset    = std::bitset<sf::Keyboard::KeyCount + inputBitOffset>;
using BtnBitset    = std::bitset<sf::Mouse::ButtonCount + inputBitOffset>;

template <typename TEnum>
[[nodiscard]] constexpr int toIndex(const TEnum value) noexcept
{
    using Underlying = std::underlying_type_t<TEnum>;
    return static_cast<int>(static_cast<Underlying>(value) + inputBitOffset);
}

[[nodiscard]] inline FingerBitset::reference getFingerBit(FingerBitset& bitset, const FingerID finger) noexcept
{
    return bitset[finger];
}

[[nodiscard]] inline constexpr bool getFingerBit(const FingerBitset& bitset, const FingerID finger) noexcept
{
    return bitset[finger];
}

[[nodiscard]] inline KeyBitset::reference getKeyBit(KeyBitset& bitset, const sf::Keyboard::Key key) noexcept
{
    return bitset[toIndex(key)];
}

[[nodiscard]] inline BtnBitset::reference getBtnBit(BtnBitset& bitset, const sf::Mouse::Button button) noexcept
{
    return bitset[toIndex(button)];
}

[[nodiscard]] inline constexpr bool getKeyBit(const KeyBitset& bitset, const sf::Keyboard::Key key) noexcept
{
    return bitset[toIndex(key)];
}

[[nodiscard]] inline constexpr bool getBtnBit(const BtnBitset& bitset, const sf::Mouse::Button button) noexcept
{
    return bitset[toIndex(button)];
}

} // namespace ssvs
