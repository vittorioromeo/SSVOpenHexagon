#include <SFML/Audio.hpp>
#include "Data/MusicData.h"
#include "Global/Assets.h"
#include "Utils/Utils.h"

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
		if(firstPlay)
		{
			firstPlay = false;
			playSegment(mMusicPtr, 0);
		}
		else playSeconds(mMusicPtr, getRandomSegment());
	}
	void MusicData::playSegment(Music*& mMusicPtr, int mSegmentIndex)
	{
		playSeconds(mMusicPtr, segments[mSegmentIndex]);
	}
	void MusicData::playSeconds(Music*& mMusicPtr, int mSeconds)
	{
		mMusicPtr = musicPtr;
		if(getNoMusic()) return;

		musicPtr->setPlayingOffset(sf::seconds(mSeconds));
		musicPtr->play();
	}

	string MusicData::getId() 		{ return id; }
	string MusicData::getFileName() { return fileName; }
	string MusicData::getName() 	{ return name; }
	string MusicData::getAlbum() 	{ return album; }
	string MusicData::getAuthor() 	{ return author; }

	void MusicData::setFirstPlay(bool mFirstPlay) 	{ firstPlay = mFirstPlay; }
	bool MusicData::getFirstPlay() 					{ return firstPlay; }
}

