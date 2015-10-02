// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_PACKETHANDLER
#define HG_ONLINE_PACKETHANDLER

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Online/Utils.hpp"

namespace hg
{
namespace Online
{
    template <typename T>
    class PacketHandler
    {
    private:
        using HandlerFunc = ssvu::Func<void(T&, sf::Packet&)>;
        std::unordered_map<unsigned int, HandlerFunc> funcs;

    public:
        inline void handle(T& mCaller, sf::Packet& mPacket)
        {
            unsigned int type{0};

            try
            {
                mPacket >> type;

                auto itr(funcs.find(type));
                if(itr == std::end(funcs)) {
                    HG_LO_VERBOSE("PacketHandler")
                    << "Can't handle packet of type: " << type << std::endl;
                    return;
                }

                itr->second(mCaller, mPacket);
            }
            catch(std::exception& mEx)
            {
                HG_LO_VERBOSE("PacketHandler")
                << "Exception during packet handling: (" << type << ")\n"
                << mEx.what() << std::endl;
            }
            catch(...)
            {
                HG_LO_VERBOSE("PacketHandler")
                << "Unknown exception during packet handling: (" << type
                << ")\n";
            }
        }

        HandlerFunc& operator[](unsigned int mType) { return funcs[mType]; }
    };
}
}

#endif
