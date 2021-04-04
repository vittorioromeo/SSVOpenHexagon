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

namespace hg
{

namespace
{

enum class PacketType : std::uint8_t
{
    CTS_Heartbeat = 0,
    CTS_Disconnect = 1,
    CTS_PublicKey = 2,
    CTS_Ready = 3,
    CTS_EncryptedMsg = 4,

    STC_Kick = 128,
    STC_PublicKey = 129
};

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

void addPacketType(sf::Packet& p, const PacketType pt)
{
    p << static_cast<std::uint8_t>(pt);
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

[[nodiscard]] bool verifyReceivedPacket(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const auto matchByte = makeByteMatcher(errorOss, p);

    return matchByte("preamble 1st byte", preamble1stByte) &&
           matchByte("preamble 2nd byte", preamble2ndByte) &&
           matchByte("major version", GAME_VERSION.major) &&
           matchByte("minor version", GAME_VERSION.minor) &&
           matchByte("micro version", GAME_VERSION.micro);
}

void initializePacketForSending(sf::Packet& p, const PacketType pt)
{
    p.clear();

    addPreamble(p);
    addVersion(p);
    addPacketType(p, pt);
}

[[nodiscard]] std::optional<PacketType> decodeReceivedPacketAndGetPacketType(
    std::ostringstream& errorOss, sf::Packet& p)
{
    if(!verifyReceivedPacket(errorOss, p))
    {
        return std::nullopt;
    }

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

} // namespace

void makeClientToServerPacket(sf::Packet& p, const CTSPHeartbeat&)
{
    initializePacketForSending(p, PacketType::CTS_Heartbeat);
}

void makeClientToServerPacket(sf::Packet& p, const CTSPDisconnect&)
{
    initializePacketForSending(p, PacketType::CTS_Disconnect);
}

void makeClientToServerPacket(sf::Packet& p, const CTSPPublicKey& data)
{
    initializePacketForSending(p, PacketType::CTS_PublicKey);
    encodeArray<sodiumPublicKeyBytes>(p, data.key);
}

void makeClientToServerPacket(sf::Packet& p, const CTSPReady&)
{
    initializePacketForSending(p, PacketType::CTS_Ready);
}

[[nodiscard]] bool makeClientToServerPacket(sf::Packet& p,
    const SodiumTransmitKeyArray& keyTransmit, const CTSPEncryptedMsg& data)
{
    initializePacketForSending(p, PacketType::CTS_EncryptedMsg);

    const SodiumNonceArray nonce = generateNonce();

    const auto messageData =
        static_cast<const unsigned char*>(data.msg.getData());

    const std::size_t messageLength = data.msg.getDataSize();

    const std::size_t ciphertextLength = getCiphertextLength(messageLength);
    std::vector<unsigned char> ciphertext;
    ciphertext.resize(ciphertextLength);

    if(crypto_secretbox_easy(ciphertext.data(), messageData, messageLength,
           nonce.data(), keyTransmit.data()) != 0)
    {
        return false;
    }

    encodeArray<sodiumNonceBytes>(p, nonce);
    p << ciphertextLength;
    encodeFirstNVectorElements(p, ciphertext, ciphertextLength);
    p << messageLength;

    return true;
}

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    std::ostringstream& errorOss, sf::Packet& p,
    const SodiumReceiveKeyArray* keyReceive)
{
    const std::optional<PacketType> pt =
        decodeReceivedPacketAndGetPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == PacketType::CTS_Heartbeat)
    {
        return {CTSPHeartbeat{}};
    }

    if(*pt == PacketType::CTS_Disconnect)
    {
        return {CTSPDisconnect{}};
    }

    if(*pt == PacketType::CTS_PublicKey)
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

    if(*pt == PacketType::CTS_Ready)
    {
        return {CTSPReady{}};
    }

    if(*pt == PacketType::CTS_EncryptedMsg)
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

        std::size_t messageLength;
        if(!(p >> messageLength))
        {
            errorOss << "Error decoding client message length\n";
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

        CTSPEncryptedMsg result;
        result.msg.append(message.data(), messageLength);
        return {result};
    }

    errorOss << "Unknown packet type '" << static_cast<int>(*pt) << "'\n";
    return {PInvalid{.error = errorOss.str()}};
}

// ----------------------------------------------------------------------------

void makeServerToClientPacket(sf::Packet& p, const STCPKick&)
{
    initializePacketForSending(p, PacketType::STC_Kick);
}

void makeServerToClientPacket(sf::Packet& p, const STCPPublicKey& data)
{
    initializePacketForSending(p, PacketType::STC_PublicKey);

    for(std::size_t i = 0; i < sodiumPublicKeyBytes; ++i)
    {
        p << data.key[i];
    }
}

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const std::optional<PacketType> pt =
        decodeReceivedPacketAndGetPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == PacketType::STC_Kick)
    {
        return {STCPKick{}};
    }

    if(*pt == PacketType::STC_PublicKey)
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
