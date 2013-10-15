// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_PACKETHANDLER
#define HG_ONLINE_PACKETHANDLER

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Online
	{
		template<typename T> class PacketHandler
		{
			private:
				using HandlerFunc = ssvu::Func<void(T&, sf::Packet&)>;
				std::unordered_map<unsigned int, HandlerFunc> functionHandlers;

			public:
				void handle(T& mCaller, sf::Packet& mPacket)
				{
					unsigned int type{0};

					try
					{
						mPacket >> type;

						auto itr(functionHandlers.find(type));
						if(itr == end(functionHandlers))
						{
							ssvu::lo("PacketHandler") << "Can't handle packet of type: " << type << std::endl;
							return;
						}

						itr->second(mCaller, mPacket);
					}
					catch(std::exception& mException)
					{
						ssvu::lo << "Exception during packet handling: (" << type << ")\n" << mException.what() << std::endl;
					}
					catch(...)
					{
						ssvu::lo << "Unknown exception during packet handling: (" << type << ")" << std::endl;
					}
				}

				HandlerFunc& operator[](unsigned int mType) { return functionHandlers[mType]; }
		};
	}
}

#endif
