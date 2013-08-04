// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_SERVER
#define HG_ONLINE_SERVER

#include <algorithm>
#include <vector>
#include <chrono>
#include <thread>
#include <SSVUtils/SSVUtils.h>
#include <SFML/Network.hpp>
#include "SSVOpenHexagon/Online/ClientHandler.h"
#include "SSVOpenHexagon/Global/Typedefs.h"
#include "SSVOpenHexagon/Online/Utils.h"

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

				void growIfNeeded()
				{
					if(find_if(begin(clientHandlers), end(clientHandlers), [](const Uptr<ClientHandler>& mCH){ return !mCH->isBusy(); }) != end(clientHandlers)) return;

					ssvu::lo << ssvu::lt("Server") << "Creating new client handlers" << std::endl;
					for(int i{0}; i < 10; ++i) clientHandlers.emplace_back(new ClientHandler{packetHandler});
				}

				void update()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));

					growIfNeeded();

					for(auto& c : clientHandlers)
					{
						if(c->isBusy()) continue;
						if(!retry([&]{ return c->tryAccept(listener); }).get()) continue;

						onClientAccepted(*c.get());
						c->refreshTimeout();
						ssvu::lo << ssvu::lt("Server") << "Accepted client (" << c->getUid() << ")" << std::endl;
					}
				}

			public:
				ssvu::Delegate<void(ClientHandler&)> onClientAccepted;

				Server(PacketHandler& mPacketHandler) : packetHandler(mPacketHandler)
				{
					listener.setBlocking(false);
					receiveThread = std::thread([&]{ while(true) update(); });
				}

				void start(unsigned int mPort)
				{
					if(listener.listen(mPort) != sf::Socket::Done) { ssvu::lo << ssvu::lt("Server") << "Error initalizing listener" << std::endl; return; }
					else ssvu::lo << ssvu::lt("Server") << "Listener initialized" << std::endl;

					receiveThread.detach();
				}

		};
	}
}

#endif

