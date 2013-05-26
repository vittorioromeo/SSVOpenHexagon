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
			std::string id{""}, fileName{""}, name{""}, album{""}, author{""};
			sf::Music* musicPtr{nullptr};
			bool firstPlay{true};

			int getRandomSegment();

		public:
			MusicData() = default;
			MusicData(const std::string& mId, const std::string& mFileName, const std::string& mName, const std::string& mAlbum, const std::string& mAuthor);

			void addSegment(int mSeconds);
			void playRandomSegment();
			void playSegment(int mSegmentIndex);
			void playSeconds(int mSeconds);

			sf::Music* getMusic();

			std::string getId();
			std::string getFileName();
			std::string getName();
			std::string getAlbum();
			std::string getAuthor();

			void setFirstPlay(bool mFirstPlay);
			bool getFirstPlay();
	};
}

#endif
