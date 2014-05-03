// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_SERVER
#define HG_ONLINE_SERVER

#include <vector>
#include <chrono>
#include <thread>
#include "SSVOpenHexagon/Global/Common.hpp"
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
				std::vector<Uptr<ClientHandler>> clientHandlers;
				std::future<void> updateFuture;

				inline void growIfNeeded()
				{
					if(ssvu::containsAnyIf(clientHandlers, [](const Uptr<ClientHandler>& mCH){ return !mCH->isBusy(); })) return;

					ssvu::lo("Server") << "Creating new client handlers" << std::endl;
					for(int i{0}; i < 10; ++i) ssvu::getEmplaceUptr<ClientHandler>(clientHandlers, packetHandler);
				}

				inline void updateImpl()
				{
					while(running)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
						update();
					}
				}

				inline void update()
				{
					growIfNeeded();

					for(auto& c : clientHandlers)
					{
						if(c->isBusy() || !c->tryAccept(listener)) continue;

						onClientAccepted(*c.get());
						c->refreshTimeout();
						ssvu::lo("Server") << "Accepted client (" << c->getUid() << ")" << std::endl;
					}
				}

			public:
				ssvu::Delegate<void(ClientHandler&)> onClientAccepted;

				Server(PacketHandler<ClientHandler>& mPacketHandler) : packetHandler(mPacketHandler) { listener.setBlocking(false); }
				~Server() { running = false; ssvu::lo() << "Server destroyed" << std::endl; }

				inline void start(unsigned short mPort)
				{
					if(listener.listen(mPort) != sf::Socket::Done) { ssvu::lo("Server") << "Error initalizing listener" << std::endl; return; }
					else ssvu::lo("Server") << "Listener initialized" << std::endl;

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

