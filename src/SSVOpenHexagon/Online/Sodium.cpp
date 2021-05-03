// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <sodium.h>

#include <vector>
#include <optional>

namespace hg {

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

// ----------------------------------------------------------------------------

[[nodiscard]] std::string sodiumHash(const std::string& in)
{
    constexpr std::array<unsigned char, crypto_generichash_KEYBYTES> key{};

    std::string out;
    out.resize(crypto_generichash_BYTES);

    crypto_generichash(reinterpret_cast<unsigned char*>(out.data()), out.size(),
        reinterpret_cast<const unsigned char*>(in.data()), in.size(),
        key.data(), key.size());

    return out;
}

[[nodiscard]] std::uint64_t randomUInt64()
{
    std::uint64_t result;
    randombytes_buf(static_cast<void*>(&result), sizeof(result));
    return result;
}

} // namespace hg
