// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/Utils.hpp"
#include "SSVOpenHexagon/Online/ManagedSocket.hpp"
#include "SSVOpenHexagon/Online/PacketHandler.hpp"

namespace hg::Online
{

class Server;

class ClientHandler : public ManagedSocket
{
private:
    static unsigned int lastUid;
    bool running{true};
    unsigned int uid{lastUid++}, untilTimeout{5};
    PacketHandler<ClientHandler> packetHandler;
    std::future<void> timeoutFuture;

public:
    ssvu::Delegate<void()> onDisconnect;

    ClientHandler(PacketHandler<ClientHandler>& mPacketHandler)
        : packetHandler(mPacketHandler)
    {
        onPacketReceived += [this](sf::Packet mPacket) {
            packetHandler.handle(*this, mPacket);
            refreshTimeout();
        };
        timeoutFuture = std::async(std::launch::async, [this] {
            while(running)
            {
                std::this_thread::sleep_for(800ms);

                if(!isBusy() || --untilTimeout > 0)
                {
                    continue;
                }

                HG_LO_VERBOSE("ClientHandler")
                    << "Client (" << uid << ") timed out\n";
                onDisconnect();
                disconnect();
            }
        });
    }
    ~ClientHandler()
    {
        running = false;
    }

    void stop()
    {
        running = false;
    }
    unsigned int getUid() const
    {
        return uid;
    }
    void refreshTimeout()
    {
        untilTimeout = 5;
    }
};

} // namespace hg::Online
