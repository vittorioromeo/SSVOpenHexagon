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
			// Array-to-compress building
			template<unsigned int TIndex, typename TArg> inline void jBuildHelper(ssvuj::Obj& mP, TArg&& mArg) { ssvuj::set(mP, TIndex, mArg); }
			template<unsigned int TIndex, typename TArg, typename... TArgs> inline void jBuildHelper(ssvuj::Obj& mP, TArg&& mArg, TArgs&&... mArgs) { ssvuj::set(mP, TIndex, mArg); jBuildHelper<TIndex + 1>(mP, mArgs...); }

			// Compression
			template<typename... TArgs> inline ssvuj::Obj buildJsonArray(TArgs&&... mArgs) { ssvuj::Obj result; Internal::jBuildHelper<0>(result, mArgs...); return result; }
			template<typename... TArgs> inline std::string buildCJsonString(TArgs&&... mArgs) { return getZLibCompress(ssvuj::getWriteToString(buildJsonArray(mArgs...))); }

			// Decompression
			inline ssvuj::Obj getDecompressedJsonString(const std::string& mData) { return ssvuj::readFromString(getZLibDecompress(mData)); }
		}

		// Build compressed packet
		template<unsigned int TType> inline sf::Packet buildCPacket() { sf::Packet result; result << TType; return result; }
		template<unsigned int TType, typename... TArgs> inline sf::Packet buildCPacket(TArgs&&... mArgs) { sf::Packet result{buildCPacket<TType>()}; result << Internal::buildCJsonString(mArgs...); return result; }

		// Decompress packet to ssvuj value
		inline ssvuj::Obj getDecompressedPacket(sf::Packet& mPacket) { std::string data; mPacket >> data; return Internal::getDecompressedJsonString(data); }
	}
}

#endif
