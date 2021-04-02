// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Network/Packet.hpp>

#include <cstdint>
#include <sstream>
#include <optional>
#include <variant>
#include <string>

namespace hg
{

// clang-format off
struct PInvalid { std::string error; };
// clang-format on

// ----------------------------------------------------------------------------

// clang-format off
struct CTSPHeartbeat  { };
struct CTSPDisconnect { };
// clang-format on

using PVClientToServer = std::variant<PInvalid, CTSPHeartbeat, CTSPDisconnect>;

void makeClientToServerPacket(sf::Packet& p, const CTSPHeartbeat& data);
void makeClientToServerPacket(sf::Packet& p, const CTSPDisconnect& data);

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    std::ostringstream& errorOss, sf::Packet& p);

// ----------------------------------------------------------------------------

// clang-format off
struct STCPKick { };
// clang-format on

using PVServerToClient = std::variant<PInvalid, STCPKick>;

void makeServerToClientPacket(sf::Packet& p, const STCPKick& data);

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    std::ostringstream& errorOss, sf::Packet& p);

} // namespace hg
