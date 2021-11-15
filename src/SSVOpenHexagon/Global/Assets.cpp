// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"

#include "SSVOpenHexagon/Core/Steam.hpp"

#include "SSVOpenHexagon/Data/MusicData.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/AssetStorage.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include "SSVOpenHexagon/Utils/BuildPackId.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/EraseIf.hpp"
#include "SSVOpenHexagon/Utils/LoadFromJson.hpp"
#include "SSVOpenHexagon/Utils/UniquePtr.hpp"

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include <chrono>

namespace hg {

static void loadAssetsFromJson(AssetStorage& assetStorage,
    const ssvu::FileSystem::Path& mRootPath, const ssvuj::Obj& mObj)
{
    for(const auto& f : ssvuj::getExtr<std::vector<std::string>>(mObj, "fonts"))
    {
        if(!assetStorage.loadFont(f, mRootPath + f))
        {
            ssvu::lo("hg::loadAssetsFromJson")
                << "Failed to load font '" << f << "'\n";
        }
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "textures"))
    {
        if(!assetStorage.loadTexture(f, mRootPath + f))
        {
            ssvu::lo("hg::loadAssetsFromJson")
                << "Failed to load texture '" << f << "'\n";
        }
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "soundBuffers"))
    {
        if(!assetStorage.loadSoundBuffer(f, mRootPath + f))
        {
            ssvu::lo("hg::loadAssetsFromJson")
                << "Failed to load sound buffer '" << f << "'\n";
        }
    }
}

[[nodiscard]] static std::vector<ssvufs::Path>& getScanBuffer()
{
    static std::vector<ssvufs::Path> buffer;
    return buffer;
}

[[nodiscard]] static const std::vector<ssvufs::Path>& scanSingleByExt(
    const ssvufs::Path& path, const std::string& extension)
{
    std::vector<ssvufs::Path>& buffer = getScanBuffer();
    buffer.clear();

    ssvufs::scan<ssvufs::Mode::Single, ssvufs::Type::File, ssvufs::Pick::ByExt>(
        buffer, path, extension);

    return buffer;
}

[[nodiscard]] static const std::vector<ssvufs::Path>& scanSingleByName(
    const ssvufs::Path& path, const std::string& name)
{
    std::vector<ssvufs::Path>& buffer = getScanBuffer();
    buffer.clear();

    ssvufs::scan<ssvufs::Mode::Single, ssvufs::Type::File,
        ssvufs::Pick::ByName>(buffer, path, name);

    return buffer;
}

[[nodiscard]] static const std::vector<ssvufs::Path>& scanSingleFolderName(
    const ssvufs::Path& path)
{
    std::vector<ssvufs::Path>& buffer = getScanBuffer();
    buffer.clear();

    ssvufs::scan<ssvufs::Mode::Single, ssvufs::Type::Folder>(buffer, path);

    return buffer;
}

template <typename... Ts>
[[nodiscard]] std::string& HGAssets::concatIntoBuf(const Ts&... xs)
{
    buf.clear();
    Utils::concatInto(buf, xs...);
    return buf;
}

HGAssets::HGAssets(
    Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly)
    : steamManager{mSteamManager},
      levelsOnly{mLevelsOnly},
      assetStorage{Utils::makeUnique<AssetStorage>()}
{
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    const TimePoint tpBeforeLoad = Clock::now();

    if(!levelsOnly && !mHeadless)
    {
        if(!ssvufs::Path{"Assets/"}.exists<ssvufs::Type::Folder>())
        {
            ssvu::lo("FATAL ERROR")
                << "Folder Assets/ does not exist" << std::endl;

            std::terminate();
            return;
        }

        auto [object, error] =
            ssvuj::getFromFileWithErrors("Assets/assets.json");

        loadAssetsFromJson(*assetStorage, "Assets/", object);

        loadInfo.addFormattedError(error);
    }

    if(!loadAllPackDatas())
    {
        ssvu::lo("HGAssets::HGAssets") << "Error loading all pack datas\n";
        std::terminate();
        return;
    }

    if(!loadAllPackAssets())
    {
        ssvu::lo("HGAssets::HGAssets") << "Error loading all pack assets\n";
        std::terminate();
        return;
    }

    if(!verifyAllPackDependencies())
    {
        ssvu::lo("HGAssets::HGAssets") << "Error verifying pack dependencies\n";
        std::terminate();
        return;
    }

    if(!loadAllLocalProfiles())
    {
        ssvu::lo("HGAssets::HGAssets") << "Error loading local profiles\n";
        // No need to terminate here, some tests do not require profiles.
        return;
    }

    for(auto& v : levelDataIdsByPack)
    {
        std::sort(v.second.begin(), v.second.end(),
            [&](const std::string& mA, const std::string& mB) {
                return levelDatas.at(mA).menuPriority <
                       levelDatas.at(mB).menuPriority;
            });
    }

    std::sort(packInfos.begin(), packInfos.end(),
        [&](const PackInfo& mA, const PackInfo& mB)
        { return getPackData(mA.id).priority < getPackData(mB.id).priority; });

    std::sort(selectablePackInfos.begin(), selectablePackInfos.end(),
        [&](const PackInfo& mA, const PackInfo& mB)
        { return getPackData(mA.id).priority < getPackData(mB.id).priority; });

    // This will not be used for the rest of the game,
    // so shrink it to fit the actually used size.
    loadInfo.errorMessages.shrink_to_fit();

    const std::chrono::duration durElapsed = Clock::now() - tpBeforeLoad;

    ssvu::lo("HGAssets::HGAssets")
        << "Loaded all assets in "
        << std::chrono::duration_cast<std::chrono::milliseconds>(durElapsed)
               .count()
        << "ms\n";
}

HGAssets::~HGAssets()
{
    ssvu::lo("HGAssets::~HGAssets") << "Cleaning up assets...\n";
}

[[nodiscard]] bool HGAssets::loadPackData(const ssvufs::Path& packPath)
{
    if(!ssvufs::Path{packPath + "/pack.json"}.exists<ssvufs::Type::File>())
    {
        return false;
    }

    auto p = ssvuj::getFromFileWithErrors(packPath + "/pack.json");

    // Workaround of lambda capture of structured binding.
    auto& packRoot = p.first;
    auto& error = p.second;

    loadInfo.addFormattedError(error);

    auto packDisambiguator = ssvuj::getExtr<std::string>(
        packRoot, "disambiguator", "no disambiguator");

    auto packName =
        ssvuj::getExtr<std::string>(packRoot, "name", "unknown name");

    auto packAuthor =
        ssvuj::getExtr<std::string>(packRoot, "author", "unknown author");

    auto packDescription =
        ssvuj::getExtr<std::string>(packRoot, "description", "no description");

    const auto packVersion = ssvuj::getExtr<int>(packRoot, "version", 0);

    const auto packPriority = ssvuj::getExtr<float>(packRoot, "priority", 100);

    const std::string packId = Utils::buildPackId(
        packDisambiguator, packAuthor, packName, packVersion);

    const auto getPackDependencies = [&]
    {
        std::vector<PackDependency> result;

        if(!ssvuj::hasObj(packRoot, "dependencies"))
        {
            return result;
        }

        const ssvuj::Obj& objDependencies =
            ssvuj::getObj(packRoot, "dependencies");

        const auto dependencyCount = ssvuj::getObjSize(objDependencies);
        result.reserve(dependencyCount);

        for(std::size_t i = 0; i < dependencyCount; ++i)
        {
            const ssvuj::Obj& pdRoot = ssvuj::getObj(objDependencies, i);

            result.emplace_back(PackDependency{
                ssvuj::getExtr<std::string>(pdRoot, "disambiguator"),
                ssvuj::getExtr<std::string>(pdRoot, "name"),
                ssvuj::getExtr<std::string>(pdRoot, "author"),
                ssvuj::getExtr<int>(pdRoot, "min_version"),
            });
        }

        return result;
    };

    packInfos.emplace_back(PackInfo{packId, packPath});

    packDatas.emplace(packId, //
        PackData{
            .folderPath{packPath.getStr()},               //
            .id{packId},                                  //
            .disambiguator{std::move(packDisambiguator)}, //
            .name{std::move(packName)},                   //
            .author{std::move(packAuthor)},               //
            .description{std::move(packDescription)},     //
            .version{packVersion},                        //
            .priority{packPriority},                      //
            .dependencies{getPackDependencies()}          //
        });

    return true;
}

[[nodiscard]] bool HGAssets::loadPackAssets(const PackData& packData)
{
    const std::string& packPath{packData.folderPath};
    const std::string& packId{packData.id};

    ssvu::lo("::loadAssets") << "loading '" << packId << "' assets\n";

    try
    {
        if(ssvufs::Path{packPath + "Shaders/"}.exists<ssvufs::Type::Folder>() &&
            !levelsOnly)
        {
            loadPackAssets_loadShaders(packId, packPath);
        }

        if(ssvufs::Path{packPath + "Music/"}.exists<ssvufs::Type::Folder>() &&
            !levelsOnly)
        {
            loadPackAssets_loadMusic(packId, packPath);
            loadPackAssets_loadMusicData(packId, packPath);
        }

        if(ssvufs::Path{packPath + "Styles/"}.exists<ssvufs::Type::Folder>())
        {
            loadPackAssets_loadStyleData(packId, packPath);
        }

        if(ssvufs::Path{packPath + "Levels/"}.exists<ssvufs::Type::Folder>())
        {
            loadPackAssets_loadLevelData(packId, packPath);
        }

        if(!levelsOnly &&
            ssvufs::Path{packPath + "Sounds/"}.exists<ssvufs::Type::Folder>())
        {
            loadPackAssets_loadCustomSounds(packId, packPath);
        }
    }
    catch(const std::runtime_error& mEx)
    {
        const std::string& errorMessage =
            concatIntoBuf("Exception during asset loading: ", mEx.what(), '\n');

        loadInfo.errorMessages.emplace_back("FATAL ERROR, " + errorMessage);
        ssvu::lo("FATAL ERROR") << errorMessage;
        return false;
    }
    catch(...)
    {
        const std::string errorMessage =
            "Exception during asset loading: unknown.\n";

        loadInfo.errorMessages.emplace_back("FATAL ERROR, " + errorMessage);
        ssvu::lo("FATAL ERROR") << errorMessage;
        return false;
    }

    if(packHasLevels(packId))
    {
        selectablePackInfos.emplace_back(PackInfo{packId, packPath});
    }

    return true;
}

//**********************************************
// LOAD

[[nodiscard]] LoadInfo& HGAssets::getLoadResults()
{
    return loadInfo;
}

[[nodiscard]] sf::Texture& HGAssets::getTexture(const std::string& mId)
{
    sf::Texture* ptr = assetStorage->getTexture(mId);
    SSVOH_ASSERT(ptr);

    return *ptr;
}

[[nodiscard]] sf::Texture& HGAssets::getTextureOrNullTexture(
    const std::string& mId)
{
    static sf::Texture nullTexture;
    sf::Texture* ptr = assetStorage->getTexture(mId);

    return ptr ? *ptr : nullTexture;
}

[[nodiscard]] sf::Font& HGAssets::getFont(const std::string& mId)
{
    sf::Font* ptr = assetStorage->getFont(mId);
    SSVOH_ASSERT(ptr);

    return *ptr;
}

[[nodiscard]] sf::Font& HGAssets::getFontOrNullFont(const std::string& mId)
{
    static sf::Font nullFont;
    sf::Font* ptr = assetStorage->getFont(mId);

    return ptr ? *ptr : nullFont;
}

[[nodiscard]] bool HGAssets::isValidLevelId(
    const std::string& mLevelId) const noexcept
{
    return levelDatas.find(mLevelId) != levelDatas.end();
}

[[nodiscard]] const LevelData& HGAssets::getLevelData(
    const std::string& mAssetId) const
{
    SSVOH_ASSERT(isValidLevelId(mAssetId));
    return levelDatas.at(mAssetId);
}

[[nodiscard]] bool HGAssets::packHasLevels(const std::string& mPackId)
{
    return levelDataIdsByPack.count(mPackId) > 0;
}

[[nodiscard]] const std::vector<std::string>& HGAssets::getLevelIdsByPack(
    const std::string& mPackId)
{
    SSVOH_ASSERT(levelDataIdsByPack.count(mPackId) > 0);
    return levelDataIdsByPack.at(mPackId);
}

[[nodiscard]] const std::unordered_map<std::string, PackData>&
HGAssets::getPackDatas()
{
    return packDatas;
}

[[nodiscard]] bool HGAssets::isValidPackId(
    const std::string& mPackId) const noexcept
{
    return packDatas.find(mPackId) != packDatas.end();
}

[[nodiscard]] const PackData& HGAssets::getPackData(const std::string& mPackId)
{
    SSVOH_ASSERT(isValidPackId(mPackId));
    return packDatas.at(mPackId);
}

[[nodiscard]] const std::vector<PackInfo>&
HGAssets::getSelectablePackInfos() const noexcept
{
    return selectablePackInfos;
}

[[nodiscard]] const PackData* HGAssets::findPackData(
    const std::string& mPackDisambiguator, const std::string& mPackName,
    const std::string& mPackAuthor) const noexcept
{
    for(const auto& [packId, packData] : packDatas)
    {
        if(packData.disambiguator == mPackDisambiguator && //
            packData.name == mPackName &&                  //
            packData.author == mPackAuthor)
        {
            return &packData;
        }
    }

    return nullptr;
}

[[nodiscard]] bool HGAssets::loadWorkshopPackDatasFromCache()
{
    if(!ssvufs::Path{"workshopCache.json"}.exists<ssvufs::Type::File>())
    {
        ssvu::lo("::loadAssets") << "Workshop cache file does not exist. No "
                                    "workshop packs to load\n";
        return false;
    }
    auto [cacheObject, cacheError] =
        ssvuj::getFromFileWithErrors("workshopCache.json");

    ssvu::lo("::loadAssets") << "Loading workshop packs from cache\n";
    if(ssvuj::hasObj(cacheObject, "cachedPacks"))
    {
        // Null check
        auto& packValue = ssvuj::getObj(cacheObject, "cachedPacks");
        if(packValue.type() == Json::ValueType::nullValue ||
            packValue.type() != Json::ValueType::arrayValue)
        {
            ssvu::lo("::loadAssets")
                << "Cache array is null. No workshop packs to load\n";
            return false;
        }

        // Empty check
        const auto packArray = ssvuj::getExtr<std::vector<std::string>>(
            cacheObject, "cachedPacks");

        if(packArray.size() <= 0)
        {
            ssvu::lo("::loadAssets")
                << "Cache array is empty. No workshop packs to load\n";
            return false;
        }

        for(const auto& f : packArray)
        {
            // Simply emplace them. We will check them later.
            cachedWorkshopPackIds.emplace(f);
        }
    }
    else
    {
        ssvu::lo("::loadAssets")
            << "[ERROR]: Cannot locate cache array in workshop cache file\n";

        return false;
    }

    loadInfo.addFormattedError(cacheError);
    return true;
}

[[nodiscard]] bool HGAssets::loadAllPackDatas()
{
    if(!ssvufs::Path{"Packs/"}.exists<ssvufs::Type::Folder>())
    {
        ssvu::lo("::loadAssets") << "Folder Packs/ does not exist" << std::endl;
        return false;
    }

    // ------------------------------------------------------------------------
    const auto tryLoadPackFromPath = [&](const auto& packPath)
    {
        if(!loadPackData(packPath))
        {
            const std::string& errorMessage =
                concatIntoBuf("Error loading pack data '",
                    static_cast<const std::string&>(packPath), '\n');

            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets") << errorMessage;
        }
        else
        {
            ++loadInfo.packs;
        }
    };

    // ------------------------------------------------------------------------
    // Load pack datas from `Packs/` folder.
    for(const auto& packPath : scanSingleFolderName("Packs/"))
    {
        tryLoadPackFromPath(packPath);
    }

    // ------------------------------------------------------------------------
    // Load pack datas from Steam workshop.
    if(steamManager != nullptr)
    {
        if(steamManager->is_initialized())
        {
            steamManager->for_workshop_pack_folders(tryLoadPackFromPath);
        }
        else if(loadWorkshopPackDatasFromCache())
        {
            // In the case the Steam API can't be retrieved, look for a cache
            // that contains the paths we need to load
            for(const auto& cachedPath : cachedWorkshopPackIds)
            {
                tryLoadPackFromPath(cachedPath);
            }
        }
    }

    return true;
}

[[nodiscard]] bool HGAssets::loadAllPackAssets()
{
    for(const auto& [packId, packData] : packDatas)
    {
        if(loadPackAssets(packData))
        {
            continue;
        }

        const std::string& errorMessage =
            concatIntoBuf("Error loading pack info '", packId, '\n');

        loadInfo.errorMessages.emplace_back(errorMessage);
        ssvu::lo("::loadAssets") << errorMessage;

        return false;
    }

    return true;
}

[[nodiscard]] bool HGAssets::verifyAllPackDependencies()
{
    // ------------------------------------------------------------------------
    // Verify pack dependencies.
    const auto dependencyExists = [this](const PackDependency& pd)
    {
        for(const auto& [packId, packData] : packDatas)
        {
            if(                                                 //
                (packData.disambiguator == pd.disambiguator) && //
                (packData.name == pd.name) &&                   //
                (packData.author == pd.author) &&               //
                (packData.version >= pd.minVersion))
            {
                return true;
            }
        }

        return false;
    };

    for(const auto& [packId, packData] : packDatas)
    {
        for(const PackDependency& pd : packData.dependencies)
        {
            if(dependencyExists(pd))
            {
                continue;
            }

            const std::string& errorMessage =
                concatIntoBuf("Missing pack dependency '", pd.name,
                    "' for pack '", packData.name, "'\n");

            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets") << errorMessage;

            packIdsWithMissingDependencies.emplace(packId);
        }
    }

    ssvu::eraseRemoveIf(selectablePackInfos, [&](const PackInfo& pi)
        { return packIdsWithMissingDependencies.contains(pi.id); });

    return true;
}

void HGAssets::addLocalProfile(ProfileData&& profileData)
{
    // Remove invalid level ids that might have been added to the files.
    Utils::erase_if(profileData.getFavoriteLevelIds(),
        [this](const std::string& favId)
        { return levelDatas.find(favId) == levelDatas.end(); });

    profileDataMap.emplace(profileData.getName(), std::move(profileData));
}

[[nodiscard]] bool HGAssets::loadAllLocalProfiles()
{
    if(!ssvufs::Path{"Profiles/"}.exists<ssvufs::Type::Folder>())
    {
        ssvu::lo("::loadAssets")
            << "Folder Profiles/ does not exist" << std::endl;

        return false;
    }

    ssvu::lo("::loadAssets") << "loading local profiles\n";

    for(const auto& p : scanSingleByExt("Profiles/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        ProfileData profileData{Utils::loadProfileFromJson(object)};
        addLocalProfile(std::move(profileData));
    }

    return true;
}

void HGAssets::loadPackAssets_loadShaders(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    const auto loadShadersOfType =
        [&](const char* const extension, sf::Shader::Type shaderType)
    {
        for(const auto& p : scanSingleByExt(mPath + "Shaders/", extension))
        {
            auto shader = Utils::makeUnique<sf::Shader>();

            if(!shader->loadFromFile(p, shaderType))
            {
                ssvu::lo("hg::loadPackAssets_loadShaders")
                    << "Failed to load shader '" << p << "'\n";

                continue;
            }

            shadersById.push_back(shader.get());
            SSVOH_ASSERT(shadersById.size() > 0);
            const std::size_t shaderId = shadersById.size() - 1;

            LoadedShader ls{.shader{std::move(shader)},
                .path{p},
                .shaderType{shaderType},
                .id{shaderId}};

            shaders.emplace(
                concatIntoBuf(mPackId, '_', p.getFileName()), std::move(ls));

            shadersPathToId.emplace(p, shaderId);

            ++loadInfo.assets;
        }
    };

    loadShadersOfType(".vert", sf::Shader::Type::Vertex);
    loadShadersOfType(".geom", sf::Shader::Type::Geometry);
    loadShadersOfType(".frag", sf::Shader::Type::Fragment);
}

void HGAssets::loadPackAssets_loadCustomSounds(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Sounds/", ".ogg"))
    {
        if(!assetStorage->loadSoundBuffer(
               concatIntoBuf(mPackId, '_', p.getFileName()), p))
        {
            ssvu::lo("hg::loadPackAssets_loadCustomSounds")
                << "Failed to load sound buffer '" << p << "'\n";
        }

        ++loadInfo.assets;
    }
}

void HGAssets::loadPackAssets_loadMusic(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".ogg"))
    {
        musicPathMap.emplace(
            concatIntoBuf(mPackId, '_', p.getFileNameNoExtensions()), p);

        ++loadInfo.assets;
    }
}

void HGAssets::loadPackAssets_loadMusicData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        MusicData musicData{Utils::loadMusicFromJson(object)};
        musicDataMap.emplace(
            concatIntoBuf(mPackId, '_', musicData.id), std::move(musicData));

        ++loadInfo.assets;
    }
}

void HGAssets::loadPackAssets_loadStyleData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        StyleData styleData{object};
        styleDataMap.emplace(
            concatIntoBuf(mPackId, '_', styleData.id), std::move(styleData));

        ++loadInfo.assets;
    }
}

void HGAssets::loadPackAssets_loadLevelData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Levels/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        LevelData levelData{object, mPath, mPackId};
        const std::string& assetId = concatIntoBuf(mPackId, '_', levelData.id);

        levelDataIdsByPack[mPackId].emplace_back(assetId);
        levelDatas.emplace(std::move(assetId), std::move(levelData));

        ++loadInfo.levels;
    }
}

//**********************************************
// PROFILE

void HGAssets::saveCurrentLocalProfile()
{
    if(currentProfilePtr == nullptr)
    {
        return;
    }

    ssvuj::Obj profileRoot;
    ssvuj::Obj currentVersion;

    ssvuj::arch(currentVersion, "major", GAME_VERSION.major);
    ssvuj::arch(currentVersion, "minor", GAME_VERSION.minor);
    ssvuj::arch(currentVersion, "micro", GAME_VERSION.micro);

    ssvuj::arch(profileRoot, "version", currentVersion);
    ssvuj::arch(profileRoot, "name", getCurrentLocalProfile().getName());
    ssvuj::arch(profileRoot, "scores", getCurrentLocalProfile().getScores());

    const auto& favSet{getCurrentLocalProfile().getFavoriteLevelIds()};
    std::vector<std::string> favorites;
    std::copy(favSet.begin(), favSet.end(), std::back_inserter(favorites));
    ssvuj::arch(profileRoot, "favorites", favorites);

    ssvuj::writeToFile(profileRoot, getCurrentLocalProfileFilePath());
}

void HGAssets::saveAllProfiles()
{
    ssvuj::Obj currentVersion;

    ssvuj::arch(currentVersion, "major", GAME_VERSION.major);
    ssvuj::arch(currentVersion, "minor", GAME_VERSION.minor);
    ssvuj::arch(currentVersion, "micro", GAME_VERSION.micro);

    std::vector<std::string> favorites;

    for(const auto& [key, profileData] : profileDataMap)
    {
        ssvuj::Obj profileRoot;
        ssvuj::arch(profileRoot, "version", currentVersion);
        ssvuj::arch(profileRoot, "name", profileData.getName());
        ssvuj::arch(profileRoot, "scores", profileData.getScores());

        favorites.clear();
        for(const std::string& favID : profileData.getFavoriteLevelIds())
        {
            favorites.emplace_back(favID);
        }

        ssvuj::arch(profileRoot, "favorites", favorites);

        ssvuj::writeToFile(
            profileRoot, "Profiles/" + profileData.getName() + ".json");
    }
}

//**********************************************
// GET

[[nodiscard]] const MusicData& HGAssets::getMusicData(
    const std::string& mPackId, const std::string& mId)
{
    const std::string& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = musicDataMap.find(assetId);
    if(it == musicDataMap.end())
    {
        ssvu::lo("getMusicData") << "Asset '" << assetId << "' not found\n";

        SSVOH_ASSERT(!musicDataMap.empty());
        return musicDataMap.begin()->second;
    }

    return it->second;
}

[[nodiscard]] const StyleData& HGAssets::getStyleData(
    const std::string& mPackId, const std::string& mId)
{
    const std::string& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = styleDataMap.find(assetId);
    if(it == styleDataMap.end())
    {
        ssvu::lo("getStyleData") << "Asset '" << assetId << "' not found\n";

        SSVOH_ASSERT(!styleDataMap.empty());
        return styleDataMap.begin()->second;
    }

    return it->second;
}

[[nodiscard]] sf::Shader* HGAssets::getShader(
    const std::string& mPackId, const std::string& mId)
{
    const std::string& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = shaders.find(assetId);
    if(it == shaders.end())
    {
        ssvu::lo("getShader") << "Asset '" << assetId << "' not found\n";
        return nullptr;
    }

    return it->second.shader.get();
}

[[nodiscard]] std::optional<std::size_t> HGAssets::getShaderId(
    const std::string& mPackId, const std::string& mId)
{
    const std::string& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = shaders.find(assetId);
    if(it == shaders.end())
    {
        ssvu::lo("getShaderId") << "Asset '" << assetId << "' not found\n";
        return std::nullopt;
    }

    return it->second.id;
}

[[nodiscard]] std::optional<std::size_t> HGAssets::getShaderIdByPath(
    const std::string& mShaderPath)
{
    const auto it = shadersPathToId.find(mShaderPath);
    if(it == shadersPathToId.end())
    {
        ssvu::lo("getShaderIdByPath") << "Shader with path '" << mShaderPath
                                      << "' not found, couldn't get id\n";

        return std::nullopt;
    }

    return it->second;
}

[[nodiscard]] sf::Shader* HGAssets::getShaderByShaderId(
    const std::size_t mShaderId)
{
    if(!isValidShaderId(mShaderId))
    {
        return nullptr;
    }

    return shadersById[mShaderId];
}

[[nodiscard]] bool HGAssets::isValidShaderId(const std::size_t mShaderId) const
{
    return mShaderId < shadersById.size();
}

//**********************************************
// RELOAD

void HGAssets::reloadAllShaders()
{
    for(auto& [id, loadedShader] : shaders)
    {
        if(!loadedShader.shader->loadFromFile(
               loadedShader.path, loadedShader.shaderType))
        {
            ssvu::lo("hg::HGAssets::reloadAllShaders")
                << "Failed to load shader '" << loadedShader.path << "'\n";

            continue;
        }
    }
}

[[nodiscard]] std::string HGAssets::reloadPack(
    const std::string& mPackId, const std::string& mPath)
{
    std::string temp, output;

    // Levels, if there is not folder cancel everything
    temp = mPath + "Levels/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        return "invalid level folder path\n";
    }
    for(const auto& p : scanSingleByExt(mPath + "Levels/", ".json"))
    {
        LevelData levelData{ssvuj::getFromFile(p), mPath, mPackId};
        temp = mPackId + "_" + levelData.id;

        auto it = levelDatas.find(temp);
        if(it == levelDatas.end())
        {
            levelDataIdsByPack[mPackId].emplace_back(temp);
            levelDatas.emplace(temp, std::move(levelData));
        }
        else
        {
            it->second = levelData;
        }
    }
    output += "Levels successfully reloaded\n";

    // Styles
    temp = mPath + "Styles/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid style folder path\n";
    }
    else
    {
        for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
        {
            StyleData styleData{ssvuj::getFromFile(p)};
            temp = mPackId + "_" + styleData.id;

            styleDataMap[temp] = std::move(styleData);
        }
        output += "Styles successfully reloaded\n";
    }

    // Music data
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid music data folder path\n";
    }
    else
    {
        for(const auto& p : scanSingleByExt(mPath + "Music/", ".json"))
        {
            MusicData musicData{
                Utils::loadMusicFromJson(ssvuj::getFromFile(p))};
            temp = mPackId + "_" + musicData.id;

            musicDataMap[temp] = std::move(musicData);
        }
        output += "Music data successfully reloaded\n";
    }

    // Music
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid music folder path\n";
    }
    else
    {
        for(const auto& p : scanSingleByExt(mPath + "Music/", ".ogg"))
        {
            temp = mPackId + "_" + p.getFileNameNoExtensions();
            musicPathMap.emplace(temp, p);
        }
        output += "Music files successfully reloaded\n";
    }

    // Custom sounds
    temp = mPath + "Sounds/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid custom sound folder path\n";
    }
    else
    {
        for(const auto& p : scanSingleByExt(mPath + "Sounds/", ".ogg"))
        {
            temp = mPackId + "_" + p.getFileName();
            if(!assetStorage->loadSoundBuffer(temp, p))
            {
                output += "Failed to load sound buffer '";
                output += p;
                output += "'\n";
            }
        }
        output += "Custom sound files successfully reloaded\n";
    }

    return output;
}

[[nodiscard]] std::string HGAssets::reloadLevel(const std::string& mPackId,
    const std::string& mPath, const std::string& mId)
{
    std::string temp, output;

    //*******************************************
    // Level
    temp = mPath + "Levels/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        return "invalid level folder path\n";
    }

    auto levelFile = scanSingleByName(temp, mId + ".json");
    if(levelFile.empty())
    {
        return "no matching level data file found\n";
    }

    // There is only one file, so we can just subscript index 0.
    // Same goes for all other files below
    LevelData levelData{ssvuj::getFromFile(levelFile[0]), mPath, mPackId};
    temp = mPackId + "_" + mId;

    auto it = levelDatas.find(temp);
    if(it == levelDatas.end())
    {
        levelDataIdsByPack[mPackId].emplace_back(temp);
        levelDatas.emplace(temp, std::move(levelData));
    }
    else
    {
        it->second = levelData;
    }
    output = "level data " + mId + ".json successfully loaded\n";

    //*******************************************
    // Style
    temp = mPath + "Styles/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid style folder path\n";
    }
    else
    {
        auto styleFile = scanSingleByName(temp, levelData.styleId + ".json");
        if(styleFile.empty())
        {
            output += "no matching style file found\n";
        }
        else
        {
            StyleData styleData{ssvuj::getFromFile(styleFile[0])};
            temp = mPackId + "_" + levelData.styleId;

            styleDataMap[temp] = std::move(styleData);

            output += "style data " + levelData.styleId +
                      ".json successfully loaded\n";
        }
    }

    //*******************************************
    // Music data
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid music folder path\n";
    }
    else
    {
        auto musicDataFile =
            scanSingleByName(temp, levelData.musicId + ".json");
        if(musicDataFile.empty())
        {
            output += "no matching music data file found\n";
        }
        else
        {
            MusicData musicData{
                Utils::loadMusicFromJson(ssvuj::getFromFile(musicDataFile[0]))};
            temp = mPackId + "_" + levelData.musicId;

            musicDataMap[temp] = std::move(musicData);

            output += "music data " + levelData.musicId +
                      ".json successfully loaded\n";
        }
    }

    //*******************************************
    // Music files
    std::string assetId;
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid music folder path\n";
    }
    else if(levelData.musicId != "nullMusicId")
    {
        assetId = mPackId + "_" + levelData.musicId;

        auto musicFile = scanSingleByName(temp, levelData.musicId + ".ogg");
        if(musicFile.empty())
        {
            output += "no matching music file found\n";
        }
        else
        {
            musicPathMap.emplace(assetId, musicFile[0]);
        }
    }

    //*******************************************
    // Sound files
    if(levelData.soundId == "nullSoundId")
    {
        // no need to keep going if sound id is null
        return output;
    }

    temp = mPath + "Sounds/";
    if(!ssvufs::Path{temp}.exists<ssvufs::Type::Folder>())
    {
        output += "invalid custom sound folder path\n";
        return output;
    }

    // Check if this custom sound file is already loaded
    assetId = mPackId + "_" + levelData.soundId;
    if(assetStorage->hasSoundBuffer(assetId))
    {
        output += "custom sound file ";
        output += levelData.soundId;
        output += ".ogg is already loaded\n";

        return output;
    }

    auto soundFile = scanSingleByName(temp, levelData.soundId + ".ogg");
    if(soundFile.empty())
    {
        output += "no matching custom sound file found\n";
        return output;
    }

    if(!assetStorage->loadSoundBuffer(assetId, soundFile[0]))
    {
        output += "Failed to load sound buffer '";
        output += soundFile[0];
        output += "'\n";

        return output;
    }

    output += "new custom sound file ";
    output += levelData.soundId;
    output += ".ogg successfully loaded\n";

    return output;
}

//**********************************************
// LOCAL SCORE

float HGAssets::getLocalScore(const std::string& mId)
{
    return getCurrentLocalProfile().getScore(mId);
}

void HGAssets::setLocalScore(const std::string& mId, float mScore)
{
    getCurrentLocalProfile().setScore(mId, mScore);
}

//**********************************************
// LOCAL PROFILE

[[nodiscard]] bool HGAssets::anyLocalProfileActive() const
{
    return currentProfilePtr != nullptr;
}

ProfileData& HGAssets::getCurrentLocalProfile()
{
    SSVOH_ASSERT(currentProfilePtr != nullptr);
    return *currentProfilePtr;
}

ProfileData* HGAssets::getLocalProfileByName(const std::string& mName)
{
    SSVOH_ASSERT(profileDataMap.contains(mName));
    return &profileDataMap.find(mName)->second;
}

const ProfileData& HGAssets::getCurrentLocalProfile() const
{
    SSVOH_ASSERT(currentProfilePtr != nullptr);
    return *currentProfilePtr;
}

const ProfileData* HGAssets::getLocalProfileByName(
    const std::string& mName) const
{
    SSVOH_ASSERT(profileDataMap.contains(mName));
    return &profileDataMap.find(mName)->second;
}

[[nodiscard]] std::string HGAssets::getCurrentLocalProfileFilePath()
{
    return "Profiles/" + currentProfilePtr->getName() + ".json";
}


[[nodiscard]] std::size_t HGAssets::getLocalProfilesSize()
{
    return profileDataMap.size();
}

[[nodiscard]] std::vector<std::string> HGAssets::getLocalProfileNames()
{
    std::vector<std::string> result;
    for(auto& pair : profileDataMap)
    {
        result.emplace_back(pair.second.getName());
    }
    return result;
}


[[nodiscard]] bool HGAssets::pIsValidLocalProfile() const
{
    return currentProfilePtr != nullptr;
}

[[nodiscard]] const std::string& HGAssets::pGetName() const
{
    return getCurrentLocalProfile().getName();
}

void HGAssets::pSaveCurrent()
{
    saveCurrentLocalProfile();
}

void HGAssets::pSaveAll()
{
    saveAllProfiles();
}

void HGAssets::pSetCurrent(const std::string& mName)
{
    const auto it = profileDataMap.find(mName);

    SSVOH_ASSERT(it != profileDataMap.end());
    currentProfilePtr = &it->second;
}

void HGAssets::pCreate(const std::string& mName)
{
    ssvuj::Obj root;
    ssvuj::arch(root, "name", mName);
    ssvuj::arch(root, "scores", ssvuj::Obj{});
    ssvuj::arch(root, "favorites", ssvuj::Obj{});
    ssvuj::writeToFile(root, "Profiles/" + mName + ".json");

    profileDataMap.clear();

    if(!loadAllLocalProfiles())
    {
        ssvu::lo("HGAssets::HGAssets") << "Error loading local profiles\n";
        std::terminate();
        return;
    }
}

void HGAssets::pRemove(const std::string& mName)
{
    profileDataMap.erase(mName);
}

[[nodiscard]] sf::SoundBuffer* HGAssets::getSoundBuffer(
    const std::string& assetId)
{
    return assetStorage->getSoundBuffer(assetId);
}

[[nodiscard]] const std::string* HGAssets::getMusicPath(
    const std::string& assetId) const
{
    auto it = musicPathMap.find(assetId);
    return it == musicPathMap.end() ? nullptr : &it->second;
}

[[nodiscard]] const std::unordered_map<std::string, LevelData>&
HGAssets::getLevelDatas() const noexcept
{
    return levelDatas;
}

[[nodiscard]] const std::unordered_set<std::string>&
HGAssets::getPackIdsWithMissingDependencies() const noexcept
{
    return packIdsWithMissingDependencies;
}

} // namespace hg
