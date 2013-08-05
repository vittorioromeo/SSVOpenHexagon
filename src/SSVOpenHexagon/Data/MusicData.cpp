// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/MusicData.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Global/Assets.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvu;

namespace hg
{
	MusicData::MusicData(const string& mId, const string& mFileName, const string& mName, const string& mAlbum, const string& mAuthor) :
		id{mId}, fileName{mFileName}, name{mName}, album{mAlbum}, author{mAuthor} { }

	int MusicData::getRandomSegment() const { return segments[getRnd(0, segments.size())]; }
	void MusicData::playRandomSegment(HGAssets& mAssets) { if(firstPlay) { firstPlay = false; playSegment(mAssets, 0); } else playSeconds(mAssets, getRandomSegment());}
	void MusicData::playSegment(HGAssets& mAssets, int mSegmentIndex) { playSeconds(mAssets, segments[mSegmentIndex]); }
	void MusicData::playSeconds(HGAssets& mAssets, int mSeconds) { if(Config::getNoMusic()) return; mAssets.playMusic(id, seconds(mSeconds)); }
}

