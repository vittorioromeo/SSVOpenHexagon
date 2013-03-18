// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Data/MusicData.h"
#include "Global/Assets.h"
#include "Utils/Utils.h"
#include "Global/Config.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvu::Utils;

namespace hg
{
	MusicData::MusicData(const string& mId, const string& mFileName, const string& mName, const string& mAlbum, const string& mAuthor) :
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

