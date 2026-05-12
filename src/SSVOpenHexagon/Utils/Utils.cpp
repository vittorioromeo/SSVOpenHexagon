// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#include <stdexcept>
#include <tuple>

namespace hg::Utils
{

void runLuaCode(Lua::LuaContext& mLua, const sf::base::String& mCode)
try
{
    mLua.executeCode(mCode.cStr());
} catch (std::runtime_error& mError)
{
    hg::lo("hg::Utils::runLuaCode") << "Fatal Lua error\n"
                                    << "Code: " << mCode << '\n'
                                    << "Error: " << mError.what() << '\n'
                                    << logEndl;

    throw;
} catch (...)
{
    hg::lo("hg::Utils::runLuaCode") << "Fatal unknown Lua error\n"
                                    << "Code: " << mCode << '\n'
                                    << logEndl;

    throw;
}

void runLuaFile(Lua::LuaContext& mLua, const sf::base::String& mFileName)
{
    sf::base::Vector<char>& fileBuf = sf::getThreadLocalScratchCharBuffer();
    fileBuf.clear();

    if (!sf::readFromFile(sf::Path{mFileName.cStr()}, fileBuf))
    {
        const sf::base::String errorStr = concat("Fatal Lua error\n", "Could not open file: ", mFileName, '\n');

        hg::lo("hg::Utils::runLuaFile") << errorStr << logEndl;
        throw std::runtime_error(errorStr.cStr());
    }

    try
    {
        mLua.executeCode(sf::base::StringView{fileBuf.data(), fileBuf.size()});
    } catch (std::runtime_error& mError)
    {
        hg::lo("hg::Utils::runLuaFile") << "Fatal Lua error\n"
                                        << "Filename: " << mFileName << '\n'
                                        << "Error: " << mError.what() << '\n'
                                        << logEndl;

        throw;
    } catch (...)
    {
        hg::lo("hg::Utils::runLuaFile") << "Fatal unknown Lua error\n"
                                        << "Filename: " << mFileName << '\n'
                                        << logEndl;

        throw;
    }
}

const PackData& findDependencyPackDataOrThrow(
    const HGAssets&         assets,
    const PackData&         currentPack,
    const sf::base::String& mPackDisambiguator,
    const sf::base::String& mPackName,
    const sf::base::String& mPackAuthor)
{
    const auto& dependencies = currentPack.dependencies;

    // ------------------------------------------------------------------------
    // Check if provided arguments are a dependency of current pack.
    const auto depIt = sf::base::findIf(dependencies.begin(),
                                        dependencies.end(),
                                        [&](const PackDependency& pd)
    {
        return pd.disambiguator == mPackDisambiguator && //
               pd.name == mPackName &&                   //
               pd.author == mPackAuthor;
    });

    if (depIt == dependencies.end())
    {
        throw std::runtime_error(
            concat("Pack with disambiguator '",
                   mPackDisambiguator,
                   "', name '",
                   mPackName,
                   "', author: '",
                   mPackAuthor,
                   "' is not a dependency of '",
                   currentPack.name,
                   "'\n")
                .cStr());
    }

    // ------------------------------------------------------------------------
    // Find the pack data corresponding to the specified arguments.
    const PackData* const dependencyData = assets.findPackData(mPackDisambiguator, mPackName, mPackAuthor);

    if (dependencyData == nullptr)
    {
        throw std::runtime_error(
            concat("Could not find dependency pack with disambiguator '",
                   mPackDisambiguator,
                   "', name '",
                   mPackName,
                   "', author: '",
                   mPackAuthor,
                   "'\n")
                .cStr());
    }

    if (dependencyData->version < depIt->minVersion)
    {
        throw std::runtime_error(
            concat("Dependency pack with disambiguator '",
                   mPackDisambiguator,
                   "', name '",
                   mPackName,
                   "', author: '",
                   mPackAuthor,
                   "' has version '",
                   dependencyData->version,
                   "' but at least '",
                   depIt->minVersion,
                   "' is required\n")
                .cStr());
    }

    return *dependencyData;
}

[[nodiscard]] static sf::base::String getDependentAssetFilename(
    const char*                 assetSubfolder,
    sf::base::Vector<sf::Path>& execScriptPackPathContext,
    const sf::Path&             currentPackPath,
    const sf::base::String&     mAssetName)
{
    const sf::Path& context = execScriptPackPathContext.empty() ? currentPackPath : execScriptPackPathContext.back();

    return (context / assetSubfolder / mAssetName.cStr()).to<sf::base::String>();
}

static void withDependencyAssetFilename(
    const char*                                                       assetSubfolder,
    const sf::base::FixedFunction<void(const sf::base::String&), 64>& f,
    sf::base::Vector<sf::Path>&                                       execScriptPackPathContext,
    HGAssets&                                                         assets,
    const PackData&                                                   currentPack,
    const sf::base::String&                                           mPackDisambiguator,
    const sf::base::String&                                           mPackName,
    const sf::base::String&                                           mPackAuthor,
    const sf::base::String&                                           mAssetName)
try
{
    const PackData& dependencyData = findDependencyPackDataOrThrow(assets, currentPack, mPackDisambiguator, mPackName, mPackAuthor);

    execScriptPackPathContext.emplaceBack(dependencyData.folderPath);
    SFML_BASE_SCOPE_GUARD({ execScriptPackPathContext.popBack(); });

    return f((dependencyData.folderPath / assetSubfolder / mAssetName.cStr()).to<sf::base::String>());
} catch (const std::runtime_error& err)
{
    hg::lo("hg::Utils::withDependencyAssetFilename")
        << "Fatal error while looking for Lua dependency\nError: " << err.what() << logEndl;

    throw;
} catch (...)
{
    hg::lo("hg::Utils::withDependencyAssetFilename") << "Fatal unknown error while looking for Lua dependency" << logEndl;

    throw;
}

void withDependencyScriptFilename(
    const sf::base::FixedFunction<void(const sf::base::String&), 64>& f,
    sf::base::Vector<sf::Path>&                                       execScriptPackPathContext,
    HGAssets&                                                         assets,
    const PackData&                                                   currentPack,
    const sf::base::String&                                           mPackDisambiguator,
    const sf::base::String&                                           mPackName,
    const sf::base::String&                                           mPackAuthor,
    const sf::base::String&                                           mScriptName)
{
    withDependencyAssetFilename("Scripts",
                                f,
                                execScriptPackPathContext,
                                assets,
                                currentPack,
                                mPackDisambiguator,
                                mPackName,
                                mPackAuthor,
                                mScriptName);
}

[[nodiscard]] sf::base::String getDependentScriptFilename(sf::base::Vector<sf::Path>& execScriptPackPathContext,
                                                          const sf::Path&             currentPackPath,
                                                          const sf::base::String&     mScriptName)
{
    return getDependentAssetFilename("Scripts", execScriptPackPathContext, currentPackPath, mScriptName);
}

void withDependencyShaderFilename(
    const sf::base::FixedFunction<void(const sf::base::String&), 64>& f,
    sf::base::Vector<sf::Path>&                                       execScriptPackPathContext,
    HGAssets&                                                         assets,
    const PackData&                                                   currentPack,
    const sf::base::String&                                           mPackDisambiguator,
    const sf::base::String&                                           mPackName,
    const sf::base::String&                                           mPackAuthor,
    const sf::base::String&                                           mShaderName)
{
    withDependencyAssetFilename("Shaders",
                                f,
                                execScriptPackPathContext,
                                assets,
                                currentPack,
                                mPackDisambiguator,
                                mPackName,
                                mPackAuthor,
                                mShaderName);
}

[[nodiscard]] sf::base::String getDependentShaderFilename(sf::base::Vector<sf::Path>& execScriptPackPathContext,
                                                          const sf::Path&             currentPackPath,
                                                          const sf::base::String&     mShaderName)
{
    return getDependentAssetFilename("Shaders", execScriptPackPathContext, currentPackPath, mShaderName);
}

template <typename T, typename... TArgs>
T runLuaFunction(Lua::LuaContext& mLua, sf::base::StringView mName, const TArgs&... mArgs)
{
    return mLua.callLuaFunction<T>(mName, std::make_tuple(mArgs...));
}

template <typename T, typename... TArgs>
sf::base::Optional<VoidToNothing<T>> runLuaFunctionIfExists(Lua::LuaContext& mLua, sf::base::StringView mName, const TArgs&... mArgs)
{
    using Ret = sf::base::Optional<VoidToNothing<T>>;

    if (!mLua.doesVariableExist(mName))
    {
        return Ret{};
    }

    if constexpr (isSameType<T, void>)
    {
        runLuaFunction<T>(mLua, mName, mArgs...);
        return Ret{Nothing{}};
    }
    else
    {
        return Ret{runLuaFunction<T>(mLua, mName, mArgs...)};
    }
}

template void runLuaFunction<void>(Lua::LuaContext&, sf::base::StringView);

template sf::base::Optional<VoidToNothing<void>> runLuaFunctionIfExists<void>(Lua::LuaContext&, sf::base::StringView);

template sf::base::Optional<VoidToNothing<float>> runLuaFunctionIfExists<float, float>(
    Lua::LuaContext&,
    sf::base::StringView,
    const float&);

template sf::base::Optional<VoidToNothing<int>> runLuaFunctionIfExists<int, float, float>(
    Lua::LuaContext&,
    sf::base::StringView,
    const float&,
    const float&);

template sf::base::Optional<VoidToNothing<bool>> runLuaFunctionIfExists<bool, float, int, bool, bool>(
    Lua::LuaContext&,
    sf::base::StringView,
    const float&,
    const int&,
    const bool&,
    const bool&);

} // namespace hg::Utils
