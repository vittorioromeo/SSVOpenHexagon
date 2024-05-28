// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include <string>
#include <type_traits>
#include <tuple>
#include <vector>
#include <optional>
#include <functional>

namespace ssvu {

class TimelineManager;

}

namespace ssvs {

class Camera;

}

namespace hg {

class HGAssets;
struct PackData;

} // namespace hg

namespace hg::Utils {

void shakeCamera(
    ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

void runLuaCode(Lua::LuaContext& mLua, const std::string& mCode);
void runLuaFile(Lua::LuaContext& mLua, const std::string& mFileName);
bool runLuaFileCached(
    HGAssets& assets, Lua::LuaContext& mLua, const std::string& mFileName);

struct Nothing
{};

template <typename T>
using VoidToNothing = std::conditional_t<std::is_same_v<T, void>, Nothing, T>;

template <typename T, typename... TArgs>
T runLuaFunction(
    Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs)
{
    return mLua.callLuaFunction<T>(mName, std::make_tuple(mArgs...));
}

template <typename T, typename... TArgs>
auto runLuaFunctionIfExists(
    Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs)
{
    using Ret = std::optional<VoidToNothing<T>>;

    if (!mLua.doesVariableExist(mName))
    {
        return Ret{};
    }

    if constexpr (std::is_same_v<T, void>)
    {
        runLuaFunction<T>(mLua, mName, mArgs...);
        return Ret{Nothing{}};
    }
    else
    {
        return Ret{runLuaFunction<T>(mLua, mName, mArgs...)};
    }
}

const PackData& findDependencyPackDataOrThrow(const HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor);

void withDependencyScriptFilename(
    const std::function<void(const std::string&)> f,
    std::vector<std::string>& execScriptPackPathContext, HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor,
    const std::string& mScriptName);

[[nodiscard]] std::string getDependentScriptFilename(
    std::vector<std::string>& execScriptPackPathContext,
    const std::string& currentPackPath, const std::string& mScriptName);

void withDependencyShaderFilename(
    const std::function<void(const std::string&)> f,
    std::vector<std::string>& execScriptPackPathContext, HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor,
    const std::string& mShaderName);

[[nodiscard]] std::string getDependentShaderFilename(
    std::vector<std::string>& execScriptPackPathContext,
    const std::string& currentPackPath, const std::string& mShaderName);

} // namespace hg::Utils
