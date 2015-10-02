// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_UTILS
#define HG_ONLINE_UTILS

#include <future>
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/Compression.hpp"

#define HG_LO_VERBOSE(...) \
    if(Config::getServerVerbose()) ssvu::lo(__VA_ARGS__)

namespace hg
{
namespace Online
{
    namespace Impl
    {
        // Compression
        template <typename... TArgs>
        inline std::string buildCJsonString(TArgs&&... mArgs)
        {
            const auto& packetStr(
            ssvuj::getWriteToString(ssvuj::getArchArray(FWD(mArgs)...)));
            return getZLibCompress(packetStr);
        }
    }

    // Build compressed packet
    template <unsigned int TType>
    inline sf::Packet buildCPacket()
    {
        sf::Packet result;
        result << TType;
        return result;
    }
    template <unsigned int TType, typename... TArgs>
    inline sf::Packet buildCPacket(TArgs&&... mArgs)
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
}
}

#endif
