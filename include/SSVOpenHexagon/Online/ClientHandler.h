// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_CLIENTHANDLER
#define HG_ONLINE_CLIENTHANDLER

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Online/ManagedSocket.h"
#include "SSVOpenHexagon/Online/PacketHandler.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

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

			public:
				ssvu::Delegate<void()> onDisconnect;

				ClientHandler(PacketHandler<ClientHandler>& mPacketHandler) : packetHandler(mPacketHandler)
				{
					onPacketReceived += [&](sf::Packet mPacket){ packetHandler.handle(*this, mPacket); refreshTimeout(); };

					std::thread([&]
					{
						while(running)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(800));
							if(!isBusy() || --untilTimeout > 0) continue;

							ssvu::lo << ssvu::lt("ClientHandler") << "Client (" << uid << ") timed out" << std::endl;
							onDisconnect(); disconnect();
						}
					}).detach();
				}

				inline void stop() { running = false; }
				inline unsigned int getUid() const { return uid; }
				inline void refreshTimeout() { untilTimeout = 5; }
		};
	}
}

#endif
