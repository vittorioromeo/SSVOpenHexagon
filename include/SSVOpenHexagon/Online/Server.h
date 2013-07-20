#ifndef HG_ONLINE_SERVER
#define HG_ONLINE_SERVER

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
#include "SSVOpenHexagon/Online/ClientHandler.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		class Server
		{
			private:
				PacketHandler& packetHandler;
				sf::TcpListener listener;
				std::vector<Uptr<ClientHandler>> clientHandlers;
				std::thread receiveThread;

				void grow()
				{
					if(find_if(begin(clientHandlers), end(clientHandlers), [](const Uptr<ClientHandler>& mCH){ return !mCH->isBusy(); }) != end(clientHandlers)) return;

					ssvu::log("Creating new client handlers", "Server");
					for(int i{0}; i < 10; ++i) clientHandlers.emplace_back(new ClientHandler{packetHandler});
				}

				void update()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));

					grow();

					for(auto& c : clientHandlers)
					{
						if(c->isBusy()) continue;
						if(!asyncTry([&]{ return c->tryAccept(listener); }).get()) continue;

						ssvu::log("Accepted client (" + ssvu::toStr(c->uid) + ")", "Server");
					}
				}

			public:
				Server(PacketHandler& mPacketHandler) : packetHandler(mPacketHandler)
				{
					listener.setBlocking(false);
					receiveThread = std::thread([&]{ while(true) update(); });
				}

				void start(unsigned int mPort)
				{
					if(listener.listen(mPort) != sf::Socket::Done) { ssvu::log("Error initializing listener", "Server"); return; }
					else ssvu::log("Listener initialized", "Server");

					receiveThread.detach();
				}

		};
	}
}

#endif

