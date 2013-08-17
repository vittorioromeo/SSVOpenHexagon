// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE_UTILS
#define HG_ONLINE_UTILS

#include <future>
#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Online/Compression.h"

namespace hg
{
	namespace Online
	{
		enum class LogMode{Quiet, Verbose};

		namespace Internal
		{
			// Compression
			template<typename... TArgs> inline std::string buildCJsonString(TArgs&&... mArgs) { return getZLibCompress(ssvuj::getWriteToString(ssvuj::getArchArray(mArgs...))); }
		}

		// Build compressed packet
		template<unsigned int TType> inline sf::Packet buildCPacket() { sf::Packet result; result << TType; return result; }
		template<unsigned int TType, typename... TArgs> inline sf::Packet buildCPacket(TArgs&&... mArgs) { sf::Packet result{buildCPacket<TType>()}; result << Internal::buildCJsonString(mArgs...); return result; }

		// Decompress packet to obj
		inline ssvuj::Obj getDecompressedPacket(sf::Packet& mPacket) { std::string data; mPacket >> data; return ssvuj::readFromString(getZLibDecompress(data)); }
	}
}

#endif
