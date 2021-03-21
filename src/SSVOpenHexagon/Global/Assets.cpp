// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"

#include <SSVStart/SoundPlayer/SoundPlayer.hpp>

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

namespace hg
{

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

HGAssets::HGAssets(Steam::steam_manager& mSteamManager, bool mLevelsOnly)
    : steamManager{mSteamManager}, levelsOnly{mLevelsOnly}
{
    if(!levelsOnly)
    {
        if(!ssvufs::Path{"Assets/"}.exists<ssvufs::Type::Folder>())
        {
            ssvu::lo("FATAL ERROR")
                << "Folder Assets/ does not exist" << std::endl;
            std::terminate();
        }
        else
        {
            auto [object, error] =
                ssvuj::getFromFileWithErrors("Assets/assets.json");
            loadAssetsFromJson(assetManager, "Assets/", object);
            loadInfo.addFormattedError(error);
        }
    }

    if(!loadAssets())
    {
        ssvu::lo("FATAL ERROR") << "Folder Packs/ does not exist" << std::endl;
        return;
    }

    for(auto& v : levelDataIdsByPack)
    {
        ssvu::sort(v.second, [&](const auto& mA, const auto& mB) {
            return levelDatas.at(mA).menuPriority <
                   levelDatas.at(mB).menuPriority;
        });
    }

    ssvu::sort(packInfos, [&](const auto& mA, const auto& mB) {
        return getPackData(mA.id).priority < getPackData(mB.id).priority;
    });

    // This will not be used for the rest of the game,
    // so shrink it to fit the actually used size.
    loadInfo.errorMessages.shrink_to_fit();
}

[[nodiscard]] bool HGAssets::loadPackData(const ssvufs::Path& packPath)
{
    const auto& packPathStr(packPath.getStr());

    // TODO: unused?
    /*
    std::string packLua;
    for(const auto& p : ssvufs::getScan<ssvufs::Mode::Recurse,
            ssvufs::Type::File, ssvufs::Pick::ByExt>(packPath, ".lua"))
    {
        packLua.append(p.getContentsAsStr());
    }
    */

    if(!ssvufs::Path{packPath + "/pack.json"}.exists<ssvufs::Type::File>())
    {
        return false;
    }

    auto [packRoot, error] =
        ssvuj::getFromFileWithErrors(packPath + "/pack.json");
    loadInfo.addFormattedError(error);

    const auto packDisambiguator =
        ssvuj::getExtr<std::string>(packRoot, "disambiguator", "");

    const auto packName =
        ssvuj::getExtr<std::string>(packRoot, "name", "unknown name");

    const auto packAuthor =
        ssvuj::getExtr<std::string>(packRoot, "author", "unknown author");

    const auto packDescription =
        ssvuj::getExtr<std::string>(packRoot, "description", "no description");

    const auto packVersion = ssvuj::getExtr<int>(packRoot, "version", 0);

    const auto packPriority = ssvuj::getExtr<float>(packRoot, "priority", 100);

    const auto packId = [&] {
        const auto spaceToUnderscore = [](std::string x) {
            for(char& c : x)
            {
                if(c == ' ' || c == '\n' || c == '\t')
                {
                    c = '_';
                }
            }

            return x;
        };

        std::string result;

        result += spaceToUnderscore(packDisambiguator);
        result += "_";
        result += spaceToUnderscore(packAuthor);
        result += "_";
        result += spaceToUnderscore(packName);
        result += "_";
        result += spaceToUnderscore(std::to_string(packVersion));

        return result;
    }();

    const PackData packData{packPathStr, packId, packDisambiguator, packName,
        packAuthor, packDescription, packVersion, packPriority};

    packDatas.emplace(packId, packData);

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
        if(!ssvufs::Path{packPath + "Music/"}.exists<ssvufs::Type::Folder>())
        {
            errorMessage = "Warning - " + packId + " has no 'Music' folder\n";
            loadInfo.errorMessages.emplace_back(errorMessage);

            ssvu::lo("::loadAssets") << errorMessage;
        }
        else if(!levelsOnly)
        {
            ssvu::lo("::loadAssets") << "loading " << packId << " music\n";
            loadMusic(packId, packPath);

            ssvu::lo("::loadAssets") << "loading " << packId << " music data\n";
            loadMusicData(packId, packPath);
        }

        if(!ssvufs::Path{packPath + "Styles/"}.exists<ssvufs::Type::Folder>())
        {
            errorMessage = "Warning - " + packId + " has no 'Styles' folder\n";
            loadInfo.errorMessages.emplace_back(errorMessage);

            ssvu::lo("::loadAssets") << errorMessage;
        }
        else
        {
            ssvu::lo("::loadAssets") << "loading " << packId << " style data\n";
            loadStyleData(packId, packPath);
        }

        if(!ssvufs::Path{packPath + "Levels/"}.exists<ssvufs::Type::Folder>())
        {
            errorMessage = "Warning - " + packId + " has no 'Levels' folder\n";
            loadInfo.errorMessages.emplace_back(errorMessage);

            ssvu::lo("::loadAssets") << errorMessage;
        }
        else
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
            "Exception during asset loading: " + std::string(mEx.what()) + "\n";
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

    return true;
}

//**********************************************
// LOAD

[[nodiscard]] bool HGAssets::loadAssets()
{
    if(!ssvufs::Path{"Packs/"}.exists<ssvufs::Type::Folder>())
    {
        return false;
    }

    std::string errorMessage;

    // ------------------------------------------------------------------------
    // Load packs from `Packs/` folder.
    for(const auto& packPath :
        ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::Folder>("Packs/"))
    {
        if(!loadPackData(packPath))
        {
            errorMessage =
                "Error loading pack data '" + packPath.getStr() + "'\n";
            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets") << errorMessage;
        }
        else
        {
            loadInfo.packs++;
        }
    }

    // ------------------------------------------------------------------------
    // Load packs from Steam workshop.
    steamManager.for_workshop_pack_folders([&](const std::string& folderPath) {
        const ssvufs::Path packPath{folderPath};

        if(!loadPackData(packPath))
        {
            errorMessage =
                "Error loading pack data '" + packPath.getStr() + "'\n";
            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets")
                << "Error loading pack data '" << packPath << "'\n";
        }
        else
        {
            loadInfo.packs++;
        }
    });

    // ------------------------------------------------------------------------
    // Load pack infos.
    for(auto& p : packDatas)
    {
        if(!loadPackInfo(p.second))
        {
            errorMessage = "Error loading pack info '" + p.first + "'\n";
            loadInfo.errorMessages.emplace_back(errorMessage);
            ssvu::lo("::loadAssets")
                << "Error loading pack info '" << p.first << "'\n";
        }
    }

    // ------------------------------------------------------------------------
    // Load profiles.
    ssvu::lo("::loadAssets") << "loading local profiles\n";
    loadLocalProfiles();

    return true;
}

void HGAssets::loadCustomSounds(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Sounds/", ".ogg"))
    {
        assetManager.load<sf::SoundBuffer>(mPackId + "_" + p.getFileName(), p);

        loadInfo.assets++;
    }
}
void HGAssets::loadMusic(const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".ogg"))
    {
        auto& music(assetManager.load<sf::SoundBuffer>(
            mPackId + "_" + p.getFileNameNoExtensions(), p));

        // TODO:
        // music.setVolume(Config::getMusicVolume());
        // music.setLoop(true);

        loadInfo.assets++;
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
            mPackId + "_" + musicData.id, std::move(musicData));

        loadInfo.assets++;
    }
}
void HGAssets::loadStyleData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
    {
        auto [object, error] = ssvuj::getFromFileWithErrors(p);
        loadInfo.addFormattedError(error);

        StyleData styleData{object, p};
        styleDataMap.emplace(
            mPackId + "_" + styleData.id, std::move(styleData));

        loadInfo.assets++;
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

        const std::string assetId = mPackId + "_" + levelData.id;
        levelDataIdsByPack[mPackId].emplace_back(assetId);
        levelDatas.emplace(assetId, std::move(levelData));

        loadInfo.levels++;
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

        ProfileData profileData{Utils::loadProfileFromJson(*this, object)};
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

    ssvuj::arch(currentVersion, "major", Config::GAME_VERSION.major);
    ssvuj::arch(currentVersion, "minor", Config::GAME_VERSION.minor);
    ssvuj::arch(currentVersion, "micro", Config::GAME_VERSION.micro);

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
    ssvuj::arch(currentVersion, "major", Config::GAME_VERSION.major);
    ssvuj::arch(currentVersion, "minor", Config::GAME_VERSION.minor);
    ssvuj::arch(currentVersion, "micro", Config::GAME_VERSION.micro);

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

const MusicData& HGAssets::getMusicData(
    const std::string& mPackId, const std::string& mId)
{
    const std::string assetId = mPackId + "_" + mId;

    const auto it = musicDataMap.find(assetId);
    if(it == musicDataMap.end())
    {
        ssvu::lo("getMusicData") << "Asset '" << assetId << "' not found\n";

        SSVU_ASSERT(!musicDataMap.empty());
        return musicDataMap.begin()->second;
    }

    return it->second;
}

const StyleData& HGAssets::getStyleData(
    const std::string& mPackId, const std::string& mId)
{
    const std::string assetId = mPackId + "_" + mId;

    const auto it = styleDataMap.find(assetId);
    if(it == styleDataMap.end())
    {
        ssvu::lo("getStyleData") << "Asset '" << assetId << "' not found\n";

        SSVU_ASSERT(!styleDataMap.empty());
        return styleDataMap.begin()->second;
    }

    return it->second;
}

//**********************************************
// RELOAD

std::string HGAssets::reloadPack(
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
            StyleData styleData{ssvuj::getFromFile(p), p};
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
            if(!assetManager.has<sf::SoundBuffer>(temp))
            {
                auto& music(assetManager.load<sf::SoundBuffer>(temp, p));
                // music.setVolume(Config::getMusicVolume());
                // music.setLoop(true);
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

std::string HGAssets::reloadLevel(const std::string& mPackId,
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
            StyleData styleData{ssvuj::getFromFile(styleFile[0]), styleFile[0]};
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
        if(assetManager.has<sf::SoundBuffer>(assetId))
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
                    assetManager.load<sf::SoundBuffer>(assetId, musicFile[0]));
                // music.setVolume(Config::getMusicVolume());
                // music.setLoop(true);
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
    assert(currentProfilePtr != nullptr);
    return *currentProfilePtr;
}

ProfileData* HGAssets::getLocalProfileByName(const std::string& mName)
{
    assert(profileDataMap.contains(mName));
    return &profileDataMap.find(mName)->second;
}

const ProfileData& HGAssets::getCurrentLocalProfile() const
{
    assert(currentProfilePtr != nullptr);
    return *currentProfilePtr;
}

const ProfileData* HGAssets::getLocalProfileByName(
    const std::string& mName) const
{
    assert(profileDataMap.contains(mName));
    return &profileDataMap.find(mName)->second;
}

std::string HGAssets::getCurrentLocalProfileFilePath()
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

std::size_t HGAssets::getLocalProfilesSize()
{
    return profileDataMap.size();
}

std::vector<std::string> HGAssets::getLocalProfileNames()
{
    std::vector<std::string> result;
    for(auto& pair : profileDataMap)
    {
        result.emplace_back(pair.second.getName());
    }
    return result;
}

std::string HGAssets::getFirstLocalProfileName()
{
    return begin(profileDataMap)->second.getName();
}

//**********************************************
// SOUND

void HGAssets::refreshVolumes()
{
    soundPlayer.setVolume(Config::getSoundVolume());
    // music.setVolume(Config::getMusicVolume());
}

void HGAssets::stopSounds()
{
    soundPlayer.stop();
}


void HGAssets::playSound(const std::string& mId, ssvs::SoundPlayer::Mode mMode)
{
    const auto assetId = mId;

    if(Config::getNoSound() || !assetManager.has<sf::SoundBuffer>(assetId))
    {
        return;
    }

    soundPlayer.play(assetManager.get<sf::SoundBuffer>(assetId), mMode);
}


void HGAssets::playPackSound(const std::string& mPackId, const std::string& mId,
    ssvs::SoundPlayer::Mode mMode)
{
    const auto assetId = mPackId + "_" + mId;

    if(Config::getNoSound() || !assetManager.has<sf::SoundBuffer>(assetId))
    {
        return;
    }

    soundPlayer.play(assetManager.get<sf::SoundBuffer>(assetId), mMode);
}

void HGAssets::playMusic(sf::Sound& music, const std::string& mPackId,
    const std::string& mId, sf::Time mPlayingOffset)
{
    const auto assetId = mPackId + "_" + mId;

    if(assetManager.has<sf::SoundBuffer>(assetId))
    {
        sf::SoundBuffer& buffer = assetManager.get<sf::SoundBuffer>(assetId);
        music.setBuffer(buffer);
        music.setPlayingOffset(mPlayingOffset);
        music.setLoop(true);
        music.play();
    }
}

} // namespace hg
