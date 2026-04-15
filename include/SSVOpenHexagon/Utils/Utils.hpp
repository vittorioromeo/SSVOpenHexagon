// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <functional>
#include <string_view>

namespace Lua
{
class LuaContext;
}

namespace ssvu
{

class TimelineManager;

}

namespace hg
{

class HGAssets;
struct PackData;

} // namespace hg

namespace hg::Utils
{

void runLuaCode(Lua::LuaContext& mLua, const sf::base::String& mCode);
void runLuaFile(Lua::LuaContext& mLua, const sf::base::String& mFileName);
bool runLuaFileCached(HGAssets& assets, Lua::LuaContext& mLua, const sf::base::String& mFileName);

struct Nothing
{
};

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
T runLuaFunction(Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs);

template <typename T, typename... TArgs>
sf::base::Optional<VoidToNothing<T>> runLuaFunctionIfExists(Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs);

template <typename... TArgs>
void runVoidLuaFunctionIfExists(Lua::LuaContext& mLua, std::string_view mName, const TArgs&... mArgs)
{
    (void)runLuaFunctionIfExists<void>(mLua, mName, mArgs...);
}

const PackData& findDependencyPackDataOrThrow(
    const HGAssets&         assets,
    const PackData&         currentPack,
    const sf::base::String& mPackDisambiguator,
    const sf::base::String& mPackName,
    const sf::base::String& mPackAuthor);

void withDependencyScriptFilename(
    const std::function<void(const sf::base::String&)> f,
    sf::base::Vector<sf::base::String>&                execScriptPackPathContext,
    HGAssets&                                          assets,
    const PackData&                                    currentPack,
    const sf::base::String&                            mPackDisambiguator,
    const sf::base::String&                            mPackName,
    const sf::base::String&                            mPackAuthor,
    const sf::base::String&                            mScriptName);

[[nodiscard]] sf::base::String getDependentScriptFilename(sf::base::Vector<sf::base::String>& execScriptPackPathContext,
                                                          const sf::base::String&             currentPackPath,
                                                          const sf::base::String&             mScriptName);

void withDependencyShaderFilename(
    const std::function<void(const sf::base::String&)> f,
    sf::base::Vector<sf::base::String>&                execScriptPackPathContext,
    HGAssets&                                          assets,
    const PackData&                                    currentPack,
    const sf::base::String&                            mPackDisambiguator,
    const sf::base::String&                            mPackName,
    const sf::base::String&                            mPackAuthor,
    const sf::base::String&                            mShaderName);

[[nodiscard]] sf::base::String getDependentShaderFilename(sf::base::Vector<sf::base::String>& execScriptPackPathContext,
                                                          const sf::base::String&             currentPackPath,
                                                          const sf::base::String&             mShaderName);

} // namespace hg::Utils
