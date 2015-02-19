// Copyright (c) 2013-2015 Vittorio Romeo
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

				inline void updateImpl()
				{
					while(busy)
					{
						update();
						std::this_thread::sleep_for(50ms);
					}
				}

				void update()
				{
					if(!busy)
					{
						HG_LO_VERBOSE("ManagedSocket") << "Update failed - not busy\n";
						return;
					}

					sf::Packet packet;

					for(int i{0}; i < 5; ++i)
					{
						if(busy && socket.receive(packet) == sf::Socket::Done) onPacketReceived(packet);
						std::this_thread::sleep_for(50ms);
					}
				}

				bool trySendPacket(sf::Packet mPacket)
				{
					if(!busy)
					{
						HG_LO_VERBOSE("ManagedSocket") << "Couldn't send packet - not busy\n";
						return false;
					}

					for(int i{0}; i < 5; ++i)
					{
						if(busy && socket.send(mPacket) == sf::Socket::Done) { onPacketSent(mPacket); return true; }
						std::this_thread::sleep_for(50ms);
					}

					HG_LO_VERBOSE("ManagedSocket") << "Couldn't send packet - disconnecting\n";
					disconnect();
					return false;
				}

			public:
				ssvu::Delegate<void(sf::Packet)> onPacketSent, onPacketReceived;

				inline ManagedSocket() { socket.setBlocking(false); }
				inline ~ManagedSocket() { disconnect(); }

				inline bool send(const sf::Packet& mPacket) { return trySendPacket(mPacket); }
				inline bool connect(sf::IpAddress mIp, unsigned short mPort)
				{
					if(busy)
					{
						HG_LO_VERBOSE("ManagedSocket") << "Error: already connected\n";
						return false;
					}

					for(int i{0}; i < 5; ++i)
					{
						if(!busy && socket.connect(mIp, mPort) == sf::Socket::Done) goto succeed;
						std::this_thread::sleep_for(60ms);
					}

					return false;

					succeed:
					if(!busy)
					{
						HG_LO_VERBOSE("ManagedSocket") << "Connecting...\n";
						busy = true;
						handlerFuture = std::async(std::launch::async, [this]{ updateImpl(); });
					}
					HG_LO_VERBOSE("ManagedSocket") << "Connected to " << mIp.toString() << ":" << mPort << "\n";
					return true;
				}
				inline bool tryAccept(sf::TcpListener& mListener)
				{
					if(busy)
					{
						HG_LO_VERBOSE("ManagedSocket") << "Error: already connected\n";
						return false;
					}

					for(int i{0}; i < 5; ++i)
					{
						if(!busy && mListener.accept(socket) == sf::Socket::Done) goto succeed;
						std::this_thread::sleep_for(60ms);
					}

					return false;

					succeed:
					if(!busy)
					{
						HG_LO_VERBOSE("ManagedSocket") << "Accepting...\n";
						busy = true;
						handlerFuture = std::async(std::launch::async, [this]{ updateImpl(); });
					}
					HG_LO_VERBOSE("ManagedSocket") << "Accepted\n";
					return true;
				}
				inline void disconnect()	{ socket.disconnect(); busy = false; }
				inline bool isBusy() const	{ return busy; }
		};
	}
}

#endif
