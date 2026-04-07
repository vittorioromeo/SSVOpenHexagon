// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


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
    sf::base::Vector<Segment> segments;

public:
    sf::base::String id;
    sf::base::String fileName;
    sf::base::String name;
    sf::base::String album;
    sf::base::String author;
    bool             firstPlay{true};

    MusicData();

    MusicData(const sf::base::String& mId,
              const sf::base::String& mFileName,
              const sf::base::String& mName,
              const sf::base::String& mAlbum,
              const sf::base::String& mAuthor);

    [[nodiscard]] const Segment& getSegment(sf::base::SizeT index) const;

    void addSegment(float mSeconds, float mBeatPulseDelayOffset);

    [[nodiscard]] Segment playRandomSegment(const sf::base::String& mPackId, Audio& mAudio);

    [[nodiscard]] Segment playSegment(const sf::base::String& mPackId, Audio& mAudio, sf::base::SizeT mIdx);

    void playSeconds(const sf::base::String& mPackId, Audio& mAudio, float mSeconds) const;
};

} // namespace hg
