// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS
#define HG_UTILS

#include <string>
#include <sstream>
#include <set>
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"

namespace hg
{
    namespace Utils
    {
        inline float getSaturated(float mValue)
        {
            return std::max(0.f, std::min(1.f, mValue));
        }
        inline float getSmootherStep(float edge0, float edge1, float x)
        {
            x = getSaturated((x - edge0) / (edge1 - edge0));
            return x * x * x * (x * (x * 6 - 15) + 10);
        }

        sf::Color getColorDarkened(sf::Color mColor, float mMultiplier);

        MusicData loadMusicFromJson(const ssvuj::Obj& mRoot);
        ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot);

        std::string getLocalValidator(
            const std::string& mId, float mDifficultyMult);

        void shakeCamera(
            ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

        std::set<std::string> getIncludedLuaFileNames(
            const std::string& mLuaScript);

        void recursiveFillIncludedLuaFileNames(
            std::set<std::string>& mLuaScriptNames, const Path& mPackPath,
            const std::string& mLuaScript);

        sf::Color transformHue(const sf::Color& in, float H);

        inline void runLuaFile(
            Lua::LuaContext& mLua, const std::string& mFileName)
        {
            std::ifstream s{mFileName};
            try
            {
                mLua.executeCode(s);
            }
            catch(std::runtime_error& mError)
            {
                ssvu::lo("hg::Utils::runLuaFile") << "Fatal lua error"
                                                  << "\n";
                ssvu::lo("hg::Utils::runLuaFile") << "Filename: " << mFileName
                                                  << "\n";
                ssvu::lo("hg::Utils::runLuaFile") << "Error: " << mError.what()
                                                  << "\n" << std::endl;
            }
        }
        template <typename T, typename... TArgs>
        inline T runLuaFunction(Lua::LuaContext& mLua, const std::string& mName,
            const TArgs&... mArgs)
        {
            try
            {
                return mLua.callLuaFunction<T>(mName, ssvu::mkTpl(mArgs...));
            }
            catch(std::runtime_error& mError)
            {
                std::cout << mName << "\n"
                          << "LUA runtime error: "
                          << "\n" << ssvu::toStr(mError.what()) << "\n"
                          << std::endl;
            }

            return T();
        }

        template <typename T, typename... TArgs>
        inline void runLuaFunctionIfExists(Lua::LuaContext& mLua,
            const std::string& mName, const TArgs&... mArgs)
        {
            if(!mLua.doesVariableExist(mName)) return;

            runLuaFunction<T>(mLua, mName, mArgs...);
        }
    }
}

#endif
