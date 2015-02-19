// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_SERVER
#define HG_ONLINE_SERVER

#include <vector>
#include <chrono>
#include <thread>
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/ClientHandler.hpp"
#include "SSVOpenHexagon/Online/Utils.hpp"

namespace hg
{
	namespace Online
	{
		class Server
		{
			private:
				bool running{false};
				PacketHandler<ClientHandler>& packetHandler;
				sf::TcpListener listener;
				ssvu::VecUPtr<ClientHandler> clientHandlers;
				std::future<void> updateFuture;

				inline void updateImpl()
				{
					while(running)
					{
						std::this_thread::sleep_for(50ms);
						update();
					}
				}

				inline void update()
				{
					bool foundNonBusy{false};

					for(auto i(0u); true; ++i)
					{
						if(i >= clientHandlers.size())
						{
							if(foundNonBusy) return;

							HG_LO_VERBOSE("Server") << "Creating new client handlers\n";
							for(auto k(0u); k < 10; ++k) ssvu::getEmplaceUPtr<ClientHandler>(clientHandlers, packetHandler);
						}

						auto& c(clientHandlers[i]);

						if(c->isBusy()) continue;
						foundNonBusy = true;

						if(!c->tryAccept(listener)) continue;
						onClientAccepted(*c.get());
						c->refreshTimeout();
						HG_LO_VERBOSE("Server") << "Accepted client (" << c->getUid() << ")\n";
						return;
					}
				}

			public:
				ssvu::Delegate<void(ClientHandler&)> onClientAccepted;

				inline Server(PacketHandler<ClientHandler>& mPacketHandler) : packetHandler(mPacketHandler) { listener.setBlocking(false); }
				inline ~Server() { running = false; }

				inline void start(unsigned short mPort)
				{
					if(listener.listen(mPort) != sf::Socket::Done) { ssvu::lo("Server") << "Error initalizing listener\n"; return; }
					else ssvu::lo("Server") << "Listener initialized\n";

					running = true;
					updateFuture = std::async(std::launch::async, [this]{ updateImpl(); });
				}
				inline void stop()
				{
					for(auto& ch : clientHandlers) ch->stop();
					running = false; listener.close();
				}
				inline bool isRunning() const { return running; }
		};
	}
}

#endif

