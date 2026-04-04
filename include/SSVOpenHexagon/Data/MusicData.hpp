// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/SizeT.hpp"

#include <string>
#include <vector>


namespace hg
{

class Audio;

class MusicData
{
public:
    struct Segment
    {
        float time;
        float beatPulseDelayOffset;
    };

private:
    std::vector<Segment> segments;

public:
    std::string id;
    std::string fileName;
    std::string name;
    std::string album;
    std::string author;
    bool        firstPlay{true};

    MusicData();

    MusicData(const std::string& mId,
              const std::string& mFileName,
              const std::string& mName,
              const std::string& mAlbum,
              const std::string& mAuthor);

    [[nodiscard]] const Segment& getSegment(sf::base::SizeT index) const;

    void addSegment(float mSeconds, float mBeatPulseDelayOffset);

    [[nodiscard]] Segment playRandomSegment(const std::string& mPackId, Audio& mAudio);

    [[nodiscard]] Segment playSegment(const std::string& mPackId, Audio& mAudio, sf::base::SizeT mIdx);

    void playSeconds(const std::string& mPackId, Audio& mAudio, float mSeconds) const;
};

} // namespace hg
