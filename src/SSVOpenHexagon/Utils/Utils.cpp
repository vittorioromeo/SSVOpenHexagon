// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace hg::Utils;
using namespace ssvuj;
using namespace ssvu::FileSystem;
using namespace ssvu;

namespace hg
{
    namespace Utils
    {
        Color getColorDarkened(Color mColor, float mMultiplier)
        {
            mColor.r /= mMultiplier;
            mColor.b /= mMultiplier;
            mColor.g /= mMultiplier;
            return mColor;
        }

        MusicData loadMusicFromJson(const ssvuj::Obj& mRoot)
        {
            MusicData result{getExtr<string>(mRoot, "id"),
                getExtr<string>(mRoot, "file_name"),
                getExtr<string>(mRoot, "name"), getExtr<string>(mRoot, "album"),
                getExtr<string>(mRoot, "author")};
            for(const auto& segment : ssvuj::getObj(mRoot, "segments"))
                result.addSegment(getExtr<float>(segment, "time"));
            return result;
        }
        ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot)
        {
            return {getExtr<float>(mRoot, "version"),
                getExtr<string>(mRoot, "name"), ssvuj::getObj(mRoot, "scores"),
                getExtr<vector<string>>(mRoot, "trackedNames", {})};
        }

        string getLocalValidator(const string& mId, float mDifficultyMult)
        {
            return mId + "_m_" + toStr(mDifficultyMult);
        }

        void shakeCamera(TimelineManager& mTimelineManager, Camera& mCamera)
        {
            int s{7};
            Vec2f oldCenter{mCamera.getCenter()};
            Timeline& timeline(mTimelineManager.create());

            for(int i{s}; i > 0; --i)
            {
                timeline.append<Do>([&mCamera, oldCenter, i]
                    {
                        mCamera.setCenter(
                            oldCenter + Vec2f(getRndI(-i, i), getRndI(-i, i)));
                    });
                timeline.append<Wait>(1);
                timeline.append<Go>(0, 3);
            }

            timeline.append<Do>([&mCamera, oldCenter]
                {
                    mCamera.setCenter(oldCenter);
                });
        }

        std::set<string> getIncludedLuaFileNames(const string& mLuaScript)
        {
            std::set<string> result;
            std::size_t currI(0);

            auto findNext = [&currI, &mLuaScript](const auto& x)
            {
                auto f = mLuaScript.find(x, currI);
                currI = f;
            };

            while(true)
            {
                if(currI >= mLuaScript.size()) break;

                findNext("u_execScript");
                if(currI >= mLuaScript.size()) break;
                if(currI == std::string::npos) break;

                findNext("(");
                if(currI == std::string::npos)
                {
                    throw std::runtime_error(
                        "Expected `(` after `u_execScript`");
                }

                findNext("\"");
                if(currI == std::string::npos)
                {
                    throw std::runtime_error(
                        "Expected `\"` after `u_execScript(`");
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
        void recursiveFillIncludedLuaFileNames(
            std::set<string>& mLuaScriptNames, const Path& mPackPath,
            const string& mLuaScript)
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

        Color transformHue(const Color& in, float H)
        {
            float u{cos(H * 3.14f / 180.f)};
            float w{sin(H * 3.14f / 180.f)};

            Color ret;
            ret.r = (.701 * u + .168 * w) * in.r +
                    (-.587 * u + .330 * w) * in.g +
                    (-.114 * u - .497 * w) * in.b;
            ret.g = (-.299 * u - .328 * w) * in.r +
                    (.413 * u + .035 * w) * in.g +
                    (-.114 * u + .292 * w) * in.b;
            ret.b = (-.3 * u + 1.25 * w) * in.r +
                    (-.588 * u - 1.05 * w) * in.g +
                    (.886 * u - .203 * w) * in.b;
            return ret;
        }
    }
}
