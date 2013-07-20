#ifndef HG_ONLINE_PACKETHANDLER
#define HG_ONLINE_PACKETHANDLER

#include <SSVUtils/SSVUtils.h>
#include <SFML/Network.hpp>
#include <unordered_map>
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		class ManagedSocket;

		struct PacketHandler
		{
			using HandlerFunc = std::function<void(ManagedSocket&, Packet&)>;

			std::unordered_map<unsigned int, HandlerFunc> functionHandlers;

			void handle(ManagedSocket& mManagedSocket, Packet& mPacket)
			{
				unsigned int type;
				mPacket >> type;

				auto itr(functionHandlers.find(type));
				if(itr == end(functionHandlers))
				{
					ssvu::log("Can't handle packet of type: " + ssvu::toStr(type), "PacketHandler");
					return;
				}

				itr->second(mManagedSocket, mPacket);
			}

			HandlerFunc& operator[](unsigned int mType) { return functionHandlers[mType]; }
		};
	}
}

#endif
