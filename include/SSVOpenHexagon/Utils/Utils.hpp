// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <vector>
#include <SFML/Base/Optional.hpp>
#include <functional>

namespace Lua {
class LuaContext;
}

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
struct VoidToNothingImpl
{
    using type = T;
};

template <>
struct VoidToNothingImpl<void>
{
    using type = Nothing;
};

template <typename T>
using VoidToNothing = typename VoidToNothingImpl<T>::type;

template <typename A, typename B>
inline constexpr bool isSameType = false;

template <typename T>
inline constexpr bool isSameType<T, T> = true;

template <typename T, typename... TArgs>
T runLuaFunction(
    Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs);

template <typename T, typename... TArgs>
sf::base::Optional<VoidToNothing<T>> runLuaFunctionIfExists(
    Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs);

template <typename... TArgs>
void runVoidLuaFunctionIfExists(
    Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs)
{
    (void)runLuaFunctionIfExists<void>(mLua, mName, mArgs...);
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
