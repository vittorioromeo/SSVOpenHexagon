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

    for(std::size_t i = 0; i < sodiumPublicKeyBytes; ++i)
    {
        p << data.key[i];
    }
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

    std::vector<unsigned char> message;
    message.reserve(data.msg.size());
    for(const char c : data.msg)
    {
        message.emplace_back(static_cast<unsigned char>(c));
    }

    const std::size_t messageLength = message.size();

    const std::size_t ciphertextLength = getCiphertextLength(messageLength);
    std::vector<unsigned char> ciphertext;
    ciphertext.resize(ciphertextLength);

    if(crypto_secretbox_easy(ciphertext.data(), message.data(), messageLength,
           nonce.data(), keyTransmit.data()) != 0)
    {
        return false;
    }

    for(std::size_t i = 0; i < sodiumNonceBytes; ++i)
    {
        p << nonce[i];
    }

    p << ciphertextLength;

    for(std::size_t i = 0; i < ciphertextLength; ++i)
    {
        p << ciphertext[i];
    }

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

        std::string s;
        s.reserve(messageLength);
        for(const unsigned char c : message)
        {
            s += static_cast<char>(c);
        }

        return {CTSPEncryptedMsg{s}};
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
