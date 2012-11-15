#include "MusicData.h"
#include <SFML/Audio.hpp>
#include "Utils.h"
#include "Assets.h"

namespace hg
{
	MusicData::MusicData(string mId, string mFileName, string mName, string mAlbum, string mAuthor) :
		id{mId}, fileName{mFileName}, name{mName}, album{mAlbum}, author{mAuthor}
	{
		musicPtr = getMusicPtr(mId);
		musicPtr->setLoop(true);
		musicPtr->setVolume(getMusicVolume());
	}

	void MusicData::addSegment(int mSeconds) { segments.push_back(mSeconds); }
	int MusicData::getRandomSegment() { return segments[getRnd(0, segments.size())]; }
	void MusicData::playRandomSegment(Music*& mMusicPtr)
	{
		mMusicPtr = musicPtr;

		if(firstPlay)
		{
			firstPlay = false;
			musicPtr->setPlayingOffset(sf::seconds(segments[0]));
		}
		else musicPtr->setPlayingOffset(sf::seconds(getRandomSegment()));

		musicPtr->play();
	}

	string MusicData::getId() 		{ return id; }
	string MusicData::getFileName() { return fileName; }
	string MusicData::getName() 	{ return name; }
	string MusicData::getAlbum() 	{ return album; }
	string MusicData::getAuthor() 	{ return author; }
}

