// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/Base/Bitset.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/UnderlyingType.hpp"

namespace ssvs
{

inline constexpr sf::base::SizeT inputBitOffset{1};
inline constexpr sf::base::SizeT fingerCount{16};

using FingerID = unsigned int;

using FingerBitset = sf::base::Bitset<fingerCount>;
using KeyBitset    = sf::base::Bitset<sf::Keyboard::KeyCount + inputBitOffset>;
using BtnBitset    = sf::base::Bitset<sf::Mouse::ButtonCount + inputBitOffset>;

template <typename TEnum>
[[nodiscard]] constexpr int toIndex(const TEnum value) noexcept
{
    return static_cast<int>(static_cast<SFML_BASE_UNDERLYING_TYPE(TEnum)>(value) + inputBitOffset);
}

[[nodiscard]] inline constexpr bool getFingerBit(const FingerBitset& bitset, const FingerID finger) noexcept
{
    return bitset[finger];
}

[[nodiscard]] inline constexpr bool getKeyBit(const KeyBitset& bitset, const sf::Keyboard::Key key) noexcept
{
    return bitset[static_cast<sf::base::SizeT>(toIndex(key))];
}

[[nodiscard]] inline constexpr bool getBtnBit(const BtnBitset& bitset, const sf::Mouse::Button button) noexcept
{
    return bitset[static_cast<sf::base::SizeT>(toIndex(button))];
}

[[gnu::always_inline]] inline constexpr void setFingerBit(FingerBitset& bitset, const FingerID finger, const bool value) noexcept
{
    bitset.setBit(finger, value);
}

[[gnu::always_inline]] inline constexpr void setKeyBit(KeyBitset& bitset, const sf::Keyboard::Key key, const bool value) noexcept
{
    bitset.setBit(static_cast<sf::base::SizeT>(toIndex(key)), value);
}

[[gnu::always_inline]] inline constexpr void setBtnBit(BtnBitset& bitset, const sf::Mouse::Button button, const bool value) noexcept
{
    bitset.setBit(static_cast<sf::base::SizeT>(toIndex(button)), value);
}

} // namespace ssvs
