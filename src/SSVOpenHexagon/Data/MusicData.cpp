// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/MusicData.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Global/Config.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvu;

namespace hg
{
	MusicData::MusicData(const string& mId, const string& mFileName, const string& mName, const string& mAlbum, const string& mAuthor) :
		id{mId}, fileName{mFileName}, name{mName}, album{mAlbum}, author{mAuthor}
	{
		musicPtr = getMusicPtr(mId);
		if(musicPtr == nullptr) log("Error loading music <" + mId + "> - ID not found", "MusicData::MusicData");
	}

	void MusicData::addSegment(int mSeconds) { segments.push_back(mSeconds); }
	int MusicData::getRandomSegment() { return segments[getRnd(0, segments.size())]; }
	void MusicData::playRandomSegment()
	{
		if(firstPlay) { firstPlay = false; playSegment(0); }
		else playSeconds(getRandomSegment());
	}
	void MusicData::playSegment(int mSegmentIndex)
	{
		playSeconds(segments[mSegmentIndex]);
	}
	void MusicData::playSeconds(int mSeconds)
	{
		if(getNoMusic()) return;

		musicPtr->setPlayingOffset(seconds(mSeconds));
		musicPtr->play();
	}

	Music* MusicData::getMusic()	{ return musicPtr; }

	string MusicData::getId() 		{ return id; }
	string MusicData::getFileName() { return fileName; }
	string MusicData::getName() 	{ return name; }
	string MusicData::getAlbum() 	{ return album; }
	string MusicData::getAuthor() 	{ return author; }

	void MusicData::setFirstPlay(bool mFirstPlay)	{ firstPlay = mFirstPlay; }
	bool MusicData::getFirstPlay() 					{ return firstPlay; }
}

