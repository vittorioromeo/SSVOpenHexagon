// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <sodium.h>

#include <array>
#include <string>
#include <cstddef>
#include <optional>

namespace hg
{

inline constexpr std::size_t sodiumPublicKeyBytes = crypto_kx_PUBLICKEYBYTES;
inline constexpr std::size_t sodiumSecretKeyBytes = crypto_kx_SECRETKEYBYTES;
inline constexpr std::size_t sodiumReceiveKeyBytes = crypto_kx_SESSIONKEYBYTES;
inline constexpr std::size_t sodiumTransmitKeyBytes = crypto_kx_SESSIONKEYBYTES;

using SodiumPublicKeyArray = std::array<unsigned char, sodiumPublicKeyBytes>;
using SodiumSecretKeyArray = std::array<unsigned char, sodiumSecretKeyBytes>;
using SodiumReceiveKeyArray = std::array<unsigned char, sodiumReceiveKeyBytes>;
using SodiumTransmitKeyArray =
    std::array<unsigned char, sodiumTransmitKeyBytes>;

struct SodiumPSKeys
{
    SodiumPublicKeyArray keyPublic;
    SodiumSecretKeyArray keySecret;
};

struct SodiumRTKeys
{
    SodiumReceiveKeyArray keyReceive;
    SodiumTransmitKeyArray keyTransmit;
};

[[nodiscard]] SodiumPSKeys generateSodiumPSKeys();

[[nodiscard]] std::optional<SodiumRTKeys> calculateServerSessionSodiumRTKeys(
    const SodiumPSKeys& serverPSKeys,
    const SodiumPublicKeyArray& clientPublicKey);

[[nodiscard]] std::optional<SodiumRTKeys> calculateClientSessionSodiumRTKeys(
    const SodiumPSKeys& clientPSKeys,
    const SodiumPublicKeyArray& serverPublicKey);

template <typename T>
[[nodiscard]] std::string sodiumKeyToString(const T& sodiumKey)
{
    std::string result;
    result.reserve(sodiumKey.size());

    for(const unsigned char c : sodiumKey)
    {
        result += std::to_string(static_cast<int>(c));
    }

    return result;
}



} // namespace hg
