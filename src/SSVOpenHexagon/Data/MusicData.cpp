// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/MusicData.hpp"

#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

#include <SSVUtils/Core/Utils/Rnd.hpp>
#include <SSVUtils/Core/Log/Log.hpp>

#include <string>
#include <cstddef>

namespace hg {

MusicData::MusicData() = default;

MusicData::MusicData(const std::string& mId, const std::string& mFileName,
    const std::string& mName, const std::string& mAlbum,
    const std::string& mAuthor)
    : id{mId}, fileName{mFileName}, name{mName}, album{mAlbum}, author{mAuthor}
{}

[[nodiscard]] const MusicData::Segment& MusicData::getSegment(
    std::size_t index) const
{
    return segments[index];
}

void MusicData::addSegment(float mSeconds, float mBeatPulseDelayOffset)
{
    segments.push_back(Segment{mSeconds, mBeatPulseDelayOffset});
}

[[nodiscard]] MusicData::Segment MusicData::playRandomSegment(
    const std::string& mPackId, Audio& mAudio)
{
    if(firstPlay)
    {
        firstPlay = false;
        return playSegment(mPackId, mAudio, 0);
    }

    const std::size_t rndIdx = ssvu::getRndI(std::size_t(0), segments.size());
    return playSegment(mPackId, mAudio, rndIdx);
}

[[nodiscard]] MusicData::Segment MusicData::playSegment(
    const std::string& mPackId, Audio& mAudio, std::size_t mIdx)
{
    const Segment& segment = segments[mIdx];
    playSeconds(mPackId, mAudio, segment.time);
    return segment;
}

void MusicData::playSeconds(
    const std::string& mPackId, Audio& mAudio, float mSeconds) const
{
    if(!mAudio.loadAndPlayMusic(mPackId, id, mSeconds))
    {
        ssvu::lo("MusicData::playSeconds")
            << "Failed playing music '" << mPackId << '_' << id << "'\n";
    }
}

} // namespace hg
