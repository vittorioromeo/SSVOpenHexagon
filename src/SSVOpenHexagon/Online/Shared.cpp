// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Shared.hpp"

#include "SSVOpenHexagon/Global/Version.hpp"

#include <SFML/Network/Packet.hpp>

#include <cstdint>
#include <sstream>
#include <optional>

namespace hg
{

namespace
{

enum class PacketType : std::uint8_t
{
    Heartbeat = 0
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

void makePacket(sf::Packet& p, const PHeartbeat& data)
{
    (void)data;
    initializePacketForSending(p, PacketType::Heartbeat);
}

[[nodiscard]] PacketVariant decodePacket(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const std::optional<PacketType> pt =
        decodeReceivedPacketAndGetPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == PacketType::Heartbeat)
    {
        return {PHeartbeat{}};
    }

    errorOss << "Unknown packet type '" << static_cast<std::uint8_t>(*pt)
             << "'\n";

    return {PInvalid{.error = errorOss.str()}};
}

} // namespace hg
