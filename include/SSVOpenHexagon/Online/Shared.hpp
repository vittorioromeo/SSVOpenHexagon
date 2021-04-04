// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <SFML/Network/Packet.hpp>

#include <sodium.h>

#include <cstdint>
#include <sstream>
#include <optional>
#include <variant>
#include <array>
#include <string>
#include <vector>

namespace hg
{

struct PInvalid
{
    std::string error;
};

struct PEncryptedMsg
{
    SodiumNonceArray nonce;
    std::size_t messageLength;
    std::size_t ciphertextLength;
    std::vector<unsigned char>* ciphertext;
};

// ----------------------------------------------------------------------------

// clang-format off
struct CTSPHeartbeat  { };
struct CTSPDisconnect { };
struct CTSPPublicKey  { SodiumPublicKeyArray key; };
struct CTSPReady      { };
struct CTSPPrint      { std::string msg; };
// clang-format on

using PVClientToServer = std::variant<PInvalid, PEncryptedMsg, CTSPHeartbeat,
    CTSPDisconnect, CTSPPublicKey, CTSPReady, CTSPPrint>;

// ----------------------------------------------------------------------------

template <typename T>
void makeClientToServerPacket(sf::Packet& p, const T& data);

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data);

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p);

// ----------------------------------------------------------------------------

// clang-format off
struct STCPKick      { };
struct STCPPublicKey { SodiumPublicKeyArray key; };
// clang-format on

using PVServerToClient =
    std::variant<PInvalid, PEncryptedMsg, STCPKick, STCPPublicKey>;

// ----------------------------------------------------------------------------

template <typename T>
void makeServerToClientPacket(sf::Packet& p, const T& data);

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p);

} // namespace hg
