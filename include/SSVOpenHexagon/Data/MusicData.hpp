// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"

namespace hg
{

class HGAssets;

class MusicData
{
private:
    std::vector<float> segments;

    [[nodiscard]] float getRandomSegment() const
    {
        return segments[ssvu::getRndI(SizeT(0), segments.size())];
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

    float getSegment(int index)
    {
        return segments[index];
    }

    void addSegment(float mSeconds)
    {
        segments.emplace_back(mSeconds);
    }

    void playRandomSegment(HGAssets& mAssets)
    {
        if(firstPlay)
        {
            firstPlay = false;
            playSegment(mAssets, 0);
        }
        else
        {
            playSeconds(mAssets, getRandomSegment());
        }
    }

    void playSegment(HGAssets& mAssets, SizeT mIdx)
    {
        playSeconds(mAssets, segments[mIdx]);
    }

    void playSeconds(HGAssets& mAssets, float mSeconds) const
    {
        if(Config::getNoMusic())
        {
            return;
        }

        mAssets.playMusic(id, sf::seconds(mSeconds));
    }
};

} // namespace hg
