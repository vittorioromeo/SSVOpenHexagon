// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_PACKETHANDLER
#define HG_ONLINE_PACKETHANDLER

#include <SSVUtils/SSVUtils.h>
#include <SFML/Network.hpp>
#include <unordered_map>
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		class ManagedSocket;

		struct PacketHandler
		{
			using HandlerFunc = std::function<void(ManagedSocket&, sf::Packet&)>;

			std::unordered_map<unsigned int, HandlerFunc> functionHandlers;

			void handle(ManagedSocket& mManagedSocket, sf::Packet& mPacket)
			{
				unsigned int type;
				mPacket >> type;

				auto itr(functionHandlers.find(type));
				if(itr == end(functionHandlers))
				{
					ssvu::lo << ssvu::lt("PacketHandler") << "Can't handle packet of type: " << type << std::endl;
					return;
				}

				itr->second(mManagedSocket, mPacket);
			}

			HandlerFunc& operator[](unsigned int mType) { return functionHandlers[mType]; }
		};
	}
}

#endif
