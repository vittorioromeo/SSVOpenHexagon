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
			template<unsigned int TIndex, typename TArg> inline void jBuildHelper(ssvuj::Value& mP, TArg&& mArg) { ssvuj::set(mP, TIndex, mArg); }
			template<unsigned int TIndex, typename TArg, typename... TArgs> inline void jBuildHelper(ssvuj::Value& mP, TArg&& mArg, TArgs&&... mArgs) { ssvuj::set(mP, TIndex, mArg); jBuildHelper<TIndex + 1>(mP, mArgs...); }

			// Compression
			template<typename... TArgs> inline ssvuj::Value buildJsonArray(TArgs&&... mArgs) { ssvuj::Value result; Internal::jBuildHelper<0>(result, mArgs...); return result; }
			template<typename... TArgs> inline std::string buildCJsonString(TArgs&&... mArgs) { return getZLibCompress(ssvuj::getWriteRootToString(buildJsonArray(mArgs...))); }

			// Decompression
			inline ssvuj::Value getDecompressedJsonString(const std::string& mData) { return ssvuj::getRootFromString(getZLibDecompress(mData)); }
		}

		// Build compressed packet
		template<unsigned int TType> inline sf::Packet buildCPacket() { sf::Packet result; result << TType; return result; }
		template<unsigned int TType, typename... TArgs> inline sf::Packet buildCPacket(TArgs&&... mArgs) { sf::Packet result{buildCPacket<TType>()}; result << Internal::buildCJsonString(mArgs...); return result; }

		// Decompress packet to ssvuj value
		inline ssvuj::Value getDecompressedPacket(sf::Packet& mPacket) { std::string data; mPacket >> data; return Internal::getDecompressedJsonString(data); }

		template<int TTimes = 5, LogMode TLM = LogMode::Quiet> inline std::future<bool> retry(std::function<bool()> mFunc, const std::chrono::duration<int, std::milli>& mDuration = std::chrono::milliseconds(1500))
		{
			auto result(std::async(std::launch::async, [=]
			{
				for(int i{0}; i < TTimes; ++i)
				{
					if(mFunc()) return true;

					if(TLM == LogMode::Verbose) ssvu::lo << ssvu::lt("asyncTry") << "Error - retrying (" << i + 1 << "/" << TTimes << ")" << std::endl;
					std::this_thread::sleep_for(mDuration);
				}

				return false;
			}));

			return result;
		}
	}
}

#endif
