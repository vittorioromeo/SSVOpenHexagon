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

namespace hg
{

// clang-format off
struct PInvalid { std::string error; };
// clang-format on

// ----------------------------------------------------------------------------

// clang-format off
struct CTSPHeartbeat    { };
struct CTSPDisconnect   { };
struct CTSPPublicKey    { SodiumPublicKeyArray key; };
struct CTSPReady        { };
struct CTSPEncryptedMsg { std::string msg; };
// clang-format on

using PVClientToServer = std::variant<PInvalid, CTSPHeartbeat, CTSPDisconnect,
    CTSPPublicKey, CTSPReady, CTSPEncryptedMsg>;

void makeClientToServerPacket(sf::Packet& p, const CTSPHeartbeat& data);
void makeClientToServerPacket(sf::Packet& p, const CTSPDisconnect& data);
void makeClientToServerPacket(sf::Packet& p, const CTSPPublicKey& data);
void makeClientToServerPacket(sf::Packet& p, const CTSPReady& data);
[[nodiscard]] bool makeClientToServerPacket(sf::Packet& p,
    const SodiumTransmitKeyArray& keyTransmit, const CTSPEncryptedMsg& data);

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    std::ostringstream& errorOss, sf::Packet& p,
    const SodiumReceiveKeyArray* keyReceive);

// ----------------------------------------------------------------------------

// clang-format off
struct STCPKick      { };
struct STCPPublicKey { SodiumPublicKeyArray key; };
// clang-format on

using PVServerToClient = std::variant<PInvalid, STCPKick, STCPPublicKey>;

void makeServerToClientPacket(sf::Packet& p, const STCPKick& data);
void makeServerToClientPacket(sf::Packet& p, const STCPPublicKey& data);

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    std::ostringstream& errorOss, sf::Packet& p);

} // namespace hg
