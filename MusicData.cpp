#include "MusicData.h"
#include <SFML/Audio.hpp>
#include "Utils.h"

namespace hg
{
	MusicData::MusicData(string mId, string mFileName, string mName, string mAlbum, string mAuthor) :
		id{mId}, fileName{mFileName}, name{mName}, album{mAlbum}, author{mAuthor}
	{
		musicPtr->openFromFile("Music/" + mFileName);
		musicPtr->setLoop(true);
		musicPtr->setVolume(getMusicVolume());
	}
	MusicData::~MusicData()
	{
		//delete musicPtr;
	}

	void MusicData::addSegment(int mSeconds) { segments.push_back(mSeconds); }
	int MusicData::getRandomSegment() { return segments[rnd(0, segments.size())]; }
	void MusicData::playRandomSegment(Music*& mMusicPtr)
	{
		mMusicPtr = musicPtr;

		int segment{getRandomSegment()};
		musicPtr->setPlayingOffset(sf::seconds(segment));
		musicPtr->play();
	}

	string MusicData::getId() 		{ return id; }
	string MusicData::getFileName() { return fileName; }
	string MusicData::getName() 	{ return name; }
	string MusicData::getAlbum() 	{ return album; }
	string MusicData::getAuthor() 	{ return author; }
}

