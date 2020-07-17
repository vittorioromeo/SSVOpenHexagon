// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <SSVStart/Camera/Camera.hpp>

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <fstream>

#include <dirent.h>
#include <sys/stat.h>

namespace hg::Utils
{

sf::Color getColorDarkened(sf::Color mColor, float mMultiplier)
{
    mColor.r /= mMultiplier;
    mColor.b /= mMultiplier;
    mColor.g /= mMultiplier;
    return mColor;
}

MusicData loadMusicFromJson(const ssvuj::Obj& mRoot)
{
    MusicData result{ssvuj::getExtr<std::string>(mRoot, "id"),
        ssvuj::getExtr<std::string>(mRoot, "file_name"),
        ssvuj::getExtr<std::string>(mRoot, "name"),
        ssvuj::getExtr<std::string>(mRoot, "album"),
        ssvuj::getExtr<std::string>(mRoot, "author")};
    for(const auto& segment : ssvuj::getObj(mRoot, "segments"))
    {
        result.addSegment(ssvuj::getExtr<float>(segment, "time"),
            ssvuj::getExtr<float>(segment, "beatPulseDelayOffset", 0.f));
    }
    return result;
}

ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot)
{
    return {ssvuj::getExtr<float>(mRoot, "version"),
        ssvuj::getExtr<std::string>(mRoot, "name"),
        ssvuj::getObj(mRoot, "scores"),
        ssvuj::getExtr<std::vector<std::string>>(mRoot, "trackedNames", {})};
}

std::string getLocalValidator(const std::string& mId, float mDifficultyMult)
{
    return mId + "_m_" + ssvu::toStr(mDifficultyMult);
}

void shakeCamera(ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera)
{
    int s{7};
    sf::Vector2f oldCenter{mCamera.getCenter()};
    ssvu::Timeline& timeline(mTimelineManager.create());

    for(int i{s}; i > 0; --i)
    {
        timeline.append<ssvu::Do>([&mCamera, oldCenter, i] {
            mCamera.setCenter(oldCenter + sf::Vector2f(ssvu::getRndI(-i, i),
                                              ssvu::getRndI(-i, i)));
        });
        timeline.append<ssvu::Wait>(1);
        timeline.append<ssvu::Go>(0, 3);
    }

    timeline.append<ssvu::Do>(
        [&mCamera, oldCenter] { mCamera.setCenter(oldCenter); });
}

std::set<std::string> getIncludedLuaFileNames(const std::string& mLuaScript)
{
    std::set<std::string> result;
    std::size_t currI(0);

    auto findNext = [&currI, &mLuaScript](const auto& x) {
        auto f = mLuaScript.find(x, currI);
        currI = f;
    };

    while(true)
    {
        if(currI >= mLuaScript.size())
        {
            break;
        }

        findNext("u_execScript");
        if(currI >= mLuaScript.size())
        {
            break;
        }
        if(currI == std::string::npos)
        {
            break;
        }

        findNext("(");
        if(currI == std::string::npos)
        {
            throw std::runtime_error("Expected `(` after `u_execScript`");
        }

        findNext("\"");
        if(currI == std::string::npos)
        {
            throw std::runtime_error("Expected `\"` after `u_execScript(`");
        }

        // beginning of script name
        auto startI(currI + 1);
        ++currI;

        findNext("\"");
        if(currI == std::string::npos)
        {
            throw std::runtime_error(
                "Expected `\"` after `u_execScript(\"...`");
        }

        // end of script name
        auto leng = currI - startI;
        auto scriptName = mLuaScript.substr(startI, leng);

        result.insert(scriptName);
        ++currI;
    }

    return result;
}

void recursiveFillIncludedLuaFileNames(std::set<std::string>& mLuaScriptNames,
    const ssvufs::Path& mPackPath, const std::string& mLuaScript)
{
    for(const auto& name : getIncludedLuaFileNames(mLuaScript))
    {
        ssvufs::Path p{mPackPath + "/Scripts/" + name};

        if(!p.exists<ssvufs::Type::File>())
        {
            throw std::runtime_error(
                "\nCould not find script file:\n" + p.getStr() + "\n");
        }

        mLuaScriptNames.insert(name);

        try
        {
            recursiveFillIncludedLuaFileNames(
                mLuaScriptNames, mPackPath, p.getContentsAsStr());
        }
        catch(const std::runtime_error& re)
        {
            std::string s;
            s += re.what();
            s += "...from...";
            s += p.getStr();
            s += "\n";

            throw std::runtime_error(s);
        }
    }
}

[[gnu::const]] sf::Color transformHue(const sf::Color& in, float H)
{
    const float u{std::cos(H * 3.14f / 180.f)};
    const float w{std::sin(H * 3.14f / 180.f)};

    sf::Color ret;
    ret.r = (.701 * u + .168 * w) * in.r + (-.587 * u + .330 * w) * in.g +
            (-.114 * u - .497 * w) * in.b;
    ret.g = (-.299 * u - .328 * w) * in.r + (.413 * u + .035 * w) * in.g +
            (-.114 * u + .292 * w) * in.b;
    ret.b = (-.3 * u + 1.25 * w) * in.r + (-.588 * u - 1.05 * w) * in.g +
            (.886 * u - .203 * w) * in.b;
    return ret;
}

} // namespace hg::Utils
