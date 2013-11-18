// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_MANAGEDSOCKET
#define HG_ONLINE_MANAGEDSOCKET

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Online/Utils.hpp"
#include "SSVOpenHexagon/Online/PacketHandler.hpp"

namespace hg
{
	namespace Online
	{
		class ManagedSocket
		{
			private:
				sf::TcpSocket socket;
				bool busy{false};

				std::future<void> handlerFuture;

				void update()
				{
					if(!busy) { ssvu::lo("ManagedSocket") << "Update failed - not busy" << std::endl; return; }

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					sf::Packet packet;

					for(int i{0}; i < 5; ++i)
					{
						if(busy && socket.receive(packet) == sf::Socket::Done) onPacketReceived(packet);
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}
				}

				bool trySendPacket(sf::Packet mPacket)
				{
					if(!busy) { ssvu::lo("ManagedSocket") << "Couldn't send packet - not busy" << std::endl; return false; }

					for(int i{0}; i < 5; ++i)
					{
						if(busy && socket.send(mPacket) == sf::Socket::Done) { onPacketSent(mPacket); return true; }
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}

					ssvu::lo("ManagedSocket") << "Couldn't send packet - disconnecting" << std::endl;
					disconnect();
					//handlerThread.join();
					return false;
				}

			public:
				ssvu::Delegate<void(sf::Packet)> onPacketSent, onPacketReceived;

				ManagedSocket() { socket.setBlocking(false); }
				~ManagedSocket() { disconnect(); ssvu::lo << "ManagedSocket destroyed" << std::endl; }

				inline bool send(const sf::Packet& mPacket) { return trySendPacket(mPacket); }
				inline bool connect(sf::IpAddress mIp, unsigned int mPort)
				{
					if(busy) { ssvu::lo("ManagedSocket") << "Error: already connected" << std::endl; return false; }

					for(int i{0}; i < 5; ++i)
					{
						if(!busy && socket.connect(mIp, mPort) == sf::Socket::Done) goto succeed;
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}

					return false;

					succeed:
					if(!busy)
					{
						ssvu::lo("ManagedSocket") << "Connecting..." << std::endl;
						busy = true;
						handlerFuture = std::async(std::launch::async, [this]{ while(busy) update(); });
					}
					ssvu::lo("ManagedSocket") << "Connected to " << mIp.toString() << ":" << mPort << std::endl;
					return true;
				}
				inline bool tryAccept(sf::TcpListener& mListener)
				{
					if(busy) { ssvu::lo("ManagedSocket") << "Error: already connected" << std::endl; return false; }

					for(int i{0}; i < 5; ++i)
					{
						if(!busy && mListener.accept(socket) == sf::Socket::Done) goto succeed;
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}

					return false;

					succeed:
					if(!busy)
					{
						ssvu::lo("ManagedSocket") << "Accepting..." << std::endl;
						busy = true;
						handlerFuture = std::async(std::launch::async, [this]{ while(busy) update(); });
					}
					ssvu::lo("ManagedSocket") << "Accepted" << std::endl;
					return true;
				}
				inline void disconnect()	{ socket.disconnect(); busy = false; }
				inline bool isBusy() const	{ return busy; }
		};
	}
}

#endif
