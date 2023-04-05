// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"

#include "SSVOpenHexagon/Core/Steam.hpp"

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/LoadInfo.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/PackInfo.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/AssetStorage.hpp"
#include "SSVOpenHexagon/Global/Macros.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include "SSVOpenHexagon/Utils/BuildPackId.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"
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

class HGAssets::HGAssetsImpl
{
private:
    Steam::steam_manager* steamManager;

    bool levelsOnly{false};

    Utils::UniquePtr<AssetStorage> assetStorage;

    std::unordered_map<std::string, LevelData> levelDatas;
    std::unordered_map<std::string, std::vector<std::string>>
        levelDataIdsByPack;

    std::unordered_map<std::string, PackData> packDatas;

    std::vector<PackInfo> packInfos;
    std::vector<PackInfo> selectablePackInfos;

    std::unordered_map<std::string, std::string> musicPathMap;
    std::map<std::string, MusicData> musicDataMap;
    std::map<std::string, StyleData> styleDataMap;
    std::map<std::string, ProfileData> profileDataMap;
    ProfileData* currentProfilePtr{nullptr};

    std::unordered_set<std::string> packIdsWithMissingDependencies;

    struct LoadedShader
    {
        Utils::UniquePtr<sf::Shader> shader;
        std::string path;
        sf::Shader::Type shaderType;
        std::size_t id;
    };

    std::unordered_map<std::string, LoadedShader> shaders;
    std::unordered_map<std::string, std::size_t> shadersPathToId;
    std::vector<sf::Shader*> shadersById;

    std::string buf;

    std::unordered_map<std::string, std::string> luaFileCache;
    LoadInfo loadInfo;

    // When the Steam API can not be retrieved, this set holds pack ids
    // retrieved from the cache to try and load the workshop packs installed
    std::unordered_set<std::string> cachedWorkshopPackIds;

    template <typename... Ts>
    [[nodiscard]] std::string& concatIntoBuf(const Ts&...);

    [[nodiscard]] bool loadAllPackDatas();
    [[nodiscard]] bool loadAllPackAssets(const bool headless);
    [[nodiscard]] bool loadWorkshopPackDatasFromCache();
    [[nodiscard]] bool verifyAllPackDependencies();
    [[nodiscard]] bool loadAllLocalProfiles();

    [[nodiscard]] bool loadPackData(const ssvufs::Path& packPath);

    [[nodiscard]] bool loadPackAssets(
        const PackData& packData, const bool headless);

    void loadPackAssets_loadShaders(const std::string& mPackId,
        const ssvufs::Path& mPath, const bool headless);
    void loadPackAssets_loadMusic(
        const std::string& mPackId, const ssvufs::Path& mPath);
    void loadPackAssets_loadMusicData(
        const std::string& mPackId, const ssvufs::Path& mPath);
    void loadPackAssets_loadStyleData(
        const std::string& mPackId, const ssvufs::Path& mPath);
    void loadPackAssets_loadLevelData(
        const std::string& mPackId, const ssvufs::Path& mPath);
    void loadPackAssets_loadCustomSounds(
        const std::string& mPackId, const ssvufs::Path& mPath);

    [[nodiscard]] std::string getCurrentLocalProfileFilePath();

public:
    HGAssetsImpl(Steam::steam_manager* mSteamManager, bool mHeadless,
        bool mLevelsOnly = false);

    ~HGAssetsImpl();

    [[nodiscard]] LoadInfo& getLoadResults();

    [[nodiscard]] sf::Texture& getTexture(const std::string& mId);
    [[nodiscard]] sf::Texture& getTextureOrNullTexture(const std::string& mId);

    [[nodiscard]] sf::Font& getFont(const std::string& mId);
    [[nodiscard]] sf::Font& getFontOrNullFont(const std::string& mId);

    [[nodiscard]] bool isValidLevelId(
        const std::string& mLevelId) const noexcept;

    [[nodiscard]] const LevelData& getLevelData(
        const std::string& mAssetId) const;

    [[nodiscard]] bool packHasLevels(const std::string& mPackId);

    [[nodiscard]] const std::vector<std::string>& getLevelIdsByPack(
        const std::string& mPackId);

    [[nodiscard]] const std::unordered_map<std::string, PackData>&
    getPackDatas();

    [[nodiscard]] bool isValidPackId(const std::string& mPackId) const noexcept;

    [[nodiscard]] const PackData& getPackData(const std::string& mPackId);

    [[nodiscard]] const std::vector<PackInfo>&
    getSelectablePackInfos() const noexcept;

    [[nodiscard]] const PackData* findPackData(
        const std::string& mPackDisambiguator, const std::string& mPackName,
        const std::string& mPackAuthor) const noexcept;

    [[nodiscard]] const MusicData& getMusicData(
        const std::string& mPackId, const std::string& mId);
    [[nodiscard]] const StyleData& getStyleData(
        const std::string& mPackId, const std::string& mId);
    [[nodiscard]] sf::Shader* getShader(
        const std::string& mPackId, const std::string& mId);

    [[nodiscard]] std::optional<std::size_t> getShaderId(
        const std::string& mPackId, const std::string& mId);
    [[nodiscard]] std::optional<std::size_t> getShaderIdByPath(
        const std::string& mShaderPath);
    [[nodiscard]] sf::Shader* getShaderByShaderId(const std::size_t mShaderId);
    [[nodiscard]] bool isValidShaderId(const std::size_t mShaderId) const;

    void reloadAllShaders();
    [[nodiscard]] std::string reloadPack(
        const std::string& mPackId, const std::string& mPath);
    [[nodiscard]] std::string reloadLevel(const std::string& mPackId,
        const std::string& mPath, const std::string& mId);

    [[nodiscard]] float getLocalScore(const std::string& mId);
    void setLocalScore(const std::string& mId, float mScore);

    void saveCurrentLocalProfile();
    void saveAllProfiles();

    [[nodiscard]] bool anyLocalProfileActive() const;
    [[nodiscard]] ProfileData& getCurrentLocalProfile();
    [[nodiscard]] const ProfileData& getCurrentLocalProfile() const;
    [[nodiscard]] ProfileData* getLocalProfileByName(const std::string& mName);
    [[nodiscard]] const ProfileData* getLocalProfileByName(
        const std::string& mName) const;
    [[nodiscard]] std::size_t getLocalProfilesSize();
    [[nodiscard]] std::vector<std::string> getLocalProfileNames();

    [[nodiscard]] bool pIsValidLocalProfile() const;
    [[nodiscard]] const std::string& pGetName() const;

    void pSaveCurrent();
    void pSaveAll();
    void pSetCurrent(const std::string& mName);
    void pCreate(const std::string& mName);
    void pRemove(const std::string& mName);

    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(const std::string& assetId);

    [[nodiscard]] const std::string* getMusicPath(
        const std::string& assetId) const;

    [[nodiscard]] const std::unordered_map<std::string, LevelData>&
    getLevelDatas() const noexcept;

    [[nodiscard]] const std::unordered_set<std::string>&
    getPackIdsWithMissingDependencies() const noexcept;

    void addLocalProfile(ProfileData&& profileData);

    [[nodiscard]] std::unordered_map<std::string, std::string>&
    getLuaFileCache();

    [[nodiscard]] const std::unordered_map<std::string, std::string>&
    getLuaFileCache() const;
};

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
[[nodiscard]] std::string& HGAssets::HGAssetsImpl::concatIntoBuf(
    const Ts&... xs)
{
    buf.clear();
    Utils::concatInto(buf, xs...);
    return buf;
}

HGAssets::HGAssetsImpl::HGAssetsImpl(
    Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly)
    : steamManager{mSteamManager},
      levelsOnly{mLevelsOnly},
      assetStorage{Utils::makeUnique<AssetStorage>()}
{
    const HRTimePoint tpBeforeLoad = HRClock::now();

    if(!levelsOnly && !mHeadless)
    {
        if(!ssvufs::Path{"Assets/"}.isFolder())
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

    if(!loadAllPackAssets(mHeadless))
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

    const std::chrono::duration durElapsed = HRClock::now() - tpBeforeLoad;

    ssvu::lo("HGAssets::HGAssets")
        << "Loaded all assets in "
        << std::chrono::duration_cast<std::chrono::milliseconds>(durElapsed)
               .count()
        << "ms\n";
}

HGAssets::HGAssetsImpl::~HGAssetsImpl()
{
    ssvu::lo("HGAssets::~HGAssets") << "Cleaning up assets...\n";
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadPackData(
    const ssvufs::Path& packPath)
{
    if(!ssvufs::Path{packPath + "/pack.json"}.isFile())
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

    packDatas.emplace(packId,                              //
        PackData{
            .folderPath{packPath.getStr()},                //
            .id{packId},                                   //
            .disambiguator{SSVOH_MOVE(packDisambiguator)}, //
            .name{SSVOH_MOVE(packName)},                   //
            .author{SSVOH_MOVE(packAuthor)},               //
            .description{SSVOH_MOVE(packDescription)},     //
            .version{packVersion},                         //
            .priority{packPriority},                       //
            .dependencies{getPackDependencies()}           //
        });

    return true;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadPackAssets(
    const PackData& packData, const bool headless)
{
    const std::string& packPath{packData.folderPath};
    const std::string& packId{packData.id};

    ssvu::lo("::loadAssets") << "loading '" << packId << "' assets\n";

    try
    {
        if(!headless)
        {
            if(ssvufs::Path{packPath + "Shaders/"}.isFolder() && !levelsOnly)
            {
                loadPackAssets_loadShaders(packId, packPath, headless);
            }

            if(!levelsOnly && ssvufs::Path{packPath + "Sounds/"}.isFolder())
            {
                loadPackAssets_loadCustomSounds(packId, packPath);
            }
        }

        if(ssvufs::Path{packPath + "Music/"}.isFolder() && !levelsOnly)
        {
            if(!headless)
            {
                loadPackAssets_loadMusic(packId, packPath);
            }

            loadPackAssets_loadMusicData(packId, packPath);
        }

        if(ssvufs::Path{packPath + "Styles/"}.isFolder())
        {
            loadPackAssets_loadStyleData(packId, packPath);
        }

        if(ssvufs::Path{packPath + "Levels/"}.isFolder())
        {
            loadPackAssets_loadLevelData(packId, packPath);
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

[[nodiscard]] LoadInfo& HGAssets::HGAssetsImpl::getLoadResults()
{
    return loadInfo;
}

[[nodiscard]] sf::Texture& HGAssets::HGAssetsImpl::getTexture(
    const std::string& mId)
{
    sf::Texture* ptr = assetStorage->getTexture(mId);
    SSVOH_ASSERT(ptr);

    return *ptr;
}

[[nodiscard]] sf::Texture& HGAssets::HGAssetsImpl::getTextureOrNullTexture(
    const std::string& mId)
{
    static sf::Texture nullTexture;
    sf::Texture* ptr = assetStorage->getTexture(mId);

    return ptr ? *ptr : nullTexture;
}

[[nodiscard]] sf::Font& HGAssets::HGAssetsImpl::getFont(const std::string& mId)
{
    sf::Font* ptr = assetStorage->getFont(mId);
    SSVOH_ASSERT(ptr);

    return *ptr;
}

[[nodiscard]] sf::Font& HGAssets::HGAssetsImpl::getFontOrNullFont(
    const std::string& mId)
{
    static sf::Font nullFont;
    sf::Font* ptr = assetStorage->getFont(mId);

    return ptr ? *ptr : nullFont;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::isValidLevelId(
    const std::string& mLevelId) const noexcept
{
    return levelDatas.find(mLevelId) != levelDatas.end();
}

[[nodiscard]] const LevelData& HGAssets::HGAssetsImpl::getLevelData(
    const std::string& mAssetId) const
{
    SSVOH_ASSERT(isValidLevelId(mAssetId));
    return levelDatas.at(mAssetId);
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::packHasLevels(
    const std::string& mPackId)
{
    return levelDataIdsByPack.count(mPackId) > 0;
}

[[nodiscard]] const std::vector<std::string>&
HGAssets::HGAssetsImpl::getLevelIdsByPack(const std::string& mPackId)
{
    SSVOH_ASSERT(levelDataIdsByPack.count(mPackId) > 0);
    return levelDataIdsByPack.at(mPackId);
}

[[nodiscard]] const std::unordered_map<std::string, PackData>&
HGAssets::HGAssetsImpl::getPackDatas()
{
    return packDatas;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::isValidPackId(
    const std::string& mPackId) const noexcept
{
    return packDatas.find(mPackId) != packDatas.end();
}

[[nodiscard]] const PackData& HGAssets::HGAssetsImpl::getPackData(
    const std::string& mPackId)
{
    SSVOH_ASSERT(isValidPackId(mPackId));
    return packDatas.at(mPackId);
}

[[nodiscard]] const std::vector<PackInfo>&
HGAssets::HGAssetsImpl::getSelectablePackInfos() const noexcept
{
    return selectablePackInfos;
}

[[nodiscard]] const PackData* HGAssets::HGAssetsImpl::findPackData(
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

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadWorkshopPackDatasFromCache()
{
    if(!ssvufs::Path{"workshopCache.json"}.isFile())
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

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadAllPackDatas()
{
    if(!ssvufs::Path{"Packs/"}.isFolder())
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

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadAllPackAssets(
    const bool headless)
{
    for(const auto& [packId, packData] : packDatas)
    {
        if(loadPackAssets(packData, headless))
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

[[nodiscard]] bool HGAssets::HGAssetsImpl::verifyAllPackDependencies()
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

void HGAssets::HGAssetsImpl::addLocalProfile(ProfileData&& profileData)
{
    // Remove invalid level ids that might have been added to the files.
    Utils::erase_if(profileData.getFavoriteLevelIds(),
        [this](const std::string& favId)
        { return levelDatas.find(favId) == levelDatas.end(); });

    profileDataMap.emplace(profileData.getName(), SSVOH_MOVE(profileData));
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadAllLocalProfiles()
{
    if(!ssvufs::Path{"Profiles/"}.isFolder())
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
        addLocalProfile(SSVOH_MOVE(profileData));
    }

    return true;
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadShaders(
    const std::string& mPackId, const ssvufs::Path& mPath, const bool headless)
{
    if(headless)
    {
        // Always return early in headless mode.
        return;
    }

    const auto loadShadersOfType =
        [&](const char* const extension, sf::Shader::Type shaderType)
    {
        for(const auto& p : scanSingleByExt(mPath + "Shaders/", extension))
        {
            auto shader = Utils::makeUnique<sf::Shader>();

            if(!shader->loadFromFile(p.getStr(), shaderType))
            {
                ssvu::lo("hg::loadPackAssets_loadShaders")
                    << "Failed to load shader '" << p << "'\n";

                continue;
            }

            shadersById.push_back(shader.get());
            SSVOH_ASSERT(shadersById.size() > 0);
            const std::size_t shaderId = shadersById.size() - 1;

            LoadedShader ls{.shader{SSVOH_MOVE(shader)},
                .path{p},
                .shaderType{shaderType},
                .id{shaderId}};

            shaders.emplace(
                concatIntoBuf(mPackId, '_', p.getFileName()), SSVOH_MOVE(ls));

            shadersPathToId.emplace(p, shaderId);

            ++loadInfo.assets;
        }
    };

    loadShadersOfType(".vert", sf::Shader::Type::Vertex);
    loadShadersOfType(".geom", sf::Shader::Type::Geometry);
    loadShadersOfType(".frag", sf::Shader::Type::Fragment);
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadCustomSounds(
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

void HGAssets::HGAssetsImpl::loadPackAssets_loadMusic(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".ogg"))
    {
        musicPathMap.emplace(
            concatIntoBuf(mPackId, '_', p.getFileNameNoExtensions()), p);

        ++loadInfo.assets;
    }
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadMusicData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        MusicData musicData{Utils::loadMusicFromJson(object)};
        musicDataMap.emplace(
            concatIntoBuf(mPackId, '_', musicData.id), SSVOH_MOVE(musicData));

        ++loadInfo.assets;
    }
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadStyleData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        StyleData styleData{object};
        styleDataMap.emplace(
            concatIntoBuf(mPackId, '_', styleData.id), SSVOH_MOVE(styleData));

        ++loadInfo.assets;
    }
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadLevelData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Levels/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        LevelData levelData{object, mPath, mPackId};
        const std::string& assetId = concatIntoBuf(mPackId, '_', levelData.id);

        levelDataIdsByPack[mPackId].emplace_back(assetId);
        levelDatas.emplace(assetId, SSVOH_MOVE(levelData));

        ++loadInfo.levels;
    }
}

//**********************************************
// PROFILE

void HGAssets::HGAssetsImpl::saveCurrentLocalProfile()
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

void HGAssets::HGAssetsImpl::saveAllProfiles()
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

[[nodiscard]] const MusicData& HGAssets::HGAssetsImpl::getMusicData(
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

[[nodiscard]] const StyleData& HGAssets::HGAssetsImpl::getStyleData(
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

[[nodiscard]] sf::Shader* HGAssets::HGAssetsImpl::getShader(
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

[[nodiscard]] std::optional<std::size_t> HGAssets::HGAssetsImpl::getShaderId(
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

[[nodiscard]] std::optional<std::size_t>
HGAssets::HGAssetsImpl::getShaderIdByPath(const std::string& mShaderPath)
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

[[nodiscard]] sf::Shader* HGAssets::HGAssetsImpl::getShaderByShaderId(
    const std::size_t mShaderId)
{
    if(!isValidShaderId(mShaderId))
    {
        return nullptr;
    }

    return shadersById[mShaderId];
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::isValidShaderId(
    const std::size_t mShaderId) const
{
    return mShaderId < shadersById.size();
}

//**********************************************
// RELOAD

void HGAssets::HGAssetsImpl::reloadAllShaders()
{
    for(auto& [id, loadedShader] : shaders)
    {
        if(!loadedShader.shader->loadFromFile(
               loadedShader.path, loadedShader.shaderType))
        {
            ssvu::lo("hg::HGAssetsImplImpl::reloadAllShaders")
                << "Failed to load shader '" << loadedShader.path << "'\n";

            continue;
        }
    }
}

[[nodiscard]] std::string HGAssets::HGAssetsImpl::reloadPack(
    const std::string& mPackId, const std::string& mPath)
{
    std::string temp, output;

    // Levels, if there is not folder cancel everything
    temp = mPath + "Levels/";
    if(!ssvufs::Path{temp}.isFolder())
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
            levelDatas.emplace(temp, SSVOH_MOVE(levelData));
        }
        else
        {
            it->second = levelData;
        }
    }
    output += "Levels successfully reloaded\n";

    // Styles
    temp = mPath + "Styles/";
    if(!ssvufs::Path{temp}.isFolder())
    {
        output += "invalid style folder path\n";
    }
    else
    {
        for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
        {
            StyleData styleData{ssvuj::getFromFile(p)};
            temp = mPackId + "_" + styleData.id;

            styleDataMap[temp] = SSVOH_MOVE(styleData);
        }
        output += "Styles successfully reloaded\n";
    }

    // Music data
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.isFolder())
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

            musicDataMap[temp] = SSVOH_MOVE(musicData);
        }
        output += "Music data successfully reloaded\n";
    }

    // Music
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.isFolder())
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
    if(!ssvufs::Path{temp}.isFolder())
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

[[nodiscard]] std::string HGAssets::HGAssetsImpl::reloadLevel(
    const std::string& mPackId, const std::string& mPath,
    const std::string& mId)
{
    std::string temp, output;

    //*******************************************
    // Level
    temp = mPath + "Levels/";
    if(!ssvufs::Path{temp}.isFolder())
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
        levelDatas.emplace(temp, SSVOH_MOVE(levelData));
    }
    else
    {
        it->second = levelData;
    }
    output = "level data " + mId + ".json successfully loaded\n";

    //*******************************************
    // Style
    temp = mPath + "Styles/";
    if(!ssvufs::Path{temp}.isFolder())
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

            styleDataMap[temp] = SSVOH_MOVE(styleData);

            output += "style data " + levelData.styleId +
                      ".json successfully loaded\n";
        }
    }

    //*******************************************
    // Music data
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.isFolder())
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

            musicDataMap[temp] = SSVOH_MOVE(musicData);

            output += "music data " + levelData.musicId +
                      ".json successfully loaded\n";
        }
    }

    //*******************************************
    // Music files
    std::string assetId;
    temp = mPath + "Music/";
    if(!ssvufs::Path{temp}.isFolder())
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
    if(!ssvufs::Path{temp}.isFolder())
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

float HGAssets::HGAssetsImpl::getLocalScore(const std::string& mId)
{
    return getCurrentLocalProfile().getScore(mId);
}

void HGAssets::HGAssetsImpl::setLocalScore(const std::string& mId, float mScore)
{
    getCurrentLocalProfile().setScore(mId, mScore);
}

//**********************************************
// LOCAL PROFILE

[[nodiscard]] bool HGAssets::HGAssetsImpl::anyLocalProfileActive() const
{
    return currentProfilePtr != nullptr;
}

ProfileData& HGAssets::HGAssetsImpl::getCurrentLocalProfile()
{
    SSVOH_ASSERT(currentProfilePtr != nullptr);
    return *currentProfilePtr;
}

ProfileData* HGAssets::HGAssetsImpl::getLocalProfileByName(
    const std::string& mName)
{
    SSVOH_ASSERT(profileDataMap.contains(mName));
    return &profileDataMap.find(mName)->second;
}

const ProfileData& HGAssets::HGAssetsImpl::getCurrentLocalProfile() const
{
    SSVOH_ASSERT(currentProfilePtr != nullptr);
    return *currentProfilePtr;
}

const ProfileData* HGAssets::HGAssetsImpl::getLocalProfileByName(
    const std::string& mName) const
{
    SSVOH_ASSERT(profileDataMap.contains(mName));
    return &profileDataMap.find(mName)->second;
}

[[nodiscard]] std::string
HGAssets::HGAssetsImpl::getCurrentLocalProfileFilePath()
{
    return "Profiles/" + currentProfilePtr->getName() + ".json";
}


[[nodiscard]] std::size_t HGAssets::HGAssetsImpl::getLocalProfilesSize()
{
    return profileDataMap.size();
}

[[nodiscard]] std::vector<std::string>
HGAssets::HGAssetsImpl::getLocalProfileNames()
{
    std::vector<std::string> result;
    result.reserve(profileDataMap.size());

    for(auto& pair : profileDataMap)
    {
        result.emplace_back(pair.second.getName());
    }

    return result;
}


[[nodiscard]] bool HGAssets::HGAssetsImpl::pIsValidLocalProfile() const
{
    return currentProfilePtr != nullptr;
}

[[nodiscard]] const std::string& HGAssets::HGAssetsImpl::pGetName() const
{
    return getCurrentLocalProfile().getName();
}

void HGAssets::HGAssetsImpl::pSaveCurrent()
{
    saveCurrentLocalProfile();
}

void HGAssets::HGAssetsImpl::pSaveAll()
{
    saveAllProfiles();
}

void HGAssets::HGAssetsImpl::pSetCurrent(const std::string& mName)
{
    const auto it = profileDataMap.find(mName);

    SSVOH_ASSERT(it != profileDataMap.end());
    currentProfilePtr = &it->second;
}

void HGAssets::HGAssetsImpl::pCreate(const std::string& mName)
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

void HGAssets::HGAssetsImpl::pRemove(const std::string& mName)
{
    profileDataMap.erase(mName);
}

[[nodiscard]] sf::SoundBuffer* HGAssets::HGAssetsImpl::getSoundBuffer(
    const std::string& assetId)
{
    return assetStorage->getSoundBuffer(assetId);
}

[[nodiscard]] const std::string* HGAssets::HGAssetsImpl::getMusicPath(
    const std::string& assetId) const
{
    auto it = musicPathMap.find(assetId);
    return it == musicPathMap.end() ? nullptr : &it->second;
}

[[nodiscard]] const std::unordered_map<std::string, LevelData>&
HGAssets::HGAssetsImpl::getLevelDatas() const noexcept
{
    return levelDatas;
}

[[nodiscard]] const std::unordered_set<std::string>&
HGAssets::HGAssetsImpl::getPackIdsWithMissingDependencies() const noexcept
{
    return packIdsWithMissingDependencies;
}

[[nodiscard]] std::unordered_map<std::string, std::string>&
HGAssets::HGAssetsImpl::getLuaFileCache()
{
    return luaFileCache;
}

[[nodiscard]] const std::unordered_map<std::string, std::string>&
HGAssets::HGAssetsImpl::getLuaFileCache() const
{
    return luaFileCache;
}

// ----------------------------------------------------------------------------

HGAssets::HGAssets(
    Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly)
    : _impl(Utils::makeUnique<HGAssetsImpl>(
          mSteamManager, mHeadless, mLevelsOnly))
{}

HGAssets::~HGAssets() = default;

LoadInfo& HGAssets::getLoadResults()
{
    return _impl->getLoadResults();
}

sf::Texture& HGAssets::getTexture(const std::string& mId)
{
    return _impl->getTexture(mId);
}

sf::Texture& HGAssets::getTextureOrNullTexture(const std::string& mId)
{
    return _impl->getTextureOrNullTexture(mId);
}

sf::Font& HGAssets::getFont(const std::string& mId)
{
    return _impl->getFont(mId);
}

sf::Font& HGAssets::getFontOrNullFont(const std::string& mId)
{
    return _impl->getFontOrNullFont(mId);
}

bool HGAssets::isValidLevelId(const std::string& mLevelId) const noexcept
{
    return _impl->isValidLevelId(mLevelId);
}

const LevelData& HGAssets::getLevelData(const std::string& mAssetId) const
{
    return _impl->getLevelData(mAssetId);
}

bool HGAssets::packHasLevels(const std::string& mPackId)
{
    return _impl->packHasLevels(mPackId);
}

const std::vector<std::string>& HGAssets::getLevelIdsByPack(
    const std::string& mPackId)
{
    return _impl->getLevelIdsByPack(mPackId);
}

const std::unordered_map<std::string, PackData>& HGAssets::getPackDatas()
{
    return _impl->getPackDatas();
}

bool HGAssets::isValidPackId(const std::string& mPackId) const noexcept
{
    return _impl->isValidPackId(mPackId);
}

const PackData& HGAssets::getPackData(const std::string& mPackId)
{
    return _impl->getPackData(mPackId);
}

const std::vector<PackInfo>& HGAssets::getSelectablePackInfos() const noexcept
{
    return _impl->getSelectablePackInfos();
}

const PackData* HGAssets::findPackData(const std::string& mPackDisambiguator,
    const std::string& mPackName, const std::string& mPackAuthor) const noexcept
{
    return _impl->findPackData(mPackDisambiguator, mPackName, mPackAuthor);
}

const MusicData& HGAssets::getMusicData(
    const std::string& mPackId, const std::string& mId)
{
    return _impl->getMusicData(mPackId, mId);
}

const StyleData& HGAssets::getStyleData(
    const std::string& mPackId, const std::string& mId)
{
    return _impl->getStyleData(mPackId, mId);
}

sf::Shader* HGAssets::getShader(
    const std::string& mPackId, const std::string& mId)
{
    return _impl->getShader(mPackId, mId);
}

std::optional<std::size_t> HGAssets::getShaderId(
    const std::string& mPackId, const std::string& mId)
{
    return _impl->getShaderId(mPackId, mId);
}

std::optional<std::size_t> HGAssets::getShaderIdByPath(
    const std::string& mShaderPath)
{
    return _impl->getShaderIdByPath(mShaderPath);
}

sf::Shader* HGAssets::getShaderByShaderId(const std::size_t mShaderId)
{
    return _impl->getShaderByShaderId(mShaderId);
}

bool HGAssets::isValidShaderId(const std::size_t mShaderId) const
{
    return _impl->isValidShaderId(mShaderId);
}

void HGAssets::reloadAllShaders()
{
    return _impl->reloadAllShaders();
}

std::string HGAssets::reloadPack(
    const std::string& mPackId, const std::string& mPath)
{
    return _impl->reloadPack(mPackId, mPath);
}

std::string HGAssets::reloadLevel(const std::string& mPackId,
    const std::string& mPath, const std::string& mId)
{
    return _impl->reloadLevel(mPackId, mPath, mId);
}

float HGAssets::getLocalScore(const std::string& mId)
{
    return _impl->getLocalScore(mId);
}

void HGAssets::setLocalScore(const std::string& mId, float mScore)
{
    return _impl->setLocalScore(mId, mScore);
}

void HGAssets::saveCurrentLocalProfile()
{
    return _impl->saveCurrentLocalProfile();
}

void HGAssets::saveAllProfiles()
{
    return _impl->saveAllProfiles();
}

bool HGAssets::anyLocalProfileActive() const
{
    return _impl->anyLocalProfileActive();
}

ProfileData& HGAssets::getCurrentLocalProfile()
{
    return _impl->getCurrentLocalProfile();
}

const ProfileData& HGAssets::getCurrentLocalProfile() const
{
    return _impl->getCurrentLocalProfile();
}

ProfileData* HGAssets::getLocalProfileByName(const std::string& mName)
{
    return _impl->getLocalProfileByName(mName);
}

const ProfileData* HGAssets::getLocalProfileByName(
    const std::string& mName) const
{
    return _impl->getLocalProfileByName(mName);
}

std::size_t HGAssets::getLocalProfilesSize()
{
    return _impl->getLocalProfilesSize();
}

std::vector<std::string> HGAssets::getLocalProfileNames()
{
    return _impl->getLocalProfileNames();
}

bool HGAssets::pIsValidLocalProfile() const
{
    return _impl->pIsValidLocalProfile();
}

const std::string& HGAssets::pGetName() const
{
    return _impl->pGetName();
}

void HGAssets::pSaveCurrent()
{
    return _impl->pSaveCurrent();
}

void HGAssets::pSaveAll()
{
    return _impl->pSaveAll();
}

void HGAssets::pSetCurrent(const std::string& mName)
{
    return _impl->pSetCurrent(mName);
}

void HGAssets::pCreate(const std::string& mName)
{
    return _impl->pCreate(mName);
}

void HGAssets::pRemove(const std::string& mName)
{
    return _impl->pRemove(mName);
}

sf::SoundBuffer* HGAssets::getSoundBuffer(const std::string& assetId)
{
    return _impl->getSoundBuffer(assetId);
}

const std::string* HGAssets::getMusicPath(const std::string& assetId) const
{
    return _impl->getMusicPath(assetId);
}

const std::unordered_map<std::string, LevelData>&
HGAssets::getLevelDatas() const noexcept
{
    return _impl->getLevelDatas();
}

const std::unordered_set<std::string>&
HGAssets::getPackIdsWithMissingDependencies() const noexcept
{
    return _impl->getPackIdsWithMissingDependencies();
}

void HGAssets::addLocalProfile(ProfileData&& profileData)
{
    return _impl->addLocalProfile(SSVOH_MOVE(profileData));
}

std::unordered_map<std::string, std::string>& HGAssets::getLuaFileCache()
{
    return _impl->getLuaFileCache();
}

const std::unordered_map<std::string, std::string>&
HGAssets::getLuaFileCache() const
{
    return _impl->getLuaFileCache();
}


} // namespace hg
