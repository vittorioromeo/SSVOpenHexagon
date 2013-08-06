// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_MUSICDATA
#define HG_MUSICDATA

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Global/Assets.h"

namespace hg
{
	class HGAssets;

	class MusicData
	{
		private:
			std::vector<int> segments;

			inline int getRandomSegment() const { return segments[ssvu::getRnd(0, segments.size())]; }

		public:
			std::string id, fileName, name, album, author;
			bool firstPlay{true};

			MusicData() = default;
			MusicData(const std::string& mId, const std::string& mFileName, const std::string& mName, const std::string& mAlbum, const std::string& mAuthor)
				: id{mId}, fileName{mFileName}, name{mName}, album{mAlbum}, author{mAuthor} { }

			inline void addSegment(int mSeconds) { segments.push_back(mSeconds); }
			inline void playRandomSegment(HGAssets& mAssets) { if(firstPlay) { firstPlay = false; playSegment(mAssets, 0); } else playSeconds(mAssets, getRandomSegment());}
			inline void playSegment(HGAssets& mAssets, int mSegmentIndex) { playSeconds(mAssets, segments[mSegmentIndex]); }
			inline void playSeconds(HGAssets& mAssets, int mSeconds) { if(Config::getNoMusic()) return; mAssets.playMusic(id, sf::seconds(mSeconds)); }
	};
}

#endif
