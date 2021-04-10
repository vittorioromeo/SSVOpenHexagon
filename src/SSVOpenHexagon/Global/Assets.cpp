// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Utils/LoadFromJson.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/BuildPackId.hpp"
#include "SSVOpenHexagon/Utils/EraseIf.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"

#include <SSVStart/SoundPlayer/SoundPlayer.hpp>

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

namespace hg {

[[nodiscard]] static auto scanSingleByExt(
    const ssvufs::Path& path, const std::string& extension)
{
    return ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::File,
        ssvufs::Pick::ByExt>(path, extension);
}

[[nodiscard]] static auto scanSingleByName(
    const ssvufs::Path& path, const std::string& name)
{
    return ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::File,
        ssvufs::Pick::ByName>(path, name);
}

HGAssets::HGAssets(
    Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly)
    : steamManager{mSteamManager}, levelsOnly{mLevelsOnly}
{
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

        loadAssetsFromJson(assetManager, "Assets/", object);

        loadInfo.addFormattedError(error);
    }

    if(!loadAssets())
    {
        std::terminate();
        return;
    }

    for(auto& v : levelDataIdsByPack)
    {
        ssvu::sort(v.second,
            [&](const auto& mA, const auto& mB) {
                return levelDatas.at(mA).menuPriority <
                       levelDatas.at(mB).menuPriority;
            });
    }

    ssvu::sort(packInfos, [&](const auto& mA, const auto& mB)
        { return getPackData(mA.id).priority < getPackData(mB.id).priority; });

    ssvu::sort(selectablePackInfos, [&](const auto& mA, const auto& mB)
        { return getPackData(mA.id).priority < getPackData(mB.id).priority; });

    // This will not be used for the rest of the game,
    // so shrink it to fit the actually used size.
    loadInfo.errorMessages.shrink_to_fit();
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

    auto [packRoot, error] =
        ssvuj::getFromFileWithErrors(packPath + "/pack.json");

    loadInfo.addFormattedError(error);

    auto packDisambiguator =
        ssvuj::getExtr<std::string>(packRoot, "disambiguator", "");

    auto packName =
        ssvuj::getExtr<std::string>(packRoot, "name", "unknown name");

    auto packAuthor =
        ssvuj::getExtr<std::string>(packRoot, "author", "unknown author");

    auto packDescription =
        ssvuj::getExtr<std::string>(packRoot, "description", "no description");

    const auto packVersion = ssvuj::getExtr<int>(packRoot, "version", 0);

    const auto packPriority = ssvuj::getExtr<float>(packRoot, "priority", 100);

    // TODO (P0): what to do with packs without a disambiguator?
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

[[nodiscard]] bool HGAssets::loadPackInfo(const PackData& packData)
{
    const std::string& packPath{packData.folderPath};
    const std::string& packId{packData.id};

    packInfos.emplace_back(PackInfo{packId, packPath});

    std::string errorMessage;

    try
    {
        if(ssvufs::Path{packPath + "Music/"}.exists<ssvufs::Type::Folder>() &&
            !levelsOnly)
        {
            ssvu::lo("::loadAssets") << "loading " << packId << " music\n";
            loadMusic(packId, packPath);

            ssvu::lo("::loadAssets") << "loading " << packId << " music data\n";
            loadMusicData(packId, packPath);
        }

        if(ssvufs::Path{packPath + "Styles/"}.exists<ssvufs::Type::Folder>())
        {
            ssvu::lo("::loadAssets") << "loading " << packId << " style data\n";
            loadStyleData(packId, packPath);
        }

        if(ssvufs::Path{packPath + "Levels/"}.exists<ssvufs::Type::Folder>())
        {
            ssvu::lo("::loadAssets") << "loading " << packId << " level data\n";
            loadLevelData(packId, packPath);
        }

        if(!levelsOnly &&
            ssvufs::Path(packPath + "Sounds/").exists<ssvufs::Type::Folder>())
        {
            ssvu::lo("::loadAssets")
                << "loading " << packId << " custom sounds\n";
            loadCustomSounds(packId, packPath);
        }
    }
    catch(const std::runtime_error& mEx)
    {
        errorMessage =
            Utils::concat("Exception during asset loading: ", mEx.what(), '\n');

        loadInfo.errorMessages.emplace_back("FATAL ERROR, " + errorMessage);
        ssvu::lo("FATAL ERROR") << errorMessage;
        return false;
    }
    catch(...)
    {
        errorMessage = "Exception during asset loading: unknown.\n";

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

[[nodiscard]] auto& HGAssets::operator()()
{
    return assetManager;
}

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] T& HGAssets::get(const std::string& mId)
{
    return assetManager.get<T>(mId);
}

template sf::Texture& HGAssets::get(const std::string& mId);
template sf::Font& HGAssets::get(const std::string& mId);

// ----------------------------------------------------------------------------

[[nodiscard]] const std::unordered_map<std::string, LevelData>&
HGAssets::getLevelDatas()
{
    return levelDatas;
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
HGAssets::getPacksData()
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
HGAssets::getPackInfos() const noexcept
{
    return packInfos;
}

[[nodiscard]] const std::vector<PackInfo>&
HGAssets::getSelectablePackInfos() const noexcept
{
    return selectablePackInfos;
}

[[nodiscard]] const std::unordered_map<std::string, PackData>&
HGAssets::getPackDatas() const noexcept
{
    return packDatas;
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

[[nodiscard]] bool HGAssets::loadAssets()
{
    if(!ssvufs::Path{"Packs/"}.exists<ssvufs::Type::Folder>())
    {
        ssvu::lo("::loadAssets") << "Folder Packs/ does not exist" << std::endl;
        return false;
    }

    std::string errorMessage;

    // ------------------------------------------------------------------------
    const auto tryLoadPackFromPath = [&](const auto& packPath)
    {
        if(!loadPackData(packPath))
        {
            errorMessage =
                Utils::concat("Error loading pack data '", packPath, '\n');

            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets") << errorMessage;
        }
        else
        {
            ++loadInfo.packs;
        }
    };

    // ------------------------------------------------------------------------
    // Load packs from `Packs/` folder.
    for(const auto& packPath :
        ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::Folder>("Packs/"))
    {
        tryLoadPackFromPath(packPath);
    }

    // ------------------------------------------------------------------------
    // Load packs from Steam workshop.
    if(steamManager != nullptr)
    {
        steamManager->for_workshop_pack_folders(tryLoadPackFromPath);
    }

    // ------------------------------------------------------------------------
    // Load pack infos.
    for(const auto& [packId, packData] : packDatas)
    {
        if(!loadPackInfo(packData))
        {
            errorMessage =
                Utils::concat("Error loading pack info '", packId, '\n');

            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets") << errorMessage;
        }
    }

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

    std::unordered_set<std::string> packIdsWithMissingDependencies;

    for(const auto& [packId, packData] : packDatas)
    {
        for(const PackDependency& pd : packData.dependencies)
        {
            if(dependencyExists(pd))
            {
                continue;
            }

            errorMessage = Utils::concat("Missing pack dependency '", pd.name,
                "' for pack '", packData.name, "'\n");

            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets") << errorMessage;

            packIdsWithMissingDependencies.emplace(packId);
        }
    }

    ssvu::eraseRemoveIf(selectablePackInfos, [&](const PackInfo& pi)
        { return packIdsWithMissingDependencies.contains(pi.id); });

    // ------------------------------------------------------------------------
    // Load profiles.
    if(!ssvufs::Path{"Profiles/"}.exists<ssvufs::Type::Folder>())
    {
        ssvu::lo("::loadAssets")
            << "Folder Profiles/ does not exist" << std::endl;
    }
    else
    {
        ssvu::lo("::loadAssets") << "loading local profiles\n";
        loadLocalProfiles();
    }

    return true;
}

void HGAssets::loadCustomSounds(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Sounds/", ".ogg"))
    {
        assetManager.load<sf::SoundBuffer>(
            Utils::concat(mPackId, '_', p.getFileName()), p);

        ++loadInfo.assets;
    }
}

void HGAssets::loadMusic(const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".ogg"))
    {
        auto& music(assetManager.load<sf::Music>(
            Utils::concat(mPackId, '_', p.getFileNameNoExtensions()), p));

        music.setLoop(true);

        ++loadInfo.assets;
    }
}

void HGAssets::loadMusicData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        MusicData musicData{Utils::loadMusicFromJson(object)};
        musicDataMap.emplace(
            Utils::concat(mPackId, '_', musicData.id), std::move(musicData));

        ++loadInfo.assets;
    }
}

void HGAssets::loadStyleData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        StyleData styleData{object};
        styleDataMap.emplace(
            Utils::concat(mPackId, '_', styleData.id), std::move(styleData));

        ++loadInfo.assets;
    }
}

void HGAssets::loadLevelData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Levels/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        LevelData levelData{object, mPath, mPackId};
        std::string assetId = Utils::concat(mPackId, '_', levelData.id);

        levelDataIdsByPack[mPackId].emplace_back(assetId);
        levelDatas.emplace(std::move(assetId), std::move(levelData));

        ++loadInfo.levels;
    }
}

//**********************************************
// PROFILE

void HGAssets::loadLocalProfiles()
{
    for(const auto& p : scanSingleByExt("Profiles/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        ProfileData profileData{Utils::loadProfileFromJson(object)};

        // Remove invalid level ids that might have been added to the files.
        Utils::erase_if(profileData.getFavoriteLevelIds(),
            [this](const std::string& favId)
            { return levelDatas.find(favId) == levelDatas.end(); });

        profileDataMap.emplace(profileData.getName(), std::move(profileData));
    }
}

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

    for(const auto& n : getCurrentLocalProfile().getTrackedNames())
    {
        profileRoot["trackedNames"].append(n);
    }

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

        for(const auto& n : profileData.getTrackedNames())
        {
            profileRoot["trackedNames"].append(n);
        }

        ssvuj::writeToFile(
            profileRoot, "Profiles/" + profileData.getName() + ".json");
    }
}

//**********************************************
// GET

[[nodiscard]] const MusicData& HGAssets::getMusicData(
    const std::string& mPackId, const std::string& mId)
{
    const std::string assetId = mPackId + "_" + mId;

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
    const std::string assetId = mPackId + "_" + mId;

    const auto it = styleDataMap.find(assetId);
    if(it == styleDataMap.end())
    {
        ssvu::lo("getStyleData") << "Asset '" << assetId << "' not found\n";

        SSVOH_ASSERT(!styleDataMap.empty());
        return styleDataMap.begin()->second;
    }

    return it->second;
}

//**********************************************
// RELOAD

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
            if(!assetManager.has<sf::Music>(temp))
            {
                auto& music(assetManager.load<sf::Music>(temp, p));
                music.setLoop(true);
            }
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
            if(!assetManager.has<sf::SoundBuffer>(temp))
            {
                assetManager.load<sf::SoundBuffer>(temp, p);
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
        if(assetManager.has<sf::Music>(assetId))
        {
            output +=
                "music file " + levelData.musicId + ".ogg is already loaded\n";
        }
        else
        {
            auto musicFile = scanSingleByName(temp, levelData.musicId + ".ogg");
            if(musicFile.empty())
            {
                output += "no matching music file found\n";
            }
            else
            {
                auto& music(
                    assetManager.load<sf::Music>(assetId, musicFile[0]));
                music.setLoop(true);
                output += "new music file " + levelData.musicId +
                          ".ogg successfully loaded\n";
            }
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
    if(assetManager.has<sf::SoundBuffer>(assetId))
    {
        output += "custom sound file " + levelData.soundId +
                  ".ogg is already loaded\n";
        return output;
    }

    auto soundFile = scanSingleByName(temp, levelData.soundId + ".ogg");
    if(soundFile.empty())
    {
        output += "no matching custom sound file found\n";
        return output;
    }

    assetManager.load<sf::SoundBuffer>(assetId, soundFile[0]);
    output += "new custom sound file " + levelData.soundId +
              ".ogg successfully loaded\n";

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

void HGAssets::setCurrentLocalProfile(const std::string& mName)
{
    currentProfilePtr = &profileDataMap.find(mName)->second;
}

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

void HGAssets::createLocalProfile(const std::string& mName)
{
    ssvuj::Obj root;
    ssvuj::arch(root, "name", mName);
    ssvuj::arch(root, "scores", ssvuj::Obj{});
    ssvuj::arch(root, "favorites", ssvuj::Obj{});
    ssvuj::writeToFile(root, "Profiles/" + mName + ".json");

    profileDataMap.clear();
    loadLocalProfiles();
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

[[nodiscard]] std::string HGAssets::pGetName() const
{
    return getCurrentLocalProfile().getName();
}

[[nodiscard]] const std::vector<std::string>& HGAssets::pGetTrackedNames() const
{
    return getCurrentLocalProfile().getTrackedNames();
}

void HGAssets::pClearTrackedNames()
{
    getCurrentLocalProfile().clearTrackedNames();
}

void HGAssets::pAddTrackedName(const std::string& mName)
{
    getCurrentLocalProfile().addTrackedName(mName);
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
    setCurrentLocalProfile(mName);
}

void HGAssets::pCreate(const std::string& mName)
{
    createLocalProfile(mName);
}

[[nodiscard]] sf::SoundBuffer* HGAssets::getSoundBuffer(
    const std::string& assetId)
{
    // TODO (P2): remove assetmanager
    if(!assetManager.has<sf::SoundBuffer>(assetId))
    {
        return nullptr;
    }

    return &assetManager.get<sf::SoundBuffer>(assetId);
}

[[nodiscard]] sf::Music* HGAssets::getMusic(const std::string& assetId)
{
    // TODO (P2): remove assetmanager
    if(!assetManager.has<sf::Music>(assetId))
    {
        return nullptr;
    }

    return &assetManager.get<sf::Music>(assetId);
}

} // namespace hg
