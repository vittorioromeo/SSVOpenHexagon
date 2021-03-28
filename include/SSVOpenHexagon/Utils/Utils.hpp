// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/ScopeGuard.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
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
#include <cctype>
#include <stdexcept>

namespace hg::Utils
{

inline constexpr float epsilon{1.0e-4};

inline void uppercasify(std::string& s)
{
    for(auto& c : s)
    {
        c = std::toupper(c);
    }
}

inline void lTrim(std::string& str)
{
    const auto it = std::find_if(
        str.begin(), str.end(), [](char ch) { return !std::isspace(ch); });

    str.erase(str.begin(), it);
}

inline void rTrim(std::string& str)
{
    const auto it = std::find_if(
        str.rbegin(), str.rend(), [](char ch) { return !std::isspace(ch); });

    str.erase(it.base(), str.end());
}

[[nodiscard]] inline std::string getLTrim(std::string s)
{
    lTrim(s);
    return s;
}

[[nodiscard]] inline std::string getRTrim(std::string s)
{
    rTrim(s);
    return s;
}

[[nodiscard]] inline std::string getLRTrim(std::string s)
{
    lTrim(s);
    rTrim(s);
    return s;
}

[[nodiscard]] inline std::string toUppercase(std::string s)
{
    uppercasify(s);
    return s;
}

[[nodiscard]] inline float getFontHeight(sf::Text& font)
{
    font.setString("A");
    return ssvs::getGlobalHeight(font);
}

[[nodiscard]] inline float getFontHeight(
    sf::Text& font, const unsigned int charSize)
{
    font.setCharacterSize(charSize);
    font.setString("A");
    return ssvs::getGlobalHeight(font);
}

bool getLinesIntersection(sf::Vector2f& mIntersection, const sf::Vector2f& l1p1,
    const sf::Vector2f& l1p2, const sf::Vector2f& l2p1,
    const sf::Vector2f& l2p2);

unsigned int getLineCircleIntersection(sf::Vector2f& i1, sf::Vector2f& i2,
    const sf::Vector2f& p1, const sf::Vector2f& p2, const float mRadiusSquared);

bool getLineCircleClosestIntersection(sf::Vector2f& mIntersection,
    const sf::Vector2f& mPos, const sf::Vector2f& p1, const sf::Vector2f& p2,
    const float mRadiusSquared);

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSaturated(
    float mValue)
{
    return std::max(0.f, std::min(1.f, mValue));
}

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSmoothStep(
    float edge0, float edge1, float x)
{
    x = getSaturated((x - edge0) / (edge1 - edge0));
    return x * x * (3 - 2 * x);
}

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSmootherStep(
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

inline void runLuaCode(Lua::LuaContext& mLua, const std::string& mCode)
{
    try
    {
        mLua.executeCode(mCode);
    }
    catch(std::runtime_error& mError)
    {
        ssvu::lo("hg::Utils::runLuaCode") << "Fatal Lua error\n"
                                          << "Code: " << mCode << '\n'
                                          << "Error: " << mError.what() << '\n'
                                          << std::endl;

        throw;
    }
    catch(...)
    {
        ssvu::lo("hg::Utils::runLuaCode") << "Fatal unknown Lua error\n"
                                          << "Code: " << mCode << '\n'
                                          << std::endl;

        throw;
    }
}

inline void runLuaFile(Lua::LuaContext& mLua, const std::string& mFileName)
{
    std::ifstream s{mFileName};

    if(!s)
    {
        const std::string errorStr = Utils::concat(
            "Fatal Lua error\n", "Could not open file: ", mFileName, '\n');

        ssvu::lo("hg::Utils::runLuaFile") << errorStr << std::endl;
        throw std::runtime_error(errorStr);
    }

#ifndef NDEBUG
    ssvu::lo("hg::Utils::runLuaFile")
        << "Running Lua file '" << mFileName << "'" << std::endl;
#endif

    try
    {
        mLua.executeCode(s);
    }
    catch(std::runtime_error& mError)
    {
        ssvu::lo("hg::Utils::runLuaFile") << "Fatal Lua error\n"
                                          << "Filename: " << mFileName << '\n'
                                          << "Error: " << mError.what() << '\n'
                                          << std::endl;

        throw;
    }
    catch(...)
    {
        ssvu::lo("hg::Utils::runLuaFile") << "Fatal unknown Lua error\n"
                                          << "Filename: " << mFileName << '\n'
                                          << std::endl;

        throw;
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

inline const PackData& findDependencyPackDataOrThrow(const HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor)
{
    const auto& dependencies = currentPack.dependencies;

    // ------------------------------------------------------------------------
    // Check if provided arguments are a dependency of current pack.
    const auto depIt = std::find_if(dependencies.begin(), dependencies.end(),
        [&](const PackDependency& pd) {
            return pd.disambiguator == mPackDisambiguator && //
                   pd.name == mPackName &&                   //
                   pd.author == mPackAuthor;
        });

    if(depIt == dependencies.end())
    {
        throw std::runtime_error(
            Utils::concat("Pack with disambiguator '", mPackDisambiguator,
                "', name '", mPackName, "', author: '", mPackAuthor,
                "' is not a dependency of '", currentPack.name, "'\n"));
    }

    // ------------------------------------------------------------------------
    // Find the pack data corresponding to the specified arguments.
    const PackData* const dependencyData =
        assets.findPackData(mPackDisambiguator, mPackName, mPackAuthor);

    if(dependencyData == nullptr)
    {
        throw std::runtime_error(
            Utils::concat("Could not find dependency pack with disambiguator '",
                mPackDisambiguator, "', name '", mPackName, "', author: '",
                mPackAuthor, "'\n"));
    }

    if(dependencyData->version < depIt->minVersion)
    {
        throw std::runtime_error(
            Utils::concat("Dependency pack with disambiguator '",
                mPackDisambiguator, "', name '", mPackName, "', author: '",
                mPackAuthor, "' has version '", dependencyData->version,
                "' but at least '", depIt->minVersion, "' is required\n"));
    }

    return *dependencyData;
}

template <typename F>
inline void withDependencyScriptFilename(F&& f,
    std::vector<std::string>& execScriptPackPathContext, HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor,
    const std::string& mScriptName)
try
{
    const PackData& dependencyData = findDependencyPackDataOrThrow(
        assets, currentPack, mPackDisambiguator, mPackName, mPackAuthor);

    execScriptPackPathContext.emplace_back(dependencyData.folderPath);
    HG_SCOPE_GUARD({ execScriptPackPathContext.pop_back(); });

    return f(concat(dependencyData.folderPath, "Scripts/", mScriptName));
}
catch(const std::runtime_error& err)
{
    ssvu::lo("hg::Utils::getDependencyScriptFilename")
        << "Fatal error while looking for Lua dependency\nError: " << err.what()
        << std::endl;

    throw;
}
catch(...)
{
    ssvu::lo("hg::Utils::getDependencyScriptFilename")
        << "Fatal unknown error while looking for Lua dependency" << std::endl;

    throw;
}

[[nodiscard]] inline std::string getDependentScriptFilename(
    std::vector<std::string>& execScriptPackPathContext,
    const std::string& currentPackPath, const std::string& mScriptName)
{
    const std::string& context = execScriptPackPathContext.empty()
                                     ? currentPackPath
                                     : execScriptPackPathContext.back();

    return concat(context, "Scripts/", mScriptName);
}

} // namespace hg::Utils
