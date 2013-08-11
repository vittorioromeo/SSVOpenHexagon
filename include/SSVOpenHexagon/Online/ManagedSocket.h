// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_MANAGEDSOCKET
#define HG_ONLINE_MANAGEDSOCKET

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Online/Utils.h"
#include "SSVOpenHexagon/Online/PacketHandler.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		class ManagedSocket
		{
			private:
				sf::TcpSocket socket;
				bool busy{false};
				std::vector<std::thread> threads;

				void update()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					if(!busy) return;

					sf::Packet packet;
					if(socket.receive(packet) == sf::Socket::Done) onPacketReceived(packet);
				}

				bool trySendPacket(sf::Packet mPacket)
				{
					if(!busy) { ssvu::lo << ssvu::lt("ManagedSocket") << "Couldn't send packet - not busy" << std::endl; return false; }

					if(socket.send(mPacket) == sf::Socket::Done) { onPacketSent(mPacket); return true; }

					ssvu::lo << ssvu::lt("ManagedSocket") << "Couldn't send packet - disconnecting" << std::endl;
					busy = false; return false;
				}

			public:
				ssvu::Delegate<void(sf::Packet)> onPacketSent, onPacketReceived;

				ManagedSocket()
				{
					socket.setBlocking(true);
					std::thread([&]{ while(true) update(); }).detach();
				}

				inline bool send(const sf::Packet& mPacket) { return trySendPacket(mPacket); }
				inline bool connect(sf::IpAddress mIp, unsigned int mPort)
				{
					if(busy) { ssvu::lo << ssvu::lt("ManagedSocket") << "Error: already connected" << std::endl; return false; }
					if(socket.connect(mIp, mPort) != sf::Socket::Done) return false;

					ssvu::lo << ssvu::lt("ManagedSocket") << "Connected to " << mIp.toString() << ":" << mPort << std::endl;
					busy = true; return true;
				}
				inline bool tryAccept(sf::TcpListener& mListener)
				{
					if(busy) { ssvu::lo << ssvu::lt("ManagedSocket") << "Error: already connected" << std::endl; return false; }
					if(mListener.accept(socket) != sf::Socket::Done) return false;

					ssvu::lo << ssvu::lt("ManagedSocket") << "Accepted" << std::endl;
					busy = true; return true;
				}
				inline void disconnect()	{ socket.disconnect(); busy = false; }
				inline bool isBusy() const	{ return busy; }
		};
	}
}

#endif
