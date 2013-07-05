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
	class MusicData
	{
		private:
			std::vector<int> segments;
			std::string id, fileName, name, album, author;
			bool firstPlay{true};

			int getRandomSegment() const;

		public:
			MusicData() = default;
			MusicData(const std::string& mId, const std::string& mFileName, const std::string& mName, const std::string& mAlbum, const std::string& mAuthor);

			void addSegment(int mSeconds);
			void playRandomSegment();
			void playSegment(int mSegmentIndex);
			void playSeconds(int mSeconds);

			std::string getId() const;
			std::string getFileName() const;
			std::string getName() const;
			std::string getAlbum() const;
			std::string getAuthor() const;

			void setFirstPlay(bool mFirstPlay);
			bool getFirstPlay() const;
	};
}

#endif
