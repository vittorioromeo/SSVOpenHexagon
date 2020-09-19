// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"

#include <SFML/System.hpp>

#include <string>

namespace hg
{

class HGAssets;

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

    [[nodiscard]] const Segment& getRandomSegment() const
    {
        return segments[ssvu::getRndI(std::size_t(0), segments.size())];
    }

public:
    std::string id;
    std::string fileName;
    std::string name;
    std::string album;
    std::string author;
    bool firstPlay{true};

    MusicData() = default;
    MusicData(const std::string& mId, const std::string& mFileName,
        const std::string& mName, const std::string& mAlbum,
        const std::string& mAuthor)
        : id{mId}, fileName{mFileName}, name{mName}, album{mAlbum}, author{
                                                                        mAuthor}
    {
    }

    const Segment& getSegment(std::size_t index) const
    {
        return segments[index];
    }

    void addSegment(float mSeconds, float mBeatPulseDelayOffset)
    {
        segments.emplace_back(Segment{mSeconds, mBeatPulseDelayOffset});
    }

    Segment playRandomSegment(const std::string& mPackId, HGAssets& mAssets)
    {
        if(firstPlay)
        {
            firstPlay = false;
            return playSegment(mPackId, mAssets, 0);
        }
        else
        {
            const Segment& segment = getRandomSegment();
            playSeconds(mPackId, mAssets, segment.time);
            return segment;
        }
    }

    Segment playSegment(
        const std::string& mPackId, HGAssets& mAssets, std::size_t mIdx)
    {
        const Segment& segment = segments[mIdx];
        playSeconds(mPackId, mAssets, segment.time);
        return segment;
    }

    void playSeconds(
        const std::string& mPackId, HGAssets& mAssets, float mSeconds) const
    {
        if(Config::getNoMusic())
        {
            return;
        }

        mAssets.playMusic(mPackId, id, sf::seconds(mSeconds));
    }
};

} // namespace hg
