// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_MUSICDATA
#define HG_MUSICDATA

#include <SFML/Audio.hpp>
#include <string>
#include <vector>

namespace hg
{
	class HGAssets;

	class MusicData
	{
		private:
			std::vector<int> segments;

			inline int getRandomSegment() const;

		public:
			std::string id, fileName, name, album, author;
			bool firstPlay{true};

			MusicData() = default;
			MusicData(const std::string& mId, const std::string& mFileName, const std::string& mName, const std::string& mAlbum, const std::string& mAuthor);

			inline void addSegment(int mSeconds) { segments.push_back(mSeconds); }
			void playRandomSegment(HGAssets& mAssets);
			void playSegment(HGAssets& mAssets, int mSegmentIndex);
			void playSeconds(HGAssets& mAssets, int mSeconds);
	};
}

#endif
