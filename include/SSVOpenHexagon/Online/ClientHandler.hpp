// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_CLIENTHANDLER
#define HG_ONLINE_CLIENTHANDLER

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/Utils.hpp"
#include "SSVOpenHexagon/Online/ManagedSocket.hpp"
#include "SSVOpenHexagon/Online/PacketHandler.hpp"

namespace hg
{
	namespace Online
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

				inline ClientHandler(PacketHandler<ClientHandler>& mPacketHandler) : packetHandler(mPacketHandler)
				{
					onPacketReceived += [this](sf::Packet mPacket){ packetHandler.handle(*this, mPacket); refreshTimeout(); };
					timeoutFuture = std::async(std::launch::async, [this]
					{
						while(running)
						{
							std::this_thread::sleep_for(800ms);

							if(!isBusy() || --untilTimeout > 0) continue;

							HG_LO_VERBOSE("ClientHandler") << "Client (" << uid << ") timed out\n";
							onDisconnect(); disconnect();
						}
					});
				}
				inline ~ClientHandler() { running = false; }

				inline void stop()					{ running = false; }
				inline unsigned int getUid() const	{ return uid; }
				inline void refreshTimeout()		{ untilTimeout = 5; }
		};
	}
}

#endif
