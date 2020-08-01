// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/Compression.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <SFML/Network.hpp>

#define HG_LO_VERBOSE(...) \
    if(::hg::Config::getServerVerbose()) ::ssvu::lo(__VA_ARGS__)

namespace hg::Online
{

namespace Impl
{

// Compression
template <typename... TArgs>
std::string buildCJsonString(TArgs&&... mArgs)
{
    const auto& packetStr(
        ssvuj::getWriteToString(ssvuj::getArchArray(FWD(mArgs)...)));
    return getZLibCompress(packetStr);
}

} // namespace Impl

// Build compressed packet
template <unsigned int TType>
sf::Packet buildCPacket()
{
    sf::Packet result;
    result << TType;
    return result;
}
template <unsigned int TType, typename... TArgs>
sf::Packet buildCPacket(TArgs&&... mArgs)
{
    sf::Packet result{buildCPacket<TType>()};
    result << Impl::buildCJsonString(mArgs...);
    return result;
}

// Decompress packet to obj
inline ssvuj::Obj getDecompressedPacket(sf::Packet& mPacket)
{
    std::string data;
    mPacket >> data;
    return ssvuj::getFromStr(getZLibDecompress(data));
}

} // namespace hg::Online
