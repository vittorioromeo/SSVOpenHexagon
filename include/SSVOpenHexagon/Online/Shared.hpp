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
struct PInvalid   { std::string error; };
struct PHeartbeat { };
// clang-format on

using PacketVariant = std::variant<PInvalid, PHeartbeat>;

void makePacket(sf::Packet& p, const PHeartbeat& data);

[[nodiscard]] PacketVariant decodePacket(
    std::ostringstream& errorOss, sf::Packet& p);

} // namespace hg
