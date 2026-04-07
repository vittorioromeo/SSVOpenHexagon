// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Sodium.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"

#include <sodium.h>


namespace hg
{

[[nodiscard]] SodiumNonceArray generateNonce()
{
    SodiumNonceArray result;
    randombytes_buf(result.data(), result.size());
    return result;
}

[[nodiscard]] SodiumPSKeys generateSodiumPSKeys()
{
    SodiumPSKeys result;
    crypto_kx_keypair(result.keyPublic.data(), result.keySecret.data());
    return result;
}

[[nodiscard]] sf::base::Optional<SodiumRTKeys> calculateServerSessionSodiumRTKeys(const SodiumPSKeys& serverPSKeys,
                                                                                  const SodiumPublicKeyArray& clientPublicKey)
{
    SodiumRTKeys result;

    if (crypto_kx_server_session_keys(result.keyReceive.data(),
                                      result.keyTransmit.data(),
                                      serverPSKeys.keyPublic.data(),
                                      serverPSKeys.keySecret.data(),
                                      clientPublicKey.data()) != 0)
    {
        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(result);
}

[[nodiscard]] sf::base::Optional<SodiumRTKeys> calculateClientSessionSodiumRTKeys(const SodiumPSKeys& clientPSKeys,
                                                                                  const SodiumPublicKeyArray& serverPublicKey)
{
    SodiumRTKeys result;

    if (crypto_kx_client_session_keys(result.keyReceive.data(),
                                      result.keyTransmit.data(),
                                      clientPSKeys.keyPublic.data(),
                                      clientPSKeys.keySecret.data(),
                                      serverPublicKey.data()) != 0)
    {
        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(result);
}

// ----------------------------------------------------------------------------

[[nodiscard]] sf::base::String sodiumHash(const sf::base::String& in)
{
    constexpr sf::base::Array<unsigned char, crypto_generichash_KEYBYTES> key{};

    sf::base::String out;
    out.resize(crypto_generichash_BYTES);

    crypto_generichash(reinterpret_cast<unsigned char*>(out.data()),
                       out.size(),
                       reinterpret_cast<const unsigned char*>(in.data()),
                       in.size(),
                       key.data(),
                       key.size());

    return out;
}

[[nodiscard]] sf::base::U64 randomUInt64()
{
    sf::base::U64 result;
    randombytes_buf(static_cast<void*>(&result), sizeof(result));
    return result;
}

} // namespace hg
