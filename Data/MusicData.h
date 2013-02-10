// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef MUSICDATA_H_
#define MUSICDATA_H_

#include <SFML/Audio.hpp>
#include <string>
#include <vector>

namespace hg
{
	class MusicData
	{
		private:
			std::vector<int> segments;
			std::string id 			{""};
			std::string fileName 	{""};
			std::string name		{""};
			std::string album		{""};
			std::string author		{""};

			sf::Music* musicPtr		{new sf::Music};
			bool firstPlay		{true};
			int getRandomSegment();

		public:
			MusicData() = default;
			MusicData(std::string mId, std::string mFileName, std::string mName, std::string mAlbum, std::string mAuthor);

			void addSegment(int mSeconds);			
			void playRandomSegment(sf::Music*& mMusicPtr);
			void playSegment(sf::Music*& mMusicPtr, int mSegmentIndex);
			void playSeconds(sf::Music*& mMusicPtr, int mSeconds);

			std::string getId();
			std::string getFileName();
			std::string getName();
			std::string getAlbum();
			std::string getAuthor();

			void setFirstPlay(bool mFirstPlay);
			bool getFirstPlay();
	};
}

#endif // MUSICDATA_H_
