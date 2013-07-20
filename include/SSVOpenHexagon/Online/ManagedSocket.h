#ifndef HG_ONLINE_MANAGEDSOCKET
#define HG_ONLINE_MANAGEDSOCKET

#include <chrono>
#include <thread>
#include <SSVUtils/SSVUtils.h>
#include <SFML/Network.hpp>
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
				Socket socket;
				bool busy{false};

				void update()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					if(!busy) return;

					sf::Packet packet;
					if(retry([&]{ return socket.receive(packet) == sf::Socket::Done; }).get())
					{
						onPacketReceived();
						packetHandler.handle(*this, packet);
					}
				}

				bool trySendPacket(Packet mPacket)
				{
					if(!busy) { ssvu::log("Couldn't send packet - not busy", "ManagedSocket"); return false; }

					if(retry([&]{ return socket.send(mPacket) == sf::Socket::Done; }).get()) { onPacketSent(); return true; }

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

				inline bool send(const Packet& mPacket) { return trySendPacket(mPacket); }
				inline bool connect(IpAddress mIp, unsigned int mPort)
				{
					if(busy) { ssvu::log("Error: already connected", "ManagedSocket"); return false; }
					if(!retry([&]{ return socket.connect(mIp, mPort) == Socket::Done; }).get()) return false;

					ssvu::log("Connected to " + mIp.toString() + ":" + ssvu::toStr(mPort), "ManagedSocket");
					busy = true; return true;
				}
				inline bool tryAccept(Listener& mListener)
				{
					if(busy) { ssvu::log("Error: already connected", "ManagedSocket"); return false; }
					if(!retry([&]{ return mListener.accept(socket) == Socket::Done; }).get()) return false;

					ssvu::log("Accepted", "ManagedSocket");
					busy = true; return true;
				}
				inline void disconnect()	{ socket.disconnect(); busy = false; }
				inline bool isBusy() const	{ return busy; }
		};
	}
}

#endif
