// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_CLIENT
#define HG_ONLINE_CLIENT

#include "SSVOpenHexagon/Online/ManagedSocket.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{
	namespace Online
	{
		class Client : public ManagedSocket
		{
			private:
				PacketHandler<Client>& packetHandler;

			public:
				Client(PacketHandler<Client>& mPacketHandler) : packetHandler(mPacketHandler)
				{
					onPacketReceived += [this](sf::Packet mPacket){ packetHandler.handle(*this, mPacket); };
				}
				~Client() { disconnect(); ssvu::lo() << "Client destroyed" << std::endl; }
		};
	}
}

#endif

