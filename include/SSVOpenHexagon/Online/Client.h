// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_CLIENT
#define HG_ONLINE_CLIENT

#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/ManagedSocket.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		class Client
		{
			private:
				ManagedSocket managedSocket;

			public:
				Client(PacketHandler& mPacketHandler) : managedSocket(mPacketHandler) { }

				inline bool connect(IpAddress mIp, unsigned int mPort)	{ return managedSocket.connect(mIp, mPort); }
				inline bool send(const Packet& mPacket)					{ return managedSocket.send(mPacket); }
				inline void disconnect()								{ managedSocket.disconnect(); }
				inline ManagedSocket& getManagedSocket()				{ return managedSocket; }
		};
	}
}

#endif

