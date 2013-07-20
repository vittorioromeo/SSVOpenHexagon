#ifndef HG_ONLINE_CLIENT
#define HG_ONLINE_CLIENT

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
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		class ManagedSocket
		{
			private:
				PacketHandler& packetHandler;
				sf::TcpSocket socket;
				bool busy{false};

				void update()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					if(!busy) return;

					sf::Packet packet;
					if(asyncTry([&]{ return socket.receive(packet) == sf::Socket::Done; }).get())
					{
						onPacketReceived();
						packetHandler.handle(*this, packet);
					}
				}

				bool trySendPacket(sf::Packet mPacket)
				{
					if(!busy) { ssvu::log("Couldn't send packet - not busy", "ManagedSocket"); return false; }

					if(asyncTry([&]{ return socket.send(mPacket) == sf::Socket::Done; }).get())
					{
						onPacketSent();
						return true;
					}

					ssvu::log("Couldn't send packet - disconnecting", "ManagedSocket");
					busy = false; return false;
				}

			public:
				ssvu::Delegate<void> onPacketSent;
				ssvu::Delegate<void> onPacketReceived;

				ManagedSocket(PacketHandler& mPacketHandler) : packetHandler(mPacketHandler)
				{
					socket.setBlocking(false);
					std::thread([&]{ while(true) update(); }).detach();
				}

				inline bool send(const sf::Packet& mPacket) { return trySendPacket(mPacket); }
				inline bool connect(sf::IpAddress mIp, unsigned int mPort)
				{
					if(busy) { ssvu::log("Error: already connected", "ManagedSocket"); return false; }
					if(!asyncTry([&]{ return socket.connect(mIp, mPort) == sf::Socket::Done; }).get()) return false;

					ssvu::log("Connected to " + mIp.toString() + ":" + ssvu::toStr(mPort), "ManagedSocket");
					busy = true; return true;
				}
				inline bool tryAccept(sf::TcpListener& mListener)
				{
					if(busy) { ssvu::log("Error: already connected", "ManagedSocket"); return false; }
					if(!asyncTry([&]{ return mListener.accept(socket) == sf::Socket::Status::Done; }).get()) return false;

					ssvu::log("Accepted", "ManagedSocket");
					busy = true; return true;
				}
				inline void disconnect()	{ socket.disconnect(); busy = false; }
				inline bool isBusy() const	{ return busy; }
		};

		class Client
		{
			private:
				ManagedSocket managedSocket;

			public:
				Client(PacketHandler& mPacketHandler) : managedSocket(mPacketHandler) { }

				inline bool connect(sf::IpAddress mIp, unsigned int mPort)	{ return managedSocket.connect(mIp, mPort); }
				inline bool send(const sf::Packet& mPacket)					{ return managedSocket.send(mPacket); }
				inline void disconnect()									{ managedSocket.disconnect(); }
		};
	}
}

#endif

