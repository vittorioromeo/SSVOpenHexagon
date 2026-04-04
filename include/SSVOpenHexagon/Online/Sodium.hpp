// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/Array.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

#include <sodium.h>
#include <string>

namespace hg
{

inline constexpr sf::base::SizeT sodiumPublicKeyBytes   = crypto_kx_PUBLICKEYBYTES;
inline constexpr sf::base::SizeT sodiumSecretKeyBytes   = crypto_kx_SECRETKEYBYTES;
inline constexpr sf::base::SizeT sodiumReceiveKeyBytes  = crypto_kx_SESSIONKEYBYTES;
inline constexpr sf::base::SizeT sodiumTransmitKeyBytes = crypto_kx_SESSIONKEYBYTES;
inline constexpr sf::base::SizeT sodiumNonceBytes       = crypto_secretbox_NONCEBYTES;

using SodiumPublicKeyArray   = sf::base::Array<unsigned char, sodiumPublicKeyBytes>;
using SodiumSecretKeyArray   = sf::base::Array<unsigned char, sodiumSecretKeyBytes>;
using SodiumReceiveKeyArray  = sf::base::Array<unsigned char, sodiumReceiveKeyBytes>;
using SodiumTransmitKeyArray = sf::base::Array<unsigned char, sodiumTransmitKeyBytes>;
using SodiumNonceArray       = sf::base::Array<unsigned char, sodiumNonceBytes>;

[[nodiscard]] inline constexpr sf::base::SizeT getCiphertextLength(const sf::base::SizeT messageLength)
{
    return crypto_secretbox_MACBYTES + messageLength;
}

[[nodiscard]] SodiumNonceArray generateNonce();

struct SodiumPSKeys
{
    SodiumPublicKeyArray keyPublic;
    SodiumSecretKeyArray keySecret;
};

struct SodiumRTKeys
{
    SodiumReceiveKeyArray  keyReceive;
    SodiumTransmitKeyArray keyTransmit;
};

[[nodiscard]] SodiumPSKeys generateSodiumPSKeys();

[[nodiscard]] sf::base::Optional<SodiumRTKeys> calculateServerSessionSodiumRTKeys(
    const SodiumPSKeys&         serverPSKeys,
    const SodiumPublicKeyArray& clientPublicKey);

[[nodiscard]] sf::base::Optional<SodiumRTKeys> calculateClientSessionSodiumRTKeys(
    const SodiumPSKeys&         clientPSKeys,
    const SodiumPublicKeyArray& serverPublicKey);

template <typename T>
[[nodiscard]] std::string sodiumKeyToString(const T& sodiumKey)
{
    std::string result;
    result.reserve(sodiumKey.size());

    for (const unsigned char c : sodiumKey)
    {
        result += std::to_string(static_cast<int>(c));
    }

    return result;
}

[[nodiscard]] std::string sodiumHash(const std::string& s);

[[nodiscard]] sf::base::U64 randomUInt64();

} // namespace hg
