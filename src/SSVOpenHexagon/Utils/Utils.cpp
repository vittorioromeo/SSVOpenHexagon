// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Utils.hpp"

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/ScopeGuard.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"

#include <SSVStart/Camera/Camera.hpp>

#include <SSVUtils/Timeline/Timeline.hpp>
#include <SSVUtils/Core/Log/Log.hpp>

#include <SFML/System/Vector2.hpp>

#include <string>
#include <fstream>
#include <stdexcept>

namespace hg::Utils {

void runLuaCode(Lua::LuaContext& mLua, const std::string& mCode)
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

void runLuaFile(Lua::LuaContext& mLua, const std::string& mFileName)
{
    std::ifstream s{mFileName};

    if(!s)
    {
        const std::string errorStr = concat(
            "Fatal Lua error\n", "Could not open file: ", mFileName, '\n');

        ssvu::lo("hg::Utils::runLuaFile") << errorStr << std::endl;
        throw std::runtime_error(errorStr);
    }

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

const PackData& findDependencyPackDataOrThrow(const HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor)
{
    const auto& dependencies = currentPack.dependencies;

    // ------------------------------------------------------------------------
    // Check if provided arguments are a dependency of current pack.
    const auto depIt = std::find_if(dependencies.begin(), dependencies.end(),
        [&](const PackDependency& pd)
        {
            return pd.disambiguator == mPackDisambiguator && //
                   pd.name == mPackName &&                   //
                   pd.author == mPackAuthor;
        });

    if(depIt == dependencies.end())
    {
        throw std::runtime_error(
            concat("Pack with disambiguator '", mPackDisambiguator, "', name '",
                mPackName, "', author: '", mPackAuthor,
                "' is not a dependency of '", currentPack.name, "'\n"));
    }

    // ------------------------------------------------------------------------
    // Find the pack data corresponding to the specified arguments.
    const PackData* const dependencyData =
        assets.findPackData(mPackDisambiguator, mPackName, mPackAuthor);

    if(dependencyData == nullptr)
    {
        throw std::runtime_error(
            concat("Could not find dependency pack with disambiguator '",
                mPackDisambiguator, "', name '", mPackName, "', author: '",
                mPackAuthor, "'\n"));
    }

    if(dependencyData->version < depIt->minVersion)
    {
        throw std::runtime_error(concat("Dependency pack with disambiguator '",
            mPackDisambiguator, "', name '", mPackName, "', author: '",
            mPackAuthor, "' has version '", dependencyData->version,
            "' but at least '", depIt->minVersion, "' is required\n"));
    }

    return *dependencyData;
}

[[nodiscard]] static std::string getDependentAssetFilename(
    const char* assetSubfolder,
    std::vector<std::string>& execScriptPackPathContext,
    const std::string& currentPackPath, const std::string& mAssetName)
{
    const std::string& context = execScriptPackPathContext.empty()
                                     ? currentPackPath
                                     : execScriptPackPathContext.back();

    return concat(context, assetSubfolder, '/', mAssetName);
}

static void withDependencyAssetFilename(const char* assetSubfolder,
    const std::function<void(const std::string&)> f,
    std::vector<std::string>& execScriptPackPathContext, HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor,
    const std::string& mAssetName)
try
{
    const PackData& dependencyData = findDependencyPackDataOrThrow(
        assets, currentPack, mPackDisambiguator, mPackName, mPackAuthor);

    execScriptPackPathContext.emplace_back(dependencyData.folderPath);
    HG_SCOPE_GUARD({ execScriptPackPathContext.pop_back(); });

    return f(
        concat(dependencyData.folderPath, assetSubfolder, '/', mAssetName));
}
catch(const std::runtime_error& err)
{
    ssvu::lo("hg::Utils::withDependencyAssetFilename")
        << "Fatal error while looking for Lua dependency\nError: " << err.what()
        << std::endl;

    throw;
}
catch(...)
{
    ssvu::lo("hg::Utils::withDependencyAssetFilename")
        << "Fatal unknown error while looking for Lua dependency" << std::endl;

    throw;
}

void withDependencyScriptFilename(
    const std::function<void(const std::string&)> f,
    std::vector<std::string>& execScriptPackPathContext, HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor,
    const std::string& mScriptName)
{
    withDependencyAssetFilename("Scripts", f, execScriptPackPathContext, assets,
        currentPack, mPackDisambiguator, mPackName, mPackAuthor, mScriptName);
}

[[nodiscard]] std::string getDependentScriptFilename(
    std::vector<std::string>& execScriptPackPathContext,
    const std::string& currentPackPath, const std::string& mScriptName)
{
    return getDependentAssetFilename(
        "Scripts", execScriptPackPathContext, currentPackPath, mScriptName);
}

void withDependencyShaderFilename(
    const std::function<void(const std::string&)> f,
    std::vector<std::string>& execScriptPackPathContext, HGAssets& assets,
    const PackData& currentPack, const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor,
    const std::string& mShaderName)
{
    withDependencyAssetFilename("Shaders", f, execScriptPackPathContext, assets,
        currentPack, mPackDisambiguator, mPackName, mPackAuthor, mShaderName);
}

[[nodiscard]] std::string getDependentShaderFilename(
    std::vector<std::string>& execScriptPackPathContext,
    const std::string& currentPackPath, const std::string& mShaderName)
{
    return getDependentAssetFilename(
        "Shaders", execScriptPackPathContext, currentPackPath, mShaderName);
}

} // namespace hg::Utils
