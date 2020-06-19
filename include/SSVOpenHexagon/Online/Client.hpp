// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/Utils.hpp"
#include "SSVOpenHexagon/Online/ManagedSocket.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg::Online
{

class Client : public ManagedSocket
{
private:
    PacketHandler<Client>& packetHandler;

public:
    Client(PacketHandler<Client>& mPacketHandler)
        : packetHandler(mPacketHandler)
    {
        onPacketReceived += [this](sf::Packet mPacket) {
            packetHandler.handle(*this, mPacket);
        };
    }
};

} // namespace hg::Online
