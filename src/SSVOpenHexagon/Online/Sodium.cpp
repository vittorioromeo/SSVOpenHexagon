// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <sodium.h>

#include <optional>

namespace hg
{

[[nodiscard]] SodiumPSKeys generateSodiumPSKeys()
{
    SodiumPSKeys result;
    crypto_kx_keypair(result.keyPublic.data(), result.keySecret.data());
    return result;
}

[[nodiscard]] std::optional<SodiumRTKeys> calculateServerSessionSodiumRTKeys(
    const SodiumPSKeys& serverPSKeys,
    const SodiumPublicKeyArray& clientPublicKey)
{
    SodiumRTKeys result;

    if(crypto_kx_server_session_keys(result.keyReceive.data(),
           result.keyTransmit.data(), serverPSKeys.keyPublic.data(),
           serverPSKeys.keySecret.data(), clientPublicKey.data()) != 0)
    {
        return std::nullopt;
    }

    return result;
}

[[nodiscard]] std::optional<SodiumRTKeys> calculateClientSessionSodiumRTKeys(
    const SodiumPSKeys& clientPSKeys,
    const SodiumPublicKeyArray& serverPublicKey)
{
    SodiumRTKeys result;

    if(crypto_kx_client_session_keys(result.keyReceive.data(),
           result.keyTransmit.data(), clientPSKeys.keyPublic.data(),
           clientPSKeys.keySecret.data(), serverPublicKey.data()) != 0)
    {
        return std::nullopt;
    }

    return result;
}

} // namespace hg
