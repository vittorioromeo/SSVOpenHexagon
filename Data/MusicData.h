#ifndef MUSICDATA_H_
#define MUSICDATA_H_

#include <SFML/Audio.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace sf;

namespace hg
{
	class MusicData
	{
		private:
			vector<int> segments;
			string id 			{""};
			string fileName 	{""};
			string name			{""};
			string album		{""};
			string author		{""};

			Music* musicPtr		{new Music};
			bool firstPlay		{true};
			int getRandomSegment();

		public:
			MusicData() = default;
			MusicData(string mId, string mFileName, string mName, string mAlbum, string mAuthor);

			void addSegment(int mSeconds);			
			void playRandomSegment(Music*& mMusicPtr);
			void playSegment(Music*& mMusicPtr, int mSegmentIndex);
			void playSeconds(Music*& mMusicPtr, int mSeconds);

			string getId();
			string getFileName();
			string getName();
			string getAlbum();
			string getAuthor();
	};
}

#endif // MUSICDATA_H_
