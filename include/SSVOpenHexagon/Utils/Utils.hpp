// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <SSVStart/Camera/Camera.hpp>

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>
#include <SSVUtils/Timeline/Timeline.hpp>

#include <SFML/Graphics/Color.hpp>

#include <string>
#include <sstream>
#include <set>
#include <cctype>
#include <optional>

namespace hg::Utils
{

constexpr float epsilon{1.0e-4};

inline void uppercasify(std::string& s)
{
    for(auto& c : s)
    {
        c = std::toupper(c);
    }
}

[[nodiscard]] inline std::string toUppercase(std::string s)
{
    uppercasify(s);
    return s;
}

[[nodiscard, gnu::pure]] inline float getFontHeight(sf::Text& font)
{
    font.setString("A");
    return ssvs::getGlobalHeight(font);
}

[[nodiscard, gnu::pure]] inline float getFontHeight(sf::Text& font, const unsigned int charSize)
{
    font.setCharacterSize(charSize);
    font.setString("A");
    return ssvs::getGlobalHeight(font);
}

[[nodiscard, gnu::pure]] inline float fastSqrt(const float &n) 
{
    static union{int i; float f;} u;
    u.i = 0x5F375A86 - (*(int*)&n >> 1);
    return (int(3) - n * u.f * u.f) * n * u.f * 0.5f;
}

[[nodiscard, gnu::pure]] inline float getSaturated(float mValue)
{
    return std::max(0.f, std::min(1.f, mValue));
}

[[nodiscard, gnu::pure]] inline float getSmootherStep(
    float edge0, float edge1, float x)
{
    x = getSaturated((x - edge0) / (edge1 - edge0));
    return x * x * x * (x * (x * 6 - 15) + 10);
}

MusicData loadMusicFromJson(const ssvuj::Obj& mRoot);
GameVersion loadVersionFromJson(const ssvuj::Obj& mRoot);
ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot);

std::string getLocalValidator(const std::string& mId, float mDifficultyMult);

void shakeCamera(
    ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

std::set<std::string> getIncludedLuaFileNames(const std::string& mLuaScript);

void recursiveFillIncludedLuaFileNames(std::set<std::string>& mLuaScriptNames,
    const ssvufs::Path& mPackPath, const std::string& mLuaScript);

[[gnu::pure]] sf::Color transformHue(const sf::Color& in, float H);

inline void runLuaFile(Lua::LuaContext& mLua, const std::string& mFileName)
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
        ssvu::lo("hg::Utils::runLuaFile") << "Filename: " << mFileName << "\n";
        ssvu::lo("hg::Utils::runLuaFile") << "Error: " << mError.what() << "\n"
                                          << std::endl;
    }
}

struct Nothing
{
};

template <typename T>
using VoidToNothing = std::conditional_t<std::is_same_v<T, void>, Nothing, T>;

template <typename T, typename... TArgs>
T runLuaFunction(
    Lua::LuaContext& mLua, const std::string& mName, const TArgs&... mArgs)
{
    return mLua.callLuaFunction<T>(mName, std::make_tuple(mArgs...));
}

template <typename T, typename... TArgs>
auto runLuaFunctionIfExists(
    Lua::LuaContext& mLua, const std::string& mName, const TArgs&... mArgs)
{
    using Ret = std::optional<VoidToNothing<T>>;

    if(!mLua.doesVariableExist(mName))
    {
        return Ret{};
    }

    if constexpr(std::is_same_v<T, void>)
    {
        runLuaFunction<T>(mLua, mName, mArgs...);
        return Ret{Nothing{}};
    }
    else
    {
        return Ret{runLuaFunction<T>(mLua, mName, mArgs...)};
    }
}

template <typename T1, typename T2, typename T3>
[[nodiscard]] auto getSkewedVecFromRad(
    const T1& mRad, const T2& mMag, const T3& mSkew) noexcept
{
    return ssvs::Vec2<ssvs::CT<T1, T2>>(
        std::cos(mRad) * (mMag / mSkew.x), std::sin(mRad) * (mMag / mSkew.y));
}

template <typename T1, typename T2, typename T3, typename T4>
[[nodiscard]] auto getSkewedOrbitRad(const ssvs::Vec2<T1>& mVec, const T2& mRad,
    const T3& mRadius, const T4& mSkew) noexcept
{
    return ssvs::Vec2<ssvs::CT<T1, T2, T3>>(mVec) +
           getSkewedVecFromRad(mRad, mRadius, mSkew);
}


} // namespace hg::Utils
