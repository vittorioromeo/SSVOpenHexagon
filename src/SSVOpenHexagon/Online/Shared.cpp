// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Shared.hpp"

#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"

#include <SFML/Network/Packet.hpp>

#include <sodium.h>

#include <cstdint>
#include <sstream>
#include <iostream>
#include <optional>
#include <type_traits>

namespace hg
{

namespace
{

template <typename...>
struct TypeList
{
};

template <typename T, typename... Ts>
[[nodiscard]] constexpr bool contains(TypeList<Ts...>)
{
    return (std::is_same_v<T, Ts> || ...);
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr std::size_t indexOfType(TypeList<Ts...>)
{
    static_assert(contains<T>(TypeList<Ts...>{}));

    constexpr std::array test{std::is_same_v<T, Ts>...};
    for(std::size_t i = 0; i < test.size(); ++i)
    {
        if(test[i])
        {
            return i;
        }
    }

    throw;
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr bool variantContains(TypeList<std::variant<Ts...>>)
{
    return contains<T>(TypeList<Ts...>{});
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr std::size_t indexOfVariantType(
    TypeList<std::variant<Ts...>>)
{
    return indexOfType<T>(TypeList<Ts...>{});
}

using PacketType = std::uint8_t;

template <typename T>
[[nodiscard]] constexpr PacketType getPacketType()
{
    if constexpr(variantContains<T>(TypeList<PVClientToServer>{}))
    {
        return indexOfVariantType<T>(TypeList<PVClientToServer>{});
    }
    else if constexpr(variantContains<T>(TypeList<PVServerToClient>{}))
    {
        return indexOfVariantType<T>(TypeList<PVServerToClient>{});
    }
    else
    {
        throw;
    }
}

static constexpr std::uint8_t preamble1stByte{'o'};
static constexpr std::uint8_t preamble2ndByte{'h'};

void encodePreamble(sf::Packet& p)
{
    p << static_cast<std::uint8_t>(preamble1stByte)
      << static_cast<std::uint8_t>(preamble2ndByte);
}

void encodeVersion(sf::Packet& p)
{
    p << static_cast<std::uint8_t>(GAME_VERSION.major)
      << static_cast<std::uint8_t>(GAME_VERSION.minor)
      << static_cast<std::uint8_t>(GAME_VERSION.micro);
}

void clearPacketAndEncodePreambleAndVersion(sf::Packet& p)
{
    p.clear();

    encodePreamble(p);
    encodeVersion(p);
}

template <typename T>
void encodePacketType(sf::Packet& p, const T&)
{
    p << static_cast<std::uint8_t>(getPacketType<T>());
}

template <typename T, typename = void>
struct Extractor
{
    [[nodiscard]] static bool extractInto(
        T& target, std::ostringstream& errorOss, sf::Packet& p)
    {
        if(!(p >> target))
        {
            errorOss << "Error extracting single object\n";
            return false;
        }

        return true;
    }
};

template <typename T, std::size_t N>
struct Extractor<std::array<T, N>>
{
    using Type = std::array<T, N>;

    [[nodiscard]] static bool extractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        for(std::size_t i = 0; i < N; ++i)
        {
            if(p >> result[i])
            {
                continue;
            }

            errorOss << "Error extracting array element at index '" << i
                     << "'\n";

            return false;
        }

        return true;
    }
};

template <typename T>
[[nodiscard]] bool extractInto(
    T& target, std::ostringstream& errorOss, sf::Packet& p)
{
    return Extractor<T>::extractInto(target, errorOss, p);
}

template <typename T>
[[nodiscard]] std::optional<T> extract(
    std::ostringstream& errorOss, sf::Packet& p)
{
    T temp;

    if(!Extractor<T>::extractInto(temp, errorOss, p))
    {
        return std::nullopt;
    }

    return {std::move(temp)};
}

template <typename T>
[[nodiscard]] auto makeMatcher(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name, const T& expected) -> bool {
        T temp;

        if(!extractInto<T>(temp, errorOss, p))
        {
            errorOss << "Error extracting " << name << '\n';
            return false;
        }

        if(temp != expected)
        {
            errorOss << "Error, " << name << " has value '" << temp
                     << ", which doesn't match expected value '" << expected
                     << "'\n";

            return false;
        }

        return true;
    };
}

template <typename T>
[[nodiscard]] auto makeExtractor(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name) -> std::optional<T> {
        T temp;

        if(!extractInto<T>(temp, errorOss, p))
        {
            errorOss << "Error extracting " << name << '\n';
            return std::nullopt;
        }

        return {std::move(temp)};
    };
}

[[nodiscard]] bool verifyReceivedPacketPreambleAndVersion(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const auto matchByte = makeMatcher<std::uint8_t>(errorOss, p);

    return matchByte("preamble 1st byte", preamble1stByte) &&
           matchByte("preamble 2nd byte", preamble2ndByte) &&
           matchByte("major version", GAME_VERSION.major) &&
           matchByte("minor version", GAME_VERSION.minor) &&
           matchByte("micro version", GAME_VERSION.micro);
}

[[nodiscard]] std::optional<PacketType> extractPacketType(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const std::optional<std::uint8_t> extracted =
        makeExtractor<std::uint8_t>(errorOss, p)("packet type");

    if(!extracted.has_value())
    {
        return std::nullopt;
    }

    return {static_cast<PacketType>(*extracted)};
}

template <std::size_t NExpected, typename T, std::size_t N>
void encodeArray(sf::Packet& p, const std::array<T, N>& arr)
{
    static_assert(NExpected == N);

    for(std::size_t i = 0; i < arr.size(); ++i)
    {
        p << arr[i];
    }
}

template <typename T>
void encodeDynamicRange(sf::Packet& p, const T* data, const std::size_t len)
{
    SSVOH_ASSERT(data != nullptr);

    for(std::size_t i = 0; i < len; ++i)
    {
        p << data[i];
    }
}

template <typename T>
void encodeFirstNVectorElements(
    sf::Packet& p, const std::vector<T>& data, const std::size_t len)
{
    SSVOH_ASSERT(data.size() >= len);

    for(std::size_t i = 0; i < len; ++i)
    {
        p << data[i];
    }
}

std::vector<std::uint8_t>& getStaticMessageBuffer()
{
    thread_local std::vector<std::uint8_t> result;
    return result;
}

std::vector<std::uint8_t>& getStaticCiphertextBuffer()
{
    thread_local std::vector<std::uint8_t> result;
    return result;
}

sf::Packet& getStaticPacketBuffer()
{
    thread_local sf::Packet result;
    return result;
}

void encodeOHPacket(sf::Packet& p, const CTSPHeartbeat& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPDisconnect& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPPublicKey& data)
{
    encodePacketType(p, data);
    encodeArray<sodiumPublicKeyBytes>(p, data.key);
}

void encodeOHPacket(sf::Packet& p, const CTSPReady& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPPrint& data)
{
    encodePacketType(p, data);
    p << data.msg;
}

void encodeOHPacket(sf::Packet& p, const PEncryptedMsg& data)
{
    encodePacketType(p, data);

    encodeArray<sodiumNonceBytes>(p, data.nonce);
    p << data.messageLength;
    p << data.ciphertextLength;
    encodeFirstNVectorElements(p, *data.ciphertext, data.ciphertextLength);
}

void encodeOHPacket(sf::Packet& p, const STCPKick& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const STCPPublicKey& data)
{
    encodePacketType(p, data);

    for(std::size_t i = 0; i < sodiumPublicKeyBytes; ++i)
    {
        p << data.key[i];
    }
}

[[nodiscard]] bool decryptPacket(std::ostringstream& errorOss, sf::Packet& p,
    const SodiumReceiveKeyArray& keyReceive, sf::Packet& decryptedPacket)
{
    SodiumNonceArray nonce;
    if(!extractInto(nonce, errorOss, p))
    {
        errorOss << "Error decoding client nonce\n";
        return false;
    }

    std::uint64_t messageLength;
    if(!extractInto(messageLength, errorOss, p))
    {
        errorOss << "Error decoding client message length\n";
        return false;
    }

    std::uint64_t ciphertextLength;
    if(!extractInto(ciphertextLength, errorOss, p))
    {
        errorOss << "Error decoding client ciphertext length\n";
        return false;
    }

    std::vector<std::uint8_t>& ciphertext = getStaticCiphertextBuffer();
    ciphertext.resize(ciphertextLength);

    for(std::size_t i = 0; i < ciphertextLength; ++i)
    {
        if(p >> ciphertext[i])
        {
            continue;
        }

        errorOss << "Error decoding client ciphertext at index '" << i << "'\n";

        return false;
    }

    std::vector<std::uint8_t>& message = getStaticMessageBuffer();
    message.resize(messageLength);

    if(crypto_secretbox_open_easy(message.data(), ciphertext.data(),
           ciphertextLength, nonce.data(), keyReceive.data()) != 0)
    {
        errorOss << "Failure decrypting encrypted client message\n";
        return false;
    }

    decryptedPacket.clear();
    decryptedPacket.append(message.data(), messageLength);

    return true;
}

} // namespace

// ----------------------------------------------------------------------------

template <typename T>
void makeClientToServerPacket(sf::Packet& p, const T& data)
{
    clearPacketAndEncodePreambleAndVersion(p);
    encodeOHPacket(p, data);
}

template void makeClientToServerPacket(sf::Packet&, const PEncryptedMsg&);
template void makeClientToServerPacket(sf::Packet&, const CTSPHeartbeat&);
template void makeClientToServerPacket(sf::Packet&, const CTSPDisconnect&);
template void makeClientToServerPacket(sf::Packet&, const CTSPPublicKey&);
template void makeClientToServerPacket(sf::Packet&, const CTSPReady&);
template void makeClientToServerPacket(sf::Packet&, const CTSPPrint&);

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    sf::Packet& packetToEncrypt = getStaticPacketBuffer();
    packetToEncrypt.clear();

    encodeOHPacket(packetToEncrypt, data);

    PEncryptedMsg encryptedMsg{
        .nonce = generateNonce(),
        .messageLength = packetToEncrypt.getDataSize(),
        .ciphertextLength = getCiphertextLength(packetToEncrypt.getDataSize())
        //
    };

    encryptedMsg.ciphertext = &getStaticCiphertextBuffer();
    encryptedMsg.ciphertext->resize(encryptedMsg.ciphertextLength);

    if(crypto_secretbox_easy(encryptedMsg.ciphertext->data(),
           static_cast<const std::uint8_t*>(packetToEncrypt.getData()),
           encryptedMsg.messageLength, encryptedMsg.nonce.data(),
           keyTransmit.data()) != 0)
    {
        return false;
    }

    makeClientToServerPacket(p, encryptedMsg);
    return true;
}

template bool makeClientToServerEncryptedPacket(
    const SodiumTransmitKeyArray&, sf::Packet&, const CTSPPrint&);

// ----------------------------------------------------------------------------

[[nodiscard]] static PVClientToServer decodeClientToServerPacketInner(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    const std::optional<PacketType> pt = extractPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == getPacketType<PEncryptedMsg>())
    {
        if(keyReceive == nullptr)
        {
            errorOss << "Cannot decode client encrypted message without "
                        "receive key\n";

            return {PInvalid{.error = errorOss.str()}};
        }

        sf::Packet& decryptedPacket = getStaticPacketBuffer();
        if(!decryptPacket(errorOss, p, *keyReceive, decryptedPacket))
        {
            return {PInvalid{.error = errorOss.str()}};
        }

        return decodeClientToServerPacketInner(
            keyReceive, errorOss, decryptedPacket);
    }

    if(*pt == getPacketType<CTSPHeartbeat>())
    {
        return {CTSPHeartbeat{}};
    }

    if(*pt == getPacketType<CTSPDisconnect>())
    {
        return {CTSPDisconnect{}};
    }

    if(*pt == getPacketType<CTSPPublicKey>())
    {
        CTSPPublicKey result;

        if(!extractInto(result.key, errorOss, p))
        {
            return {PInvalid{.error = errorOss.str()}};
        }

        return {result};
    }

    if(*pt == getPacketType<CTSPReady>())
    {
        return {CTSPReady{}};
    }

    if(*pt == getPacketType<CTSPPrint>())
    {
        CTSPPrint result;

        if(!extractInto(result.msg, errorOss, p))
        {
            errorOss << "Error decoding client print message\n";
            return {PInvalid{.error = errorOss.str()}};
        }

        return {result};
    }

    errorOss << "Unknown packet type '" << static_cast<int>(*pt) << "'\n";
    return {PInvalid{.error = errorOss.str()}};
}

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    if(!verifyReceivedPacketPreambleAndVersion(errorOss, p))
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    return decodeClientToServerPacketInner(keyReceive, errorOss, p);
}

// ----------------------------------------------------------------------------

template <typename T>
void makeServerToClientPacket(sf::Packet& p, const T& data)
{
    clearPacketAndEncodePreambleAndVersion(p);
    encodeOHPacket(p, data);
}

template void makeServerToClientPacket(sf::Packet&, const PEncryptedMsg&);
template void makeServerToClientPacket(sf::Packet&, const STCPKick&);
template void makeServerToClientPacket(sf::Packet&, const STCPPublicKey&);

// ----------------------------------------------------------------------------

[[nodiscard]] static PVServerToClient decodeServerToClientPacketInner(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    const std::optional<PacketType> pt = extractPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == getPacketType<PEncryptedMsg>())
    {
        if(keyReceive == nullptr)
        {
            errorOss << "Cannot decode client encrypted message without "
                        "receive key\n";

            return {PInvalid{.error = errorOss.str()}};
        }

        sf::Packet& decryptedPacket = getStaticPacketBuffer();
        if(!decryptPacket(errorOss, p, *keyReceive, decryptedPacket))
        {
            return {PInvalid{.error = errorOss.str()}};
        }

        return decodeServerToClientPacketInner(
            keyReceive, errorOss, decryptedPacket);
    }

    if(*pt == getPacketType<STCPKick>())
    {
        return {STCPKick{}};
    }

    if(*pt == getPacketType<STCPPublicKey>())
    {
        STCPPublicKey result;

        if(!extractInto(result.key, errorOss, p))
        {
            return {PInvalid{.error = errorOss.str()}};
        }

        return {result};
    }

    errorOss << "Unknown packet type '" << static_cast<int>(*pt) << "'\n";
    return {PInvalid{.error = errorOss.str()}};
}

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    if(!verifyReceivedPacketPreambleAndVersion(errorOss, p))
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    return decodeServerToClientPacketInner(keyReceive, errorOss, p);
}

} // namespace hg
