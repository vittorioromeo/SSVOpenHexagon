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

void addPreamble(sf::Packet& p)
{
    p << static_cast<std::uint8_t>(preamble1stByte)
      << static_cast<std::uint8_t>(preamble2ndByte);
}

void addVersion(sf::Packet& p)
{
    p << static_cast<std::uint8_t>(GAME_VERSION.major)
      << static_cast<std::uint8_t>(GAME_VERSION.minor)
      << static_cast<std::uint8_t>(GAME_VERSION.micro);
}

template <typename T>
void addPacketType(sf::Packet& p, const T&)
{
    p << static_cast<std::uint8_t>(getPacketType<T>());
}


auto makeByteMatcher(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name, const std::uint8_t expected) {
        if(std::uint8_t tempByte; !(p >> tempByte))
        {
            errorOss << "Error extracting " << name << '\n';
            return false;
        }
        else if(tempByte != expected)
        {
            errorOss << "Error, " << name << " has value '" << tempByte
                     << ", which doesn't match expected value '" << expected
                     << "'\n";

            return false;
        }

        return true;
    };
}

auto makeByteExtractor(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name) -> std::optional<std::uint8_t> {
        std::uint8_t tempByte;

        if(!(p >> tempByte))
        {
            errorOss << "Error extracting " << name << '\n';
            return std::nullopt;
        }

        return {tempByte};
    };
}

[[nodiscard]] bool verifyReceivedPacketPreambleAndVersion(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const auto matchByte = makeByteMatcher(errorOss, p);

    return matchByte("preamble 1st byte", preamble1stByte) &&
           matchByte("preamble 2nd byte", preamble2ndByte) &&
           matchByte("major version", GAME_VERSION.major) &&
           matchByte("minor version", GAME_VERSION.minor) &&
           matchByte("micro version", GAME_VERSION.micro);
}

void initializePacketForSending(sf::Packet& p)
{
    p.clear();

    addPreamble(p);
    addVersion(p);
}

[[nodiscard]] std::optional<PacketType> decodeReceivedPacketAndGetPacketType(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const auto extractByte = makeByteExtractor(errorOss, p);
    const std::optional<std::uint8_t> extracted = extractByte("packet type");

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

std::vector<unsigned char>& getStaticMessageBuffer()
{
    thread_local std::vector<unsigned char> result;
    return result;
}

std::vector<unsigned char>& getStaticCiphertextBuffer()
{
    thread_local std::vector<unsigned char> result;
    return result;
}

void encodeOHPacket(sf::Packet& p, const CTSPHeartbeat& data)
{
    addPacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPDisconnect& data)
{
    addPacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPPublicKey& data)
{
    addPacketType(p, data);
    encodeArray<sodiumPublicKeyBytes>(p, data.key);
}

void encodeOHPacket(sf::Packet& p, const CTSPReady& data)
{
    addPacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPPrint& data)
{
    addPacketType(p, data);
    p << data.msg;
}

void encodeOHPacket(sf::Packet& p, const CTSPEncryptedMsg& data)
{
    addPacketType(p, data);

    encodeArray<sodiumNonceBytes>(p, data.nonce);
    p << data.messageLength;
    p << data.ciphertextLength;
    encodeFirstNVectorElements(p, data.ciphertext, data.ciphertextLength);
}

void encodeOHPacket(sf::Packet& p, const STCPKick& data)
{
    addPacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const STCPPublicKey& data)
{
    addPacketType(p, data);

    for(std::size_t i = 0; i < sodiumPublicKeyBytes; ++i)
    {
        p << data.key[i];
    }
}

} // namespace

// ----------------------------------------------------------------------------

template <typename T>
void makeClientToServerPacket(sf::Packet& p, const T& data)
{
    initializePacketForSending(p);
    encodeOHPacket(p, data);
}

template void makeClientToServerPacket(sf::Packet&, const CTSPHeartbeat&);
template void makeClientToServerPacket(sf::Packet&, const CTSPDisconnect&);
template void makeClientToServerPacket(sf::Packet&, const CTSPPublicKey&);
template void makeClientToServerPacket(sf::Packet&, const CTSPReady&);
template void makeClientToServerPacket(sf::Packet&, const CTSPPrint&);
template void makeClientToServerPacket(sf::Packet&, const CTSPEncryptedMsg&);

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    sf::Packet packetToEncrypt;
    encodeOHPacket(packetToEncrypt, data);

    CTSPEncryptedMsg encryptedMsg{
        .nonce = generateNonce(),
        .messageLength = packetToEncrypt.getDataSize(),
        .ciphertextLength = getCiphertextLength(packetToEncrypt.getDataSize())
        //
    };

    encryptedMsg.ciphertext.resize(encryptedMsg.ciphertextLength);
    if(crypto_secretbox_easy(encryptedMsg.ciphertext.data(),
           static_cast<const unsigned char*>(packetToEncrypt.getData()),
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
    std::ostringstream& errorOss, sf::Packet& p,
    const SodiumReceiveKeyArray* keyReceive)
{
    const std::optional<PacketType> pt =
        decodeReceivedPacketAndGetPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
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

        for(std::size_t i = 0; i < sodiumPublicKeyBytes; ++i)
        {
            if(p >> result.key[i])
            {
                continue;
            }

            errorOss << "Error decoding client public key packet at index '"
                     << i << "'\n";

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

        if(!(p >> result.msg))
        {
            errorOss << "Error decoding client print message\n";
            return {PInvalid{.error = errorOss.str()}};
        }

        return {result};
    }

    if(*pt == getPacketType<CTSPEncryptedMsg>())
    {
        if(keyReceive == nullptr)
        {
            errorOss << "Cannot decode client encrypted message without "
                        "receive key\n";

            return {PInvalid{.error = errorOss.str()}};
        }

        SodiumNonceArray nonce;
        for(std::size_t i = 0; i < sodiumNonceBytes; ++i)
        {
            if(p >> nonce[i])
            {
                continue;
            }

            errorOss << "Error decoding client nonce at index '" << i << "'\n";
            return {PInvalid{.error = errorOss.str()}};
        }

        std::size_t messageLength;
        if(!(p >> messageLength))
        {
            errorOss << "Error decoding client message length\n";
            return {PInvalid{.error = errorOss.str()}};
        }

        std::size_t ciphertextLength;
        if(!(p >> ciphertextLength))
        {
            errorOss << "Error decoding client ciphertext length\n";
            return {PInvalid{.error = errorOss.str()}};
        }

        std::vector<unsigned char> ciphertext;
        ciphertext.resize(ciphertextLength);

        for(std::size_t i = 0; i < ciphertextLength; ++i)
        {
            if(p >> ciphertext[i])
            {
                continue;
            }

            errorOss << "Error decoding client ciphertext at index '" << i
                     << "'\n";

            return {PInvalid{.error = errorOss.str()}};
        }

        std::vector<unsigned char> message;
        message.resize(messageLength);

        SSVOH_ASSERT(keyReceive != nullptr);

        if(crypto_secretbox_open_easy(message.data(), ciphertext.data(),
               ciphertextLength, nonce.data(), keyReceive->data()) != 0)
        {
            errorOss << "Failure decrypting encrypted client message\n";
            return {PInvalid{.error = errorOss.str()}};
        }

        sf::Packet decryptedPacket;
        decryptedPacket.append(message.data(), messageLength);

        return decodeClientToServerPacketInner(
            errorOss, decryptedPacket, keyReceive);
    }

    errorOss << "Unknown packet type '" << static_cast<int>(*pt) << "'\n";
    return {PInvalid{.error = errorOss.str()}};
}

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    std::ostringstream& errorOss, sf::Packet& p,
    const SodiumReceiveKeyArray* keyReceive)
{
    if(!verifyReceivedPacketPreambleAndVersion(errorOss, p))
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    return decodeClientToServerPacketInner(errorOss, p, keyReceive);
}

// ----------------------------------------------------------------------------

template <typename T>
void makeServerToClientPacket(sf::Packet& p, const T& data)
{
    initializePacketForSending(p);
    encodeOHPacket(p, data);
}

template void makeServerToClientPacket(sf::Packet&, const STCPKick&);
template void makeServerToClientPacket(sf::Packet&, const STCPPublicKey&);

// ----------------------------------------------------------------------------

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    std::ostringstream& errorOss, sf::Packet& p)
{
    if(!verifyReceivedPacketPreambleAndVersion(errorOss, p))
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    const std::optional<PacketType> pt =
        decodeReceivedPacketAndGetPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == getPacketType<STCPKick>())
    {
        return {STCPKick{}};
    }

    if(*pt == getPacketType<STCPPublicKey>())
    {
        STCPPublicKey result;

        for(std::size_t i = 0; i < sodiumPublicKeyBytes; ++i)
        {
            if(p >> result.key[i])
            {
                continue;
            }

            errorOss << "Error decoding server public key packet at index '"
                     << i << "'\n";

            return {PInvalid{.error = errorOss.str()}};
        }

        return {result};
    }

    errorOss << "Unknown packet type '" << static_cast<int>(*pt) << "'\n";
    return {PInvalid{.error = errorOss.str()}};
}

} // namespace hg
