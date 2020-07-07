// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"

#include <string>

namespace hg
{

class HGAssets;

class MusicData
{
private:
    std::vector<float> segments;

    [[nodiscard]] float getRandomSegment() const
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

    float getSegment(int index)
    {
        return segments[index];
    }

    void addSegment(float mSeconds)
    {
        segments.emplace_back(mSeconds);
    }

    void playRandomSegment(const std::string& mPackId, HGAssets& mAssets)
    {
        if(firstPlay)
        {
            firstPlay = false;
            playSegment(mPackId, mAssets, 0);
        }
        else
        {
            playSeconds(mPackId, mAssets, getRandomSegment());
        }
    }

    void playSegment(
        const std::string& mPackId, HGAssets& mAssets, std::size_t mIdx)
    {
        playSeconds(mPackId, mAssets, segments[mIdx]);
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
