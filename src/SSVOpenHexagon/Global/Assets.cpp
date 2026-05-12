// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

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
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/JsonCpp.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters_AnkerlUnorderedDense.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Io.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"
#include "SSVOpenHexagon/Utils/BuildPackId.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/EraseIf.hpp"
#include "SSVOpenHexagon/Utils/LoadFromJson.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Algorithm/Remove.hpp"
#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <exception>
#include <stdexcept>

#include <cstring>

namespace hg
{

class HGAssets::HGAssetsImpl
{
private:
    Steam::steam_manager* steamManager;
    const bool            _headless;

    bool levelsOnly{false};

    sf::base::UniquePtr<AssetStorage> assetStorage;

    // `UniquePtr` indirection gives address stability: `HexagonGame::levelData`
    // (set via `setLevelData(&...)`) and `MenuGame::currentPack` hold long-lived
    // pointers into the per-entry payloads, which must survive map rehashes.
    ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<LevelData>>     levelDatas;
    ankerl::unordered_dense::map<sf::base::String, sf::base::Vector<sf::base::String>> levelDataIdsByPack;

    ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<PackData>> packDatas;

    sf::base::Vector<PackInfo> packInfos;
    sf::base::Vector<PackInfo> selectablePackInfos;

    ankerl::unordered_dense::map<sf::base::String, sf::base::String>                 musicPathMap;
    ankerl::unordered_dense::map<sf::base::String, MusicData>                        musicDataMap;
    ankerl::unordered_dense::map<sf::base::String, StyleData>                        styleDataMap;
    // `UniquePtr` indirection: `currentProfilePtr` / `Services::currentProfile`
    // hold long-lived `ProfileData*` into this map.
    ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<ProfileData>> profileDataMap;
    ProfileData*                                                                     currentProfilePtr{nullptr};

    ankerl::unordered_dense::set<sf::base::String> packIdsWithMissingDependencies;

    // Bumped any time the level/pack list changes; surfaced via
    // `HGAssets::packListVersion()`.
    sf::base::U64 _packListVersion{0};

    struct LoadedShader
    {
        sf::base::UniquePtr<sf::Shader> shader;
        sf::base::String                path;
        sf::Shader::Type                shaderType;
        sf::base::SizeT                 id;
    };

    ankerl::unordered_dense::map<sf::base::String, LoadedShader>    shaders;
    ankerl::unordered_dense::map<sf::base::String, sf::base::SizeT> shadersPathToId;
    sf::base::Vector<sf::Shader*>                                   shadersById;

    sf::base::String buf;

    LoadInfo loadInfo;

    // When the Steam API can not be retrieved, this set holds pack ids
    // retrieved from the cache to try and load the workshop packs installed
    ankerl::unordered_dense::set<sf::base::String> cachedWorkshopPackIds;

    template <typename... Ts>
    [[nodiscard]] sf::base::String& concatIntoBuf(const Ts&...);

    [[nodiscard]] bool loadAllPackDatas();
    [[nodiscard]] bool loadAllPackAssets(const bool headless);
    [[nodiscard]] bool loadWorkshopPackDatasFromCache();
    [[nodiscard]] bool verifyAllPackDependencies();
    [[nodiscard]] bool loadAllLocalProfiles();

    [[nodiscard]] bool loadPackData(const sf::Path& packPath);

    [[nodiscard]] bool loadPackAssets(const PackData& packData, const bool headless);

    void loadPackAssets_loadShaders(const sf::base::String& mPackId, const sf::Path& mPath, const bool headless);
    void loadPackAssets_loadMusic(const sf::base::String& mPackId, const sf::Path& mPath);
    void loadPackAssets_loadMusicData(const sf::base::String& mPackId, const sf::Path& mPath);
    void loadPackAssets_loadStyleData(const sf::base::String& mPackId, const sf::Path& mPath);
    void loadPackAssets_loadLevelData(const sf::base::String& mPackId, const sf::Path& mPath);
    void loadPackAssets_loadCustomSounds(const sf::base::String& mPackId, const sf::Path& mPath);

    [[nodiscard]] sf::base::String getCurrentLocalProfileFilePath();

public:
    HGAssetsImpl(Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly = false);

    ~HGAssetsImpl();

    [[nodiscard]] bool isHeadless() const;

    [[nodiscard]] LoadInfo& getLoadResults();

    [[nodiscard]] sf::base::U64 getPackListVersion() const noexcept
    {
        return _packListVersion;
    }
    void bumpPackListVersion() noexcept
    {
        ++_packListVersion;
    }

    [[nodiscard]] sf::base::Optional<sf::base::String> installPackAtRuntime(const sf::Path& folderPath);

    [[nodiscard]] bool removePackAtRuntime(const sf::base::String& packId);

    [[nodiscard]] bool         hasTexture(const sf::base::String& mId);
    [[nodiscard]] sf::Texture& getTexture(const sf::base::String& mId);

    [[nodiscard]] sf::Font& getFont(const sf::base::String& mId);

    [[nodiscard]] bool isValidLevelId(const sf::base::String& mLevelId) const noexcept;

    [[nodiscard]] const LevelData& getLevelData(const sf::base::String& mAssetId) const;

    [[nodiscard]] bool packHasLevels(const sf::base::String& mPackId);

    [[nodiscard]] const sf::base::Vector<sf::base::String>& getLevelIdsByPack(const sf::base::String& mPackId);

    [[nodiscard]] const ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<PackData>>& getPackDatas();

    [[nodiscard]] bool isValidPackId(const sf::base::String& mPackId) const noexcept;

    [[nodiscard]] const PackData& getPackData(const sf::base::String& mPackId);

    [[nodiscard]] const sf::base::Vector<PackInfo>& getSelectablePackInfos() const noexcept;

    [[nodiscard]] const PackData* findPackData(const sf::base::String& mPackDisambiguator,
                                               const sf::base::String& mPackName,
                                               const sf::base::String& mPackAuthor) const noexcept;

    [[nodiscard]] const MusicData& getMusicData(const sf::base::String& mPackId, const sf::base::String& mId);
    [[nodiscard]] const StyleData& getStyleData(const sf::base::String& mPackId, const sf::base::String& mId);
    [[nodiscard]] sf::Shader*      getShader(const sf::base::String& mPackId, const sf::base::String& mId);

    [[nodiscard]] sf::base::Optional<sf::base::SizeT> getShaderId(const sf::base::String& mPackId,
                                                                  const sf::base::String& mId);
    [[nodiscard]] sf::base::Optional<sf::base::SizeT> getShaderIdByPath(const sf::base::String& mShaderPath);
    [[nodiscard]] sf::Shader*                         getShaderByShaderId(const sf::base::SizeT mShaderId);
    [[nodiscard]] bool                                isValidShaderId(const sf::base::SizeT mShaderId) const;

    void                           reloadAllShaders();
    [[nodiscard]] sf::base::String reloadPack(const sf::base::String& mPackId, const sf::Path& mPath);
    [[nodiscard]] sf::base::String reloadLevel(const sf::base::String& mPackId,
                                               const sf::Path&         mPath,
                                               const sf::base::String& mId);

    [[nodiscard]] float getLocalScore(const sf::base::String& mId);
    void                setLocalScore(const sf::base::String& mId, float mScore);

    void saveCurrentLocalProfile();
    void saveAllProfiles();

    [[nodiscard]] bool                               anyLocalProfileActive() const;
    [[nodiscard]] ProfileData&                       getCurrentLocalProfile();
    [[nodiscard]] const ProfileData&                 getCurrentLocalProfile() const;
    [[nodiscard]] ProfileData*                       getLocalProfileByName(const sf::base::String& mName);
    [[nodiscard]] const ProfileData*                 getLocalProfileByName(const sf::base::String& mName) const;
    [[nodiscard]] sf::base::SizeT                    getLocalProfilesSize();
    [[nodiscard]] sf::base::Vector<sf::base::String> getLocalProfileNames();

    [[nodiscard]] bool                    pIsValidLocalProfile() const;
    [[nodiscard]] const sf::base::String& pGetName() const;

    void pSaveCurrent();
    void pSaveAll();
    void pSetCurrent(const sf::base::String& mName);
    void pCreate(const sf::base::String& mName);
    void pRemove(const sf::base::String& mName);

    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(const sf::base::String& assetId);

    [[nodiscard]] const sf::base::String* getMusicPath(const sf::base::String& assetId) const;

    [[nodiscard]] const ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<LevelData>>& getLevelDatas() const noexcept;

    [[nodiscard]] const ankerl::unordered_dense::set<sf::base::String>& getPackIdsWithMissingDependencies() const noexcept;

    void addLocalProfile(ProfileData&& profileData);
};

static void loadAssetsFromJson(AssetStorage& assetStorage, const sf::base::String& mRootPath, const ssvuj::Obj& mObj)
{
    for (const auto& f : ssvuj::getExtr<sf::base::Vector<sf::base::String>>(mObj, "fonts"))
    {
        if (!assetStorage.loadFont(f, sf::base::String(mRootPath + f.cStr())))
        {
            hg::lo("hg::loadAssetsFromJson") << "Failed to load font '" << f << "'\n";
        }
    }

    for (const auto& f : ssvuj::getExtr<sf::base::Vector<sf::base::String>>(mObj, "textures"))
    {
        if (!assetStorage.loadTexture(f, sf::base::String(mRootPath + f.cStr())))
        {
            hg::lo("hg::loadAssetsFromJson") << "Failed to load texture '" << f << "'\n";
        }
    }

    for (const auto& f : ssvuj::getExtr<sf::base::Vector<sf::base::String>>(mObj, "soundBuffers"))
    {
        if (!assetStorage.loadSoundBuffer(f, sf::base::String(mRootPath + f.cStr())))
        {
            hg::lo("hg::loadAssetsFromJson") << "Failed to load sound buffer '" << f << "'\n";
        }
    }
}

[[nodiscard]] static sf::base::Vector<sf::Path>& getScanBuffer()
{
    static sf::base::Vector<sf::Path> buffer;
    return buffer;
}

[[nodiscard]] static const sf::base::Vector<sf::Path>& scanSingleByExt(const sf::Path& path, sf::base::StringView extension)
{
    auto& buffer = getScanBuffer();
    buffer.clear();

    if (!path.forEachEntry([&](const sf::Path& entry)
    {
        if (entry.isRegularFile() && entry.extensionIs(extension))
            buffer.emplaceBack(entry);
    }))
    {
        hg::lo("scanSingleByExt") << "Failed to open directory '" << path << "'\n";
    }

    return buffer;
}

[[nodiscard]] static const sf::base::Vector<sf::Path>& scanSingleByName(const sf::Path& path, const char* name)
{
    auto& buffer = getScanBuffer();
    buffer.clear();

    const sf::Path target{name};

    if (!path.forEachEntry([&](const sf::Path& entry)
    {
        if (entry.isRegularFile() && entry.getFilename() == target)
            buffer.emplaceBack(entry);
    }))
    {
        hg::lo("scanSingleByName") << "Failed to open directory '" << path << "'\n";
    }

    return buffer;
}

[[nodiscard]] static const sf::base::Vector<sf::Path>& scanSingleFolderName(const sf::Path& path)
{
    auto& buffer = getScanBuffer();
    buffer.clear();

    if (!path.forEachEntry([&](const sf::Path& entry)
    {
        if (entry.isDirectory())
            buffer.emplaceBack(entry);
    }))
    {
        hg::lo("scanSingleFolderName") << "Failed to open directory '" << path << "'\n";
    }

    return buffer;
}

template <typename... Ts>
[[nodiscard]] sf::base::String& HGAssets::HGAssetsImpl::concatIntoBuf(const Ts&... xs)
{
    buf.clear();
    Utils::concatInto(buf, xs...);
    return buf;
}

HGAssets::HGAssetsImpl::HGAssetsImpl(Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly) :
    steamManager{mSteamManager},
    _headless{mHeadless},
    levelsOnly{mLevelsOnly},
    assetStorage{sf::base::makeUnique<AssetStorage>()}
{
    const HRTimePoint tpBeforeLoad = HRClock::now();

    if (!levelsOnly && !mHeadless)
    {
        if (!sf::Path{"Assets"}.isDirectory())
        {
            hg::lo("FATAL ERROR") << "Folder Assets/ does not exist" << logEndl;

            std::terminate();
            return;
        }

        auto [object, error] = ssvuj::getFromFileWithErrors("Assets/assets.json");

        loadAssetsFromJson(*assetStorage, "Assets/", object);

        loadInfo.addFormattedError(error);
    }

    if (!loadAllPackDatas())
    {
        hg::lo("HGAssets::HGAssets") << "Error loading all pack datas\n";
        std::terminate();
        return;
    }

    if (!loadAllPackAssets(mHeadless))
    {
        hg::lo("HGAssets::HGAssets") << "Error loading all pack assets\n";
        std::terminate();
        return;
    }

    if (!verifyAllPackDependencies())
    {
        hg::lo("HGAssets::HGAssets") << "Error verifying pack dependencies\n";
        std::terminate();
        return;
    }

    bumpPackListVersion();

    if (!loadAllLocalProfiles())
    {
        hg::lo("HGAssets::HGAssets") << "Error loading local profiles\n";
        // No need to terminate here, some tests do not require profiles.
        return;
    }

    for (auto& v : levelDataIdsByPack)
    {
        sf::base::quickSort(v.second.begin(), v.second.end(), [&](const sf::base::String& mA, const sf::base::String& mB) {
            return levelDatas.at(mA)->menuPriority < levelDatas.at(mB)->menuPriority;
        });
    }

    sf::base::quickSort(packInfos.begin(), packInfos.end(), [&](const PackInfo& mA, const PackInfo& mB) {
        return getPackData(mA.id).priority < getPackData(mB.id).priority;
    });

    sf::base::quickSort(selectablePackInfos.begin(), selectablePackInfos.end(), [&](const PackInfo& mA, const PackInfo& mB) {
        return getPackData(mA.id).priority < getPackData(mB.id).priority;
    });

    // This will not be used for the rest of the game,
    // so shrink it to fit the actually used size.
    loadInfo.errorMessages.shrinkToFit();

    const std::chrono::duration durElapsed = HRClock::now() - tpBeforeLoad;

    hg::lo("HGAssets::HGAssets") << "Loaded all assets in "
                                 << std::chrono::duration_cast<std::chrono::milliseconds>(durElapsed).count() << "ms\n";
}

HGAssets::HGAssetsImpl::~HGAssetsImpl()
{
    hg::lo("HGAssets::~HGAssets") << "Cleaning up assets...\n";
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::isHeadless() const
{
    return _headless;
}

[[nodiscard]] bool HGAssets::isHeadless() const
{
    return _impl->isHeadless();
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadPackData(const sf::Path& packPath)
{
    const sf::Path packJsonPath = packPath / "pack.json";

    if (!packJsonPath.isRegularFile())
    {
        return false;
    }

    auto p = ssvuj::getFromFileWithErrors(packJsonPath);

    // Workaround of lambda capture of structured binding.
    auto& packRoot = p.first;
    auto& error    = p.second;

    loadInfo.addFormattedError(error);

    auto packDisambiguator = ssvuj::getExtr<sf::base::String>(packRoot, "disambiguator", "no disambiguator");

    auto packName = ssvuj::getExtr<sf::base::String>(packRoot, "name", "unknown name");

    auto packAuthor = ssvuj::getExtr<sf::base::String>(packRoot, "author", "unknown author");

    auto packDescription = ssvuj::getExtr<sf::base::String>(packRoot, "description", "no description");

    const auto packVersion = ssvuj::getExtr<int>(packRoot, "version", 0);

    const auto packPriority = ssvuj::getExtr<float>(packRoot, "priority", 100);

    const sf::base::String packId = Utils::buildPackId(packDisambiguator, packAuthor, packName, packVersion);

    const auto getPackDependencies = [&]
    {
        sf::base::Vector<PackDependency> result;

        if (!ssvuj::hasObj(packRoot, "dependencies"))
        {
            return result;
        }

        const ssvuj::Obj& objDependencies = ssvuj::getObj(packRoot, "dependencies");

        const auto dependencyCount = ssvuj::getObjSize(objDependencies);
        result.reserve(dependencyCount);

        for (sf::base::SizeT i = 0; i < dependencyCount; ++i)
        {
            const ssvuj::Obj& pdRoot = ssvuj::getObj(objDependencies, i);

            result.emplaceBack(PackDependency{
                ssvuj::getExtr<sf::base::String>(pdRoot, "disambiguator"),
                ssvuj::getExtr<sf::base::String>(pdRoot, "name"),
                ssvuj::getExtr<sf::base::String>(pdRoot, "author"),
                ssvuj::getExtr<int>(pdRoot, "min_version"),
            });
        }

        return result;
    };

    sf::base::String packIdStdString{packId.data(), packId.size()};

    packInfos.emplaceBack(PackInfo{packIdStdString, packPath});

    packDatas.emplace(packIdStdString, //
                      sf::base::makeUnique<PackData>(PackData{
                          .folderPath{packPath},                             //
                          .id{packIdStdString},                              //
                          .disambiguator{SFML_BASE_MOVE(packDisambiguator)}, //
                          .name{SFML_BASE_MOVE(packName)},                   //
                          .author{SFML_BASE_MOVE(packAuthor)},               //
                          .description{SFML_BASE_MOVE(packDescription)},     //
                          .version{packVersion},                             //
                          .priority{packPriority},                           //
                          .dependencies{getPackDependencies()}               //
                      }));

    return true;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadPackAssets(const PackData& packData, const bool headless)
{
    const sf::Path&         packPath{packData.folderPath};
    const sf::base::String& packId{packData.id};

    hg::lo("::loadAssets") << "loading '" << packId << "' assets\n";

    try
    {
        if (!headless)
        {
            if ((packPath / "Shaders").isDirectory() && !levelsOnly)
            {
                loadPackAssets_loadShaders(packId, packPath, headless);
            }

            if (!levelsOnly && (packPath / "Sounds").isDirectory())
            {
                loadPackAssets_loadCustomSounds(packId, packPath);
            }
        }

        if ((packPath / "Music").isDirectory() && !levelsOnly)
        {
            if (!headless)
            {
                loadPackAssets_loadMusic(packId, packPath);
            }

            loadPackAssets_loadMusicData(packId, packPath);
        }

        if ((packPath / "Styles").isDirectory())
        {
            loadPackAssets_loadStyleData(packId, packPath);
        }

        if ((packPath / "Levels").isDirectory())
        {
            loadPackAssets_loadLevelData(packId, packPath);
        }
    } catch (const std::runtime_error& mEx)
    {
        const sf::base::String& errorMessage = concatIntoBuf("Exception during asset loading: ", mEx.what(), '\n');

        loadInfo.errorMessages.emplaceBack("FATAL ERROR, " + errorMessage);
        hg::lo("FATAL ERROR") << errorMessage;
        return false;
    } catch (...)
    {
        const sf::base::String errorMessage = "Exception during asset loading: unknown.\n";

        loadInfo.errorMessages.emplaceBack("FATAL ERROR, " + errorMessage);
        hg::lo("FATAL ERROR") << errorMessage;
        return false;
    }

    if (packHasLevels(packId))
    {
        selectablePackInfos.emplaceBack(PackInfo{packId, packPath});
    }

    return true;
}

//**********************************************
// LOAD

[[nodiscard]] LoadInfo& HGAssets::HGAssetsImpl::getLoadResults()
{
    return loadInfo;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::hasTexture(const sf::base::String& mId)
{
    sf::Texture* ptr = assetStorage->getTexture(mId);
    return ptr != nullptr;
}

[[nodiscard]] sf::Texture& HGAssets::HGAssetsImpl::getTexture(const sf::base::String& mId)
{
    sf::Texture* ptr = assetStorage->getTexture(mId);
    SSVOH_ASSERT(ptr);

    return *ptr;
}

[[nodiscard]] sf::Font& HGAssets::HGAssetsImpl::getFont(const sf::base::String& mId)
{
    if (sf::Font* ptr = assetStorage->getFont(mId))
    {
        return *ptr;
    }

    sf::cErr() << "Fatal error: missing font file '" << mId << '\'' << sf::endL;
    std::terminate();
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::isValidLevelId(const sf::base::String& mLevelId) const noexcept
{
    return levelDatas.find(mLevelId) != levelDatas.end();
}

[[nodiscard]] const LevelData& HGAssets::HGAssetsImpl::getLevelData(const sf::base::String& mAssetId) const
{
    SSVOH_ASSERT(isValidLevelId(mAssetId));
    return *levelDatas.at(mAssetId);
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::packHasLevels(const sf::base::String& mPackId)
{
    return levelDataIdsByPack.count(mPackId) > 0;
}

[[nodiscard]] const sf::base::Vector<sf::base::String>& HGAssets::HGAssetsImpl::getLevelIdsByPack(const sf::base::String& mPackId)
{
    SSVOH_ASSERT(levelDataIdsByPack.count(mPackId) > 0);
    return levelDataIdsByPack.at(mPackId);
}

[[nodiscard]] const ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<PackData>>& HGAssets::HGAssetsImpl::getPackDatas()
{
    return packDatas;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::isValidPackId(const sf::base::String& mPackId) const noexcept
{
    return packDatas.find(mPackId) != packDatas.end();
}

[[nodiscard]] const PackData& HGAssets::HGAssetsImpl::getPackData(const sf::base::String& mPackId)
{
    SSVOH_ASSERT(isValidPackId(mPackId));
    return *packDatas.at(mPackId);
}

[[nodiscard]] const sf::base::Vector<PackInfo>& HGAssets::HGAssetsImpl::getSelectablePackInfos() const noexcept
{
    return selectablePackInfos;
}

[[nodiscard]] const PackData* HGAssets::HGAssetsImpl::findPackData(const sf::base::String& mPackDisambiguator,
                                                                   const sf::base::String& mPackName,
                                                                   const sf::base::String& mPackAuthor) const noexcept
{
    for (const auto& [packId, packData] : packDatas)
    {
        if (packData->disambiguator == mPackDisambiguator && //
            packData->name == mPackName &&                   //
            packData->author == mPackAuthor)
        {
            return packData.get();
        }
    }

    return nullptr;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadWorkshopPackDatasFromCache()
{
    if (!sf::Path{"workshopCache.json"}.isRegularFile())
    {
        hg::lo("::loadAssets") << "Workshop cache file does not exist. No "
                                  "workshop packs to load\n";
        return false;
    }
    auto [cacheObject, cacheError] = ssvuj::getFromFileWithErrors("workshopCache.json");

    hg::lo("::loadAssets") << "Loading workshop packs from cache\n";
    if (ssvuj::hasObj(cacheObject, "cachedPacks"))
    {
        // Null check
        auto& packValue = ssvuj::getObj(cacheObject, "cachedPacks");
        if (packValue.type() == Json::ValueType::nullValue || packValue.type() != Json::ValueType::arrayValue)
        {
            hg::lo("::loadAssets") << "Cache array is null. No workshop packs to load\n";
            return false;
        }

        // Empty check
        const auto packArray = ssvuj::getExtr<sf::base::Vector<sf::base::String>>(cacheObject, "cachedPacks");

        if (packArray.size() <= 0)
        {
            hg::lo("::loadAssets") << "Cache array is empty. No workshop packs to load\n";
            return false;
        }

        for (const auto& f : packArray)
        {
            // Simply emplace them. We will check them later.
            cachedWorkshopPackIds.emplace(f);
        }
    }
    else
    {
        hg::lo("::loadAssets") << "[ERROR]: Cannot locate cache array in "
                                  "workshop cache file\n";

        return false;
    }

    loadInfo.addFormattedError(cacheError);
    return true;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadAllPackDatas()
{
    if (!sf::Path{"Packs"}.isDirectory())
    {
        hg::lo("::loadAssets") << "Folder Packs/ does not exist" << logEndl;
        return false;
    }

    // ------------------------------------------------------------------------
    const auto tryLoadPackFromPath = [&](const sf::Path& packPath)
    {
        if (!loadPackData(packPath))
        {
            const sf::base::String& errorMessage = concatIntoBuf("Error loading pack data '",
                                                                 packPath.to<sf::base::String>(),
                                                                 '\n');

            loadInfo.errorMessages.emplaceBack(errorMessage);
            hg::lo("::loadAssets") << errorMessage;
        }
        else
        {
            ++loadInfo.packs;
        }
    };

    // ------------------------------------------------------------------------
    // Load pack datas from `Packs/` folder.
    for (const sf::Path& packPath : scanSingleFolderName("Packs"))
    {
        tryLoadPackFromPath(packPath);
    }

    // ------------------------------------------------------------------------
    // Load pack datas from Steam workshop.
    if (steamManager != nullptr)
    {
        if (steamManager->is_initialized())
        {
            steamManager->for_workshop_pack_folders([&](const sf::base::String& packPath)
            { tryLoadPackFromPath(sf::Path{packPath.cStr()}); });
        }
        else if (loadWorkshopPackDatasFromCache())
        {
            // In the case the Steam API can't be retrieved, look for a
            // cache that contains the paths we need to load
            for (const sf::base::String& cachedPath : cachedWorkshopPackIds)
            {
                tryLoadPackFromPath(sf::Path{cachedPath.cStr()});
            }
        }
    }

    return true;
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadAllPackAssets(const bool headless)
{
    for (const auto& [packId, packData] : packDatas)
    {
        if (loadPackAssets(*packData, headless))
        {
            continue;
        }

        const sf::base::String& errorMessage = concatIntoBuf("Error loading pack info '", packId, '\n');

        loadInfo.errorMessages.emplaceBack(errorMessage);
        hg::lo("::loadAssets") << errorMessage;

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
        for (const auto& [packId, packData] : packDatas)
        {
            if (                                                 //
                (packData->disambiguator == pd.disambiguator) && //
                (packData->name == pd.name) &&                   //
                (packData->author == pd.author) &&               //
                (packData->version >= pd.minVersion))
            {
                return true;
            }
        }

        return false;
    };

    for (const auto& [packId, packData] : packDatas)
    {
        for (const PackDependency& pd : packData->dependencies)
        {
            if (dependencyExists(pd))
            {
                continue;
            }

            const sf::base::String&
                errorMessage = concatIntoBuf("Missing pack dependency '", pd.name, "' for pack '", packData->name, "'\n");

            loadInfo.errorMessages.emplaceBack(errorMessage);
            hg::lo("::loadAssets") << errorMessage;

            packIdsWithMissingDependencies.emplace(packId);
        }
    }

    auto eraseRemoveIf = [](auto& mContainer, auto&& mPredicate)
    { mContainer.erase(sf::base::removeIf(mContainer.begin(), mContainer.end(), mPredicate), mContainer.end()); };

    eraseRemoveIf(selectablePackInfos, [&](const PackInfo& pi) { return packIdsWithMissingDependencies.contains(pi.id); });

    return true;
}

void HGAssets::HGAssetsImpl::addLocalProfile(ProfileData&& profileData)
{
    // Remove invalid level ids that might have been added to the files.
    Utils::erase_if(profileData.getFavoriteLevelIds(),
                    [this](const sf::base::String& favId) { return levelDatas.find(favId) == levelDatas.end(); });

    sf::base::String name = profileData.getName();
    profileDataMap.emplace(SFML_BASE_MOVE(name), sf::base::makeUnique<ProfileData>(SFML_BASE_MOVE(profileData)));
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::loadAllLocalProfiles()
{
    if (!sf::Path{"Profiles"}.isDirectory())
    {
        hg::lo("::loadAssets") << "Folder Profiles/ does not exist" << logEndl;

        return false;
    }

    hg::lo("::loadAssets") << "loading local profiles\n";

    for (const auto& p : scanSingleByExt("Profiles/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        ProfileData profileData{Utils::loadProfileFromJson(object)};
        addLocalProfile(SFML_BASE_MOVE(profileData));
    }

    return true;
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadShaders(const sf::base::String& mPackId, const sf::Path& mPath, const bool headless)
{
    if (headless)
    {
        // Always return early in headless mode.
        return;
    }

    const auto migrateShader = [](const sf::base::String& shaderCode) -> sf::base::String
    {
        const bool needsMigration = shaderCode.contains("#version") || shaderCode.contains("gl_Color") ||
                                    shaderCode.contains("gl_FragColor") || shaderCode.contains("gl_TexCoord");

        if (!needsMigration)
            return shaderCode;

        hg::lo("HGAssets::migrateShader") << "Legacy shader code detected, migrating...\n";

        // Split shader code into lines
        sf::base::Vector<sf::base::String> lines;
        shaderCode.forLines([&](sf::base::StringView line) { lines.emplaceBack(line); });

        const auto mergeLines = [&lines]
        {
            sf::base::String result;
            bool             first = true;

            for (const auto& line : lines)
            {
                if (!first)
                    result += '\n';

                first = false;

                result += line;
            }

            return result;
        };

        // Find and remove any line starting with `#version`
        sf::base::vectorEraseIf(lines, [](const sf::base::String& line) { return line.startsWith("#version"); });

        // Replace legacy usages of `gl_Color` or `gl_FragColor` and replace them with `in`/`out` variables
        lines.insert(lines.begin(), "in vec4 sf_v_color;");
        lines.insert(lines.begin() + 1, "in vec2 sf_v_texCoord;");
        lines.insert(lines.begin() + 2, "layout(location = 0) out vec4 sf_fragColor;");

        for (auto& line : lines)
        {
            line.replaceAllOccurrences("gl_Color", "sf_v_color");
            line.replaceAllOccurrences("gl_FragColor", "sf_fragColor");
            line.replaceAllOccurrences("gl_TexCoord[0].xy", "sf_v_texCoord");
        }

        return mergeLines();
    };

    const auto loadShadersOfType = [&](sf::base::StringView extension, sf::Shader::Type shaderType)
    {
        for (const auto& p : scanSingleByExt(mPath / "Shaders", extension))
        {
            sf::base::Optional<sf::Shader> shader;

            sf::base::String contents;
            if (sf::readFromFile(p, contents))
            {
                const sf::base::String migrated = migrateShader(contents);

                if (shaderType == sf::Shader::Type::Vertex)
                    shader = sf::Shader::loadFromMemory({.vertexCode = migrated});
                else if (shaderType == sf::Shader::Type::Fragment)
                    shader = sf::Shader::loadFromMemory({.fragmentCode = migrated});
                else if (shaderType == sf::Shader::Type::Geometry)
                    shader = sf::Shader::loadFromMemory({.geometryCode = migrated});
            }

            if (!shader.hasValue())
            {
                hg::lo("hg::loadPackAssets_loadShaders") << "Failed to load shader '" << p << "'\n";

                continue;
            }

            auto shaderUptr = sf::base::makeUnique<sf::Shader>(*SFML_BASE_MOVE(shader));

            shadersById.pushBack(shaderUptr.get());
            SSVOH_ASSERT(shadersById.size() > 0);
            const sf::base::SizeT shaderId = shadersById.size() - 1;

            LoadedShader ls{.shader{SFML_BASE_MOVE(shaderUptr)},
                            .path{p.to<sf::base::String>()},
                            .shaderType{shaderType},
                            .id{shaderId}};

            shaders.emplace(concatIntoBuf(mPackId, '_', p.getFilename().to<sf::base::String>()), SFML_BASE_MOVE(ls));

            shadersPathToId.emplace(p.to<sf::base::String>(), shaderId);

            ++loadInfo.assets;
        }
    };

    loadShadersOfType(".vert", sf::Shader::Type::Vertex);
    loadShadersOfType(".geom", sf::Shader::Type::Geometry);
    loadShadersOfType(".frag", sf::Shader::Type::Fragment);
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadCustomSounds(const sf::base::String& mPackId, const sf::Path& mPath)
{
    for (const sf::Path& p : scanSingleByExt(mPath / "Sounds", ".ogg"))
    {
        if (!assetStorage->loadSoundBuffer(concatIntoBuf(mPackId, '_', p.getFilename().to<sf::base::String>()),
                                           p.to<sf::base::String>()))
        {
            hg::lo("hg::loadPackAssets_loadCustomSounds") << "Failed to load sound buffer '" << p << "'\n";
        }

        ++loadInfo.assets;
    }
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadMusic(const sf::base::String& mPackId, const sf::Path& mPath)
{
    for (const sf::Path& p : scanSingleByExt(mPath / "Music", ".ogg"))
    {
        musicPathMap.emplace(concatIntoBuf(mPackId, '_', p.getStem().to<sf::base::String>()), p.to<sf::base::String>());

        ++loadInfo.assets;
    }
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadMusicData(const sf::base::String& mPackId, const sf::Path& mPath)
{
    for (const sf::Path& p : scanSingleByExt(mPath / "Music", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        MusicData musicData{Utils::loadMusicFromJson(object)};
        musicDataMap.emplace(concatIntoBuf(mPackId, '_', musicData.id), SFML_BASE_MOVE(musicData));

        ++loadInfo.assets;
    }
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadStyleData(const sf::base::String& mPackId, const sf::Path& mPath)
{
    for (const sf::Path& p : scanSingleByExt(mPath / "Styles", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        StyleData styleData{object};
        styleDataMap.emplace(concatIntoBuf(mPackId, '_', styleData.id), SFML_BASE_MOVE(styleData));

        ++loadInfo.assets;
    }
}

void HGAssets::HGAssetsImpl::loadPackAssets_loadLevelData(const sf::base::String& mPackId, const sf::Path& mPath)
{
    for (const sf::Path& p : scanSingleByExt(mPath / "Levels", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        LevelData               levelData{object, mPath, mPackId};
        const sf::base::String& assetId = concatIntoBuf(mPackId, '_', levelData.id);

        levelDataIdsByPack[mPackId].emplaceBack(assetId);
        levelDatas.emplace(assetId, sf::base::makeUnique<LevelData>(SFML_BASE_MOVE(levelData)));

        ++loadInfo.levels;
    }
}

//**********************************************
// PROFILE

void HGAssets::HGAssetsImpl::saveCurrentLocalProfile()
{
    if (currentProfilePtr == nullptr)
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

    const auto&                        favSet{getCurrentLocalProfile().getFavoriteLevelIds()};
    sf::base::Vector<sf::base::String> favorites;
    favorites.reserve(favSet.size());
    for (const auto& f : favSet)
        favorites.pushBack(f);
    ssvuj::arch(profileRoot, "favorites", favorites);

    ssvuj::writeToFile(profileRoot, sf::Path{getCurrentLocalProfileFilePath().cStr()});
}

void HGAssets::HGAssetsImpl::saveAllProfiles()
{
    ssvuj::Obj currentVersion;

    ssvuj::arch(currentVersion, "major", GAME_VERSION.major);
    ssvuj::arch(currentVersion, "minor", GAME_VERSION.minor);
    ssvuj::arch(currentVersion, "micro", GAME_VERSION.micro);

    sf::base::Vector<sf::base::String> favorites;

    for (const auto& [key, profileData] : profileDataMap)
    {
        ssvuj::Obj profileRoot;
        ssvuj::arch(profileRoot, "version", currentVersion);
        ssvuj::arch(profileRoot, "name", profileData->getName());
        ssvuj::arch(profileRoot, "scores", profileData->getScores());

        favorites.clear();
        for (const sf::base::String& favID : profileData->getFavoriteLevelIds())
        {
            favorites.emplaceBack(favID);
        }

        ssvuj::arch(profileRoot, "favorites", favorites);

        ssvuj::writeToFile(profileRoot, sf::Path{("Profiles/" + profileData->getName() + ".json").cStr()});
    }
}

//**********************************************
// GET

[[nodiscard]] const MusicData& HGAssets::HGAssetsImpl::getMusicData(const sf::base::String& mPackId, const sf::base::String& mId)
{
    const sf::base::String& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = musicDataMap.find(assetId);
    if (it == musicDataMap.end())
    {
        hg::lo("getMusicData") << "Asset '" << assetId << "' not found\n";

        SSVOH_ASSERT(!musicDataMap.empty());
        return musicDataMap.begin()->second;
    }

    return it->second;
}

[[nodiscard]] const StyleData& HGAssets::HGAssetsImpl::getStyleData(const sf::base::String& mPackId, const sf::base::String& mId)
{
    const sf::base::String& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = styleDataMap.find(assetId);
    if (it == styleDataMap.end())
    {
        hg::lo("getStyleData") << "Asset '" << assetId << "' not found\n";

        SSVOH_ASSERT(!styleDataMap.empty());
        return styleDataMap.begin()->second;
    }

    return it->second;
}

[[nodiscard]] sf::Shader* HGAssets::HGAssetsImpl::getShader(const sf::base::String& mPackId, const sf::base::String& mId)
{
    const sf::base::String& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = shaders.find(assetId);
    if (it == shaders.end())
    {
        hg::lo("getShader") << "Asset '" << assetId << "' not found\n";
        return nullptr;
    }

    return it->second.shader.get();
}

[[nodiscard]] sf::base::Optional<sf::base::SizeT> HGAssets::HGAssetsImpl::getShaderId(const sf::base::String& mPackId,
                                                                                      const sf::base::String& mId)
{
    const sf::base::String& assetId = concatIntoBuf(mPackId, '_', mId);

    const auto it = shaders.find(assetId);
    if (it == shaders.end())
    {
        hg::lo("getShaderId") << "Asset '" << assetId << "' not found\n";
        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(it->second.id);
}

[[nodiscard]] sf::base::Optional<sf::base::SizeT> HGAssets::HGAssetsImpl::getShaderIdByPath(const sf::base::String& mShaderPath)
{
    const auto it = shadersPathToId.find(mShaderPath);
    if (it == shadersPathToId.end())
    {
        hg::lo("getShaderIdByPath") << "Shader with path '" << mShaderPath << "' not found, couldn't get id\n";

        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(it->second);
}

[[nodiscard]] sf::Shader* HGAssets::HGAssetsImpl::getShaderByShaderId(const sf::base::SizeT mShaderId)
{
    if (!isValidShaderId(mShaderId))
    {
        return nullptr;
    }

    return shadersById[mShaderId];
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::isValidShaderId(const sf::base::SizeT mShaderId) const
{
    return mShaderId < shadersById.size();
}

//**********************************************
// RELOAD

void HGAssets::HGAssetsImpl::reloadAllShaders()
{
    for (auto& [id, loadedShader] : shaders)
    {
        sf::base::Optional<sf::Shader> reloadedShader;

        if (loadedShader.shaderType == sf::Shader::Type::Vertex)
            reloadedShader = sf::Shader::loadFromFile({.vertexPath = loadedShader.path});
        else if (loadedShader.shaderType == sf::Shader::Type::Fragment)
            reloadedShader = sf::Shader::loadFromFile({.fragmentPath = loadedShader.path});
        else if (loadedShader.shaderType == sf::Shader::Type::Geometry)
            reloadedShader = sf::Shader::loadFromFile({.geometryPath = loadedShader.path});

        if (!reloadedShader.hasValue())
        {
            hg::lo("hg::HGAssetsImplImpl::reloadAllShaders") << "Failed to load shader '" << loadedShader.path << "'\n";

            continue;
        }

        (*loadedShader.shader) = *SFML_BASE_MOVE(reloadedShader);
    }
}

[[nodiscard]] sf::base::String HGAssets::HGAssetsImpl::reloadPack(const sf::base::String& mPackId, const sf::Path& mPath)
{
    sf::base::String temp, output;

    // Levels, if there is no folder cancel everything
    if (!(mPath / "Levels").isDirectory())
    {
        return "invalid level folder path\n";
    }
    for (const sf::Path& p : scanSingleByExt(mPath / "Levels", ".json"))
    {
        LevelData levelData{ssvuj::getFromFile(p), mPath, mPackId};
        temp = mPackId + "_" + levelData.id;

        auto it = levelDatas.find(temp);
        if (it == levelDatas.end())
        {
            levelDataIdsByPack[mPackId].emplaceBack(temp);
            levelDatas.emplace(temp, sf::base::makeUnique<LevelData>(SFML_BASE_MOVE(levelData)));
        }
        else
        {
            *it->second = SFML_BASE_MOVE(levelData);
        }
    }
    output += "Levels successfully reloaded\n";

    // Styles
    if (!(mPath / "Styles").isDirectory())
    {
        output += "invalid style folder path\n";
    }
    else
    {
        for (const sf::Path& p : scanSingleByExt(mPath / "Styles", ".json"))
        {
            StyleData styleData{ssvuj::getFromFile(p)};
            temp = mPackId + "_" + styleData.id;

            styleDataMap[temp] = SFML_BASE_MOVE(styleData);
        }
        output += "Styles successfully reloaded\n";
    }

    // Music data
    if (!(mPath / "Music").isDirectory())
    {
        output += "invalid music data folder path\n";
    }
    else
    {
        for (const sf::Path& p : scanSingleByExt(mPath / "Music", ".json"))
        {
            MusicData musicData{Utils::loadMusicFromJson(ssvuj::getFromFile(p))};
            temp = mPackId + "_" + musicData.id;

            musicDataMap[temp] = SFML_BASE_MOVE(musicData);
        }
        output += "Music data successfully reloaded\n";
    }

    // Music
    if (!(mPath / "Music").isDirectory())
    {
        output += "invalid music folder path\n";
    }
    else
    {
        for (const sf::Path& p : scanSingleByExt(mPath / "Music", ".ogg"))
        {
            temp = mPackId + "_" + p.getStem().to<sf::base::String>();
            musicPathMap.emplace(temp, p.to<sf::base::String>());
        }
        output += "Music files successfully reloaded\n";
    }

    // Custom sounds
    if (!(mPath / "Sounds").isDirectory())
    {
        output += "invalid custom sound folder path\n";
    }
    else
    {
        for (const sf::Path& p : scanSingleByExt(mPath / "Sounds", ".ogg"))
        {
            temp = mPackId + "_" + p.getFilename().to<sf::base::String>();
            if (!assetStorage->loadSoundBuffer(temp, p.to<sf::base::String>()))
            {
                output += "Failed to load sound buffer '";
                output += p.to<sf::base::String>();
                output += "'\n";
            }
        }
        output += "Custom sound files successfully reloaded\n";
    }

    bumpPackListVersion();
    return output;
}

[[nodiscard]] sf::base::String HGAssets::HGAssetsImpl::reloadLevel(const sf::base::String& mPackId,
                                                                   const sf::Path&         mPath,
                                                                   const sf::base::String& mId)
{
    sf::base::String temp, output;

    //*******************************************
    // Level
    if (!(mPath / "Levels").isDirectory())
    {
        return "invalid level folder path\n";
    }

    const auto& levelFile = scanSingleByName(mPath / "Levels", (mId + ".json").cStr());
    if (levelFile.empty())
    {
        return "no matching level data file found\n";
    }

    // There is only one file, so we can just subscript index 0.
    // Same goes for all other files below
    LevelData levelData{ssvuj::getFromFile(levelFile[0]), mPath, mPackId};
    temp = mPackId + "_" + mId;

    auto it = levelDatas.find(temp);
    if (it == levelDatas.end())
    {
        levelDataIdsByPack[mPackId].emplaceBack(temp);
        levelDatas.emplace(temp, sf::base::makeUnique<LevelData>(SFML_BASE_MOVE(levelData)));
    }
    else
    {
        *it->second = levelData;
    }
    output = "level data " + mId + ".json successfully loaded\n";

    //*******************************************
    // Style
    if (!(mPath / "Styles").isDirectory())
    {
        output += "invalid style folder path\n";
    }
    else
    {
        const auto& styleFile = scanSingleByName(mPath / "Styles", (levelData.styleId + ".json").cStr());
        if (styleFile.empty())
        {
            output += "no matching style file found\n";
        }
        else
        {
            StyleData styleData{ssvuj::getFromFile(styleFile[0])};
            temp = mPackId + "_" + levelData.styleId;

            styleDataMap[temp] = SFML_BASE_MOVE(styleData);

            output += "style data " + levelData.styleId + ".json successfully loaded\n";
        }
    }

    //*******************************************
    // Music data
    if (!(mPath / "Music").isDirectory())
    {
        output += "invalid music folder path\n";
    }
    else
    {
        auto musicDataFile = scanSingleByName(mPath / "Music", (levelData.musicId + ".json").cStr());
        if (musicDataFile.empty())
        {
            output += "no matching music data file found\n";
        }
        else
        {
            MusicData musicData{Utils::loadMusicFromJson(ssvuj::getFromFile(musicDataFile[0]))};
            temp = mPackId + "_" + levelData.musicId;

            musicDataMap[temp] = SFML_BASE_MOVE(musicData);

            output += "music data " + levelData.musicId + ".json successfully loaded\n";
        }
    }

    //*******************************************
    // Music files
    sf::base::String assetId;
    if (!(mPath / "Music").isDirectory())
    {
        output += "invalid music folder path\n";
    }
    else if (levelData.musicId != "nullMusicId")
    {
        assetId = mPackId + "_" + levelData.musicId;

        const auto& musicFile = scanSingleByName(mPath / "Music", (levelData.musicId + ".ogg").cStr());
        if (musicFile.empty())
        {
            output += "no matching music file found\n";
        }
        else
        {
            musicPathMap.emplace(assetId, musicFile[0].to<sf::base::String>());
        }
    }

    //*******************************************
    // Sound files
    if (levelData.soundId == "nullSoundId")
    {
        // no need to keep going if sound id is null
        return output;
    }

    if (!(mPath / "Sounds").isDirectory())
    {
        output += "invalid custom sound folder path\n";
        return output;
    }

    // Check if this custom sound file is already loaded
    assetId = mPackId + "_" + levelData.soundId;
    if (assetStorage->hasSoundBuffer(assetId))
    {
        output += "custom sound file ";
        output += levelData.soundId;
        output += ".ogg is already loaded\n";

        return output;
    }

    const auto& soundFile = scanSingleByName(mPath / "Sounds", (levelData.soundId + ".ogg").cStr());
    if (soundFile.empty())
    {
        output += "no matching custom sound file found\n";
        return output;
    }

    if (!assetStorage->loadSoundBuffer(assetId, soundFile[0].to<sf::base::String>()))
    {
        output += "Failed to load sound buffer '";
        output += soundFile[0].to<sf::base::String>();
        output += "'\n";

        return output;
    }

    output += "new custom sound file ";
    output += levelData.soundId;
    output += ".ogg successfully loaded\n";

    bumpPackListVersion();
    return output;
}

//**********************************************
// HOT INSTALL

[[nodiscard]] sf::base::Optional<sf::base::String> HGAssets::HGAssetsImpl::installPackAtRuntime(const sf::Path& folderPath)
{
    // Validate the folder has a `pack.json` we can parse.
    if (!(folderPath / "pack.json").isRegularFile())
    {
        hg::lo("HGAssets::installPackAtRuntime") << "No pack.json under '" << folderPath << "'\n";
        return sf::base::nullOpt;
    }

    // Idempotency check. The Steam UGC subscribe path can deliver the
    // same pack to us via more than one callback in a single session
    // (e.g. `ItemInstalled_t` for the install + a synthesized event from
    // `on_item_subscribed` for an already-on-disk re-subscribe). Without
    // this guard, the second call appends duplicate entries to
    // `packInfos` / `selectablePackInfos` / `levelDataIdsByPack[id]` --
    // most of the underlying maps reject the duplicate
    // `emplace` silently, but the vector-keyed indexes don't, so the
    // pack ends up listed twice in the level select and every level
    // shows up twice within it.
    //
    // Match by `folderPath` because that's the only identifier we have
    // before the JSON parse runs. `sf::Path::operator==` does canonical
    // comparison, so trailing-separator differences don't matter.
    for (const auto& [pid, pdata] : packDatas)
    {
        if (pdata->folderPath == folderPath)
        {
            hg::lo("HGAssets::installPackAtRuntime")
                << "Pack at folder '" << folderPath << "' already loaded as '" << pid << "'; no-op\n";
            return sf::base::makeOptional(pid);
        }
    }

    // Load metadata + assets. `loadPackData` populates `packDatas` and
    // `packInfos`; `loadPackAssets` then populates `levelDatas` /
    // `styleDataMap` / `musicDataMap` / `selectablePackInfos`.
    if (!loadPackData(folderPath))
    {
        hg::lo("HGAssets::installPackAtRuntime") << "loadPackData failed for '" << folderPath << "'\n";
        return sf::base::nullOpt;
    }

    // The pack id was just inserted as the last element of `packInfos`. Use
    // it to fetch the corresponding `PackData` for `loadPackAssets`.
    const sf::base::String newPackId = packInfos.back().id;
    const auto             it        = packDatas.find(newPackId);
    if (it == packDatas.end())
    {
        hg::lo("HGAssets::installPackAtRuntime") << "PackData missing after loadPackData\n";
        return sf::base::nullOpt;
    }

    if (!loadPackAssets(*it->second, isHeadless()))
    {
        hg::lo("HGAssets::installPackAtRuntime") << "loadPackAssets failed for '" << newPackId << "'\n";
        return sf::base::nullOpt;
    }

    // Re-validate dependencies so a freshly-installed pack is recognized
    // (and any of its dependents that were previously missing become
    // available).
    if (!verifyAllPackDependencies())
    {
        hg::lo("HGAssets::installPackAtRuntime") << "verifyAllPackDependencies reported issues\n";
        // Don't fail outright -- `verifyAllPackDependencies` populates
        // `packIdsWithMissingDependencies` for the UI to surface.
    }

    // Re-sort selectablePackInfos by priority. (Matches the initial-load
    // sort at the end of `loadAllPackAssets`.)
    sf::base::quickSort(selectablePackInfos.begin(), selectablePackInfos.end(), [&](const PackInfo& a, const PackInfo& b) {
        return packDatas.at(a.id)->priority < packDatas.at(b.id)->priority;
    });

    bumpPackListVersion();
    return sf::base::makeOptional(newPackId);
}

[[nodiscard]] bool HGAssets::HGAssetsImpl::removePackAtRuntime(const sf::base::String& packId)
{
    // Mirror image of `installPackAtRuntime`: tear down every container
    // that the install path touched, in roughly the reverse order, then
    // run the same dependency-verification + selectable-list rebuild +
    // version-bump that the install does.
    //
    // The pack must currently be loaded -- if it isn't, the caller has a
    // stale id and we surface that as a bool failure rather than silent
    // success.
    if (packDatas.find(packId) == packDatas.end())
    {
        hg::lo("HGAssets::removePackAtRuntime") << "Unknown packId '" << packId << "'\n";
        return false;
    }

    // Build a "starts with `packId_`" predicate once. Used to sweep every
    // map keyed by `packId_<assetName>` (level/music/style/shader/etc.).
    sf::base::String prefixBuf;
    prefixBuf.reserve(packId.size() + 1);
    prefixBuf += packId;
    prefixBuf += '_';

    const auto startsWithPrefix = [&prefixBuf](const sf::base::String& key) noexcept
    { return key.size() >= prefixBuf.size() && std::memcmp(key.data(), prefixBuf.data(), prefixBuf.size()) == 0; };

    const auto sweepMap = [&](auto& map)
    {
        for (auto it = map.begin(); it != map.end();)
        {
            if (startsWithPrefix(it->first))
                it = map.erase(it);
            else
                ++it;
        }
    };

    // ------------------------------------------------------------------------
    // 1. Levels: erase every `levelDatas[packId_*]`, drop the per-pack index.
    sweepMap(levelDatas);
    levelDataIdsByPack.erase(packId);

    // ------------------------------------------------------------------------
    // 2. Music + style metadata.
    sweepMap(musicPathMap);
    sweepMap(musicDataMap);
    sweepMap(styleDataMap);

    // ------------------------------------------------------------------------
    // 3. Shaders. The vector `shadersById` is append-only by design (its
    //    indices are baked into LevelData / Lua scripts), so we null out
    //    the slots rather than reclaim them. `getShaderByShaderId` returns
    //    `nullptr` for a hole, which the rendering code already tolerates.
    for (auto it = shaders.begin(); it != shaders.end();)
    {
        if (startsWithPrefix(it->first))
        {
            const sf::base::SizeT id = it->second.id;
            if (id < shadersById.size())
                shadersById[id] = nullptr;

            // Wipe the path-to-id index entry that pointed at this shader.
            // (We can't reverse-lookup by id efficiently, so we sweep
            // `shadersPathToId` once at the end.)
            it = shaders.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Drop any `shadersPathToId` entry whose id now points to a null slot.
    for (auto it = shadersPathToId.begin(); it != shadersPathToId.end();)
    {
        const sf::base::SizeT id = it->second;
        if (id < shadersById.size() && shadersById[id] == nullptr)
            it = shadersPathToId.erase(it);
        else
            ++it;
    }

    // ------------------------------------------------------------------------
    // 4. AssetStorage (textures / fonts / sound buffers). Custom-sound load
    //    sites all key by `packId_*`; the global `Assets/` load uses bare
    //    asset names (no underscore prefix at all), so this sweep can't
    //    collide with them.
    assetStorage->removeByPackPrefix(prefixBuf);

    // ------------------------------------------------------------------------
    // 5. Pack metadata.
    packDatas.erase(packId);

    sf::base::vectorEraseIf(packInfos, [&](const PackInfo& pi) { return pi.id == packId; });
    sf::base::vectorEraseIf(selectablePackInfos, [&](const PackInfo& pi) { return pi.id == packId; });

    // The pack we removed may have been keeping `packIdsWithMissingDependencies`
    // up to date for itself; drop its entry so it doesn't shadow a later
    // re-install. Other packs that *depended on* this one will get re-flagged
    // by the upcoming `verifyAllPackDependencies` call.
    packIdsWithMissingDependencies.erase(packId);

    // ------------------------------------------------------------------------
    // 6. Active-profile favorites: drop ids that no longer resolve. (S3 in
    //    the design doc.) We sweep every loaded profile so cleanup is
    //    consistent regardless of which one is active.
    for (auto& [profileName, profileData] : profileDataMap)
    {
        Utils::erase_if(profileData->getFavoriteLevelIds(),
                        [this](const sf::base::String& favId) { return levelDatas.find(favId) == levelDatas.end(); });
    }

    // ------------------------------------------------------------------------
    // 8. Re-validate dependencies and re-sort the selectable list -- same
    //    final-pass the install path runs.
    if (!verifyAllPackDependencies())
    {
        hg::lo("HGAssets::removePackAtRuntime") << "verifyAllPackDependencies reported issues\n";
        // Same rationale as install: the function populates
        // `packIdsWithMissingDependencies` for the UI to surface.
    }

    sf::base::quickSort(selectablePackInfos.begin(), selectablePackInfos.end(), [&](const PackInfo& a, const PackInfo& b) {
        return packDatas.at(a.id)->priority < packDatas.at(b.id)->priority;
    });

    bumpPackListVersion();
    return true;
}

//**********************************************
// LOCAL SCORE

float HGAssets::HGAssetsImpl::getLocalScore(const sf::base::String& mId)
{
    return getCurrentLocalProfile().getScore(mId);
}

void HGAssets::HGAssetsImpl::setLocalScore(const sf::base::String& mId, float mScore)
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

ProfileData* HGAssets::HGAssetsImpl::getLocalProfileByName(const sf::base::String& mName)
{
    SSVOH_ASSERT(profileDataMap.contains(mName));
    return profileDataMap.find(mName)->second.get();
}

const ProfileData& HGAssets::HGAssetsImpl::getCurrentLocalProfile() const
{
    SSVOH_ASSERT(currentProfilePtr != nullptr);
    return *currentProfilePtr;
}

const ProfileData* HGAssets::HGAssetsImpl::getLocalProfileByName(const sf::base::String& mName) const
{
    SSVOH_ASSERT(profileDataMap.contains(mName));
    return profileDataMap.find(mName)->second.get();
}

[[nodiscard]] sf::base::String HGAssets::HGAssetsImpl::getCurrentLocalProfileFilePath()
{
    return "Profiles/" + currentProfilePtr->getName() + ".json";
}


[[nodiscard]] sf::base::SizeT HGAssets::HGAssetsImpl::getLocalProfilesSize()
{
    return profileDataMap.size();
}

[[nodiscard]] sf::base::Vector<sf::base::String> HGAssets::HGAssetsImpl::getLocalProfileNames()
{
    sf::base::Vector<sf::base::String> result;
    result.reserve(profileDataMap.size());

    for (auto& pair : profileDataMap)
    {
        result.emplaceBack(pair.second->getName());
    }

    return result;
}


[[nodiscard]] bool HGAssets::HGAssetsImpl::pIsValidLocalProfile() const
{
    return currentProfilePtr != nullptr;
}

[[nodiscard]] const sf::base::String& HGAssets::HGAssetsImpl::pGetName() const
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

void HGAssets::HGAssetsImpl::pSetCurrent(const sf::base::String& mName)
{
    const auto it = profileDataMap.find(mName);

    SSVOH_ASSERT(it != profileDataMap.end());
    currentProfilePtr = it->second.get();
}

void HGAssets::HGAssetsImpl::pCreate(const sf::base::String& mName)
{
    ssvuj::Obj root;
    ssvuj::arch(root, "name", mName);
    ssvuj::arch(root, "scores", ssvuj::Obj{});
    ssvuj::arch(root, "favorites", ssvuj::Obj{});
    ssvuj::writeToFile(root, ("Profiles/" + mName + ".json").cStr());

    profileDataMap.clear();

    if (!loadAllLocalProfiles())
    {
        hg::lo("HGAssets::HGAssets") << "Error loading local profiles\n";
        std::terminate();
        return;
    }
}

void HGAssets::HGAssetsImpl::pRemove(const sf::base::String& mName)
{
    profileDataMap.erase(mName);
}

[[nodiscard]] sf::SoundBuffer* HGAssets::HGAssetsImpl::getSoundBuffer(const sf::base::String& assetId)
{
    return assetStorage->getSoundBuffer(assetId);
}

[[nodiscard]] const sf::base::String* HGAssets::HGAssetsImpl::getMusicPath(const sf::base::String& assetId) const
{
    auto it = musicPathMap.find(assetId);
    return it == musicPathMap.end() ? nullptr : &it->second;
}

[[nodiscard]] const ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<LevelData>>& HGAssets::HGAssetsImpl::getLevelDatas() const noexcept
{
    return levelDatas;
}

[[nodiscard]] const ankerl::unordered_dense::set<sf::base::String>& HGAssets::HGAssetsImpl::getPackIdsWithMissingDependencies() const noexcept
{
    return packIdsWithMissingDependencies;
}

// ----------------------------------------------------------------------------

HGAssets::HGAssets(Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly) :
    _impl(sf::base::makeUnique<HGAssetsImpl>(mSteamManager, mHeadless, mLevelsOnly))
{
}

HGAssets::~HGAssets() = default;

LoadInfo& HGAssets::getLoadResults()
{
    return _impl->getLoadResults();
}

bool HGAssets::hasTexture(const sf::base::String& mId)
{
    return _impl->hasTexture(mId);
}

sf::Texture& HGAssets::getTexture(const sf::base::String& mId)
{
    return _impl->getTexture(mId);
}

sf::Font& HGAssets::getFont(const sf::base::String& mId)
{
    return _impl->getFont(mId);
}

bool HGAssets::isValidLevelId(const sf::base::String& mLevelId) const noexcept
{
    return _impl->isValidLevelId(mLevelId);
}

const LevelData& HGAssets::getLevelData(const sf::base::String& mAssetId) const
{
    return _impl->getLevelData(mAssetId);
}

bool HGAssets::packHasLevels(const sf::base::String& mPackId)
{
    return _impl->packHasLevels(mPackId);
}

const sf::base::Vector<sf::base::String>& HGAssets::getLevelIdsByPack(const sf::base::String& mPackId)
{
    return _impl->getLevelIdsByPack(mPackId);
}

const ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<PackData>>& HGAssets::getPackDatas()
{
    return _impl->getPackDatas();
}

bool HGAssets::isValidPackId(const sf::base::String& mPackId) const noexcept
{
    return _impl->isValidPackId(mPackId);
}

const PackData& HGAssets::getPackData(const sf::base::String& mPackId)
{
    return _impl->getPackData(mPackId);
}

const sf::base::Vector<PackInfo>& HGAssets::getSelectablePackInfos() const noexcept
{
    return _impl->getSelectablePackInfos();
}

sf::base::U64 HGAssets::packListVersion() const noexcept
{
    return _impl->getPackListVersion();
}

sf::base::Optional<sf::base::String> HGAssets::installPackAtRuntime(const sf::Path& folderPath)
{
    return _impl->installPackAtRuntime(folderPath);
}

bool HGAssets::removePackAtRuntime(const sf::base::String& packId)
{
    return _impl->removePackAtRuntime(packId);
}

const PackData* HGAssets::findPackData(const sf::base::String& mPackDisambiguator,
                                       const sf::base::String& mPackName,
                                       const sf::base::String& mPackAuthor) const noexcept
{
    return _impl->findPackData(mPackDisambiguator, mPackName, mPackAuthor);
}

const MusicData& HGAssets::getMusicData(const sf::base::String& mPackId, const sf::base::String& mId)
{
    return _impl->getMusicData(mPackId, mId);
}

const StyleData& HGAssets::getStyleData(const sf::base::String& mPackId, const sf::base::String& mId)
{
    return _impl->getStyleData(mPackId, mId);
}

sf::Shader* HGAssets::getShader(const sf::base::String& mPackId, const sf::base::String& mId)
{
    return _impl->getShader(mPackId, mId);
}

sf::base::Optional<sf::base::SizeT> HGAssets::getShaderId(const sf::base::String& mPackId, const sf::base::String& mId)
{
    return _impl->getShaderId(mPackId, mId);
}

sf::base::Optional<sf::base::SizeT> HGAssets::getShaderIdByPath(const sf::base::String& mShaderPath)
{
    return _impl->getShaderIdByPath(mShaderPath);
}

sf::Shader* HGAssets::getShaderByShaderId(const sf::base::SizeT mShaderId)
{
    return _impl->getShaderByShaderId(mShaderId);
}

bool HGAssets::isValidShaderId(const sf::base::SizeT mShaderId) const
{
    return _impl->isValidShaderId(mShaderId);
}

void HGAssets::reloadAllShaders()
{
    return _impl->reloadAllShaders();
}

sf::base::String HGAssets::reloadPack(const sf::base::String& mPackId, const sf::Path& mPath)
{
    return _impl->reloadPack(mPackId, mPath);
}

sf::base::String HGAssets::reloadLevel(const sf::base::String& mPackId, const sf::Path& mPath, const sf::base::String& mId)
{
    return _impl->reloadLevel(mPackId, mPath, mId);
}

float HGAssets::getLocalScore(const sf::base::String& mId)
{
    return _impl->getLocalScore(mId);
}

void HGAssets::setLocalScore(const sf::base::String& mId, float mScore)
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

ProfileData* HGAssets::getLocalProfileByName(const sf::base::String& mName)
{
    return _impl->getLocalProfileByName(mName);
}

const ProfileData* HGAssets::getLocalProfileByName(const sf::base::String& mName) const
{
    return _impl->getLocalProfileByName(mName);
}

sf::base::SizeT HGAssets::getLocalProfilesSize()
{
    return _impl->getLocalProfilesSize();
}

sf::base::Vector<sf::base::String> HGAssets::getLocalProfileNames()
{
    return _impl->getLocalProfileNames();
}

bool HGAssets::pIsValidLocalProfile() const
{
    return _impl->pIsValidLocalProfile();
}

const sf::base::String& HGAssets::pGetName() const
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

void HGAssets::pSetCurrent(const sf::base::String& mName)
{
    return _impl->pSetCurrent(mName);
}

void HGAssets::pCreate(const sf::base::String& mName)
{
    return _impl->pCreate(mName);
}

void HGAssets::pRemove(const sf::base::String& mName)
{
    return _impl->pRemove(mName);
}

sf::SoundBuffer* HGAssets::getSoundBuffer(const sf::base::String& assetId)
{
    return _impl->getSoundBuffer(assetId);
}

const sf::base::String* HGAssets::getMusicPath(const sf::base::String& assetId) const
{
    return _impl->getMusicPath(assetId);
}

const ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<LevelData>>& HGAssets::getLevelDatas() const noexcept
{
    return _impl->getLevelDatas();
}

const ankerl::unordered_dense::set<sf::base::String>& HGAssets::getPackIdsWithMissingDependencies() const noexcept
{
    return _impl->getPackIdsWithMissingDependencies();
}

void HGAssets::addLocalProfile(ProfileData&& profileData)
{
    return _impl->addLocalProfile(SFML_BASE_MOVE(profileData));
}

} // namespace hg
