#ifndef HG_ONLINE_CLIENTHANDLER
#define HG_ONLINE_CLIENTHANDLER

#include <chrono>
#include <thread>
#include <SSVUtils/SSVUtils.h>
#include <SFML/Network.hpp>
#include "SSVOpenHexagon/Online/ManagedSocket.h"
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
				unsigned int uid;
				ManagedSocket managedSocket;
				int untilTimeout{100};

			public:

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
							if(untilTimeout > 0) continue;

							ssvu::lo << ssvu::lt("ClientHandler") << "Client timed out" << std::endl;
							managedSocket.disconnect();
						}
					}).detach();
				}

				inline bool send(const Packet& mPacket)		{ return managedSocket.send(mPacket); }
				inline bool tryAccept(Listener& mListener)	{ return managedSocket.tryAccept(mListener); }
				inline bool isBusy() const					{ return managedSocket.isBusy(); }
				inline unsigned int getUid() const			{ return uid; }
				inline ManagedSocket& getManagedSocket()	{ return managedSocket; }
		};
	}
}

#endif
