#ifndef HG_ONLINE_CLIENTHANDLER
#define HG_ONLINE_CLIENTHANDLER

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <SSVUtils/SSVUtils.h>
#include <SFML/Network.hpp>
#include <unordered_map>
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/PacketHandler.h"
#include "SSVOpenHexagon/Online/Client.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		class Server;

		class ClientHandler
		{
			private:
				static unsigned int lastUid;
				ManagedSocket managedSocket;
				int untilTimeout{100};

			public:
				unsigned int uid;

				ClientHandler(PacketHandler& mPacketHandler) : managedSocket(mPacketHandler)
				{
					managedSocket.onPacketReceived += [&]{ untilTimeout = 100; };
					std::thread([&]
					{
						while(true)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(150));

							if(!isBusy()) continue;

							--untilTimeout;
							if(untilTimeout <= 0)
							{
								ssvu::log("Client timed out", "ClientHandler");
								managedSocket.disconnect();
							}
						}
					}).detach();
				}

				inline bool send(const sf::Packet& mPacket)			{ return managedSocket.send(mPacket); }
				inline bool tryAccept(sf::TcpListener& mListener)	{ return managedSocket.tryAccept(mListener); }
				inline bool isBusy() const							{ return managedSocket.isBusy(); }
		};
	}
}

#endif
