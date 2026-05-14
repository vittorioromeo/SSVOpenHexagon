// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/Random.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"


namespace hg
{

MusicData::MusicData() = default;

MusicData::MusicData(const sf::base::String& mId,
                     const sf::base::String& mFileName,
                     const sf::base::String& mName,
                     const sf::base::String& mAlbum,
                     const sf::base::String& mAuthor) :
    id{mId},
    fileName{mFileName},
    name{mName},
    album{mAlbum},
    author{mAuthor}
{
}

[[nodiscard]] const MusicData::Segment& MusicData::getSegment(sf::base::SizeT index) const
{
    return segments[index];
}

void MusicData::addSegment(float mSeconds, float mBeatPulseDelayOffset)
{
    segments.pushBack(Segment{mSeconds, mBeatPulseDelayOffset});
}

[[nodiscard]] MusicData::Segment MusicData::playRandomSegment(const sf::base::String& mPackId, Audio& mAudio)
{
    if (firstPlay)
    {
        firstPlay = false;
        return playSegment(mPackId, mAudio, 0);
    }

    const sf::base::SizeT rndIdx = hg::Utils::getRndI(sf::base::SizeT(0), segments.size());
    return playSegment(mPackId, mAudio, rndIdx);
}

[[nodiscard]] MusicData::Segment MusicData::playSegment(const sf::base::String& mPackId, Audio& mAudio, sf::base::SizeT mIdx)
{
    const Segment& segment = segments[mIdx];
    playSeconds(mPackId, mAudio, segment.time);
    return segment;
}

void MusicData::playSeconds(const sf::base::String& mPackId, Audio& mAudio, float mSeconds) const
{
    if (!mAudio.loadAndPlayMusic(mPackId, id, mSeconds))
    {
        hg::lo("MusicData::playSeconds") << "Failed playing music '" << mPackId << '_' << id << "'\n";
    }
}

} // namespace hg
