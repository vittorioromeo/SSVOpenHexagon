/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
	int MusicData::getRandomSegment() { return segments[getRnd(0, segments.size() + 1)]; }
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

