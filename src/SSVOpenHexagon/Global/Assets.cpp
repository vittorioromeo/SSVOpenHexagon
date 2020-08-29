// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"

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
        loadAssetsFromJson(
            assetManager, "Assets/", ssvuj::getFromFile("Assets/assets.json"));
    }

    if(!loadAssets())
    {
        ssvu::lo("FATAL ERROR")
            << "Error loading assets, exiting." << std::endl;

        std::terminate();
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

    const bool hasPackJson =
        ssvufs::Path{packPath + "/pack.json"}.exists<ssvufs::Type::File>();

    if(!hasPackJson)
    {
        ssvu::lo("::loadAssets")
            << "Warning - " << packPath << " has no 'pack.json' file\n";

        return false;
    }

    ssvuj::Obj packRoot{ssvuj::getFromFile(packPath + "/pack.json")};

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

    try
    {
        if(!ssvufs::Path{packPath + "Music/"}.exists<ssvufs::Type::Folder>())
        {
            ssvu::lo("::loadAssets")
                << "Warning - " << packId << " has no 'Music' folder\n";
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
            ssvu::lo("::loadAssets")
                << "Warning - " << packId << " has no 'Styles' folder\n";
        }
        else
        {
            ssvu::lo("::loadAssets") << "loading " << packId << " style data\n";
            loadStyleData(packId, packPath);
        }

        if(!ssvufs::Path{packPath + "Levels/"}.exists<ssvufs::Type::Folder>())
        {
            ssvu::lo("::loadAssets")
                << "Warning - " << packId << " has no 'Levels' folder\n";
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
        ssvu::lo("FATAL ERROR")
            << "Exception during asset loading: " << mEx.what() << std::endl;

        return false;
    }
    catch(...)
    {
        ssvu::lo("FATAL ERROR")
            << "Exception during asset loading: unknown." << std::endl;

        return false;
    }

    return true;
}

//**********************************************
// LOAD

[[nodiscard]] bool HGAssets::loadAssets()
{
    ssvu::lo("::loadAssets") << "loading local profiles\n";
    loadLocalProfiles();

    // ------------------------------------------------------------------------
    // Load packs from `Packs/` folder.
    for(const auto& packPath :
        ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::Folder>("Packs/"))
    {
        if(!loadPackData(packPath))
        {
            ssvu::lo("::loadAssets")
                << "Error loading pack data '" << packPath << "'\n";
        }
    }

    // ------------------------------------------------------------------------
    // Load packs from Steam workshop.
    steamManager.for_workshop_pack_folders([&](const std::string& folderPath) {
        const ssvufs::Path packPath{folderPath};

        if(!loadPackData(packPath))
        {
            ssvu::lo("::loadAssets")
                << "Error loading pack data '" << packPath << "'\n";
        }
    });

    // ------------------------------------------------------------------------
    // Load pack infos.
    for(auto& p : packDatas)
    {
        if(!loadPackInfo(p.second))
        {
            ssvu::lo("::loadAssets")
                << "Error loading pack info '" << p.first << "'\n";
        }
    }

    return true;
}

void HGAssets::loadCustomSounds(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Sounds/", ".ogg"))
    {
        assetManager.load<sf::SoundBuffer>(mPackId + "_" + p.getFileName(), p);
    }
}
void HGAssets::loadMusic(const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".ogg"))
    {
        auto& music(assetManager.load<sf::Music>(
            mPackId + "_" + p.getFileNameNoExtensions(), p));

        music.setVolume(Config::getMusicVolume());
        music.setLoop(true);
    }
}
void HGAssets::loadMusicData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".json"))
    {
        MusicData musicData{Utils::loadMusicFromJson(ssvuj::getFromFile(p))};
        musicDataMap.emplace(
            mPackId + "_" + musicData.id, std::move(musicData));
    }
}
void HGAssets::loadStyleData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
    {
        StyleData styleData{ssvuj::getFromFile(p), p};
        styleDataMap.emplace(
            mPackId + "_" + styleData.id, std::move(styleData));
    }
}
void HGAssets::loadLevelData(
    const std::string& mPackId, const ssvufs::Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Levels/", ".json"))
    {
        LevelData levelData{ssvuj::getFromFile(p), mPath, mPackId};

        const std::string assetId = mPackId + "_" + levelData.id;
        levelDataIdsByPack[mPackId].emplace_back(assetId);
        levelDatas.emplace(assetId, std::move(levelData));
    }
}

//**********************************************
// PROFILE

void HGAssets::loadLocalProfiles()
{
    for(const auto& p : scanSingleByExt("Profiles/", ".json"))
    {
        ProfileData profileData{
            Utils::loadProfileFromJson(ssvuj::getFromFile(p))};
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
    ssvuj::arch(profileRoot, "version", Config::getVersion());
    ssvuj::arch(profileRoot, "name", getCurrentLocalProfile().getName());
    ssvuj::arch(profileRoot, "scores", getCurrentLocalProfile().getScores());

    for(const auto& n : getCurrentLocalProfile().getTrackedNames())
    {
        profileRoot["trackedNames"].append(n);
    }

    ssvuj::writeToFile(profileRoot, getCurrentLocalProfileFilePath());
}

//**********************************************
// GET

const MusicData& SSVU_ATTRIBUTE(pure) HGAssets::getMusicData(
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

const StyleData& SSVU_ATTRIBUTE(pure) HGAssets::getStyleData(
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

std::string HGAssets::reloadLevelData(const std::string& mPackId, const std::string& mPath, const std::string& mId)
{
    // get the styles folder and check it exists
    const std::string path = mPath + "Levels/";
    if(!ssvufs::Path{path}.exists<ssvufs::Type::Folder>())
        return "invalid level folder path\n";

    // reload the style data
    for(const auto& p : scanSingleByName(path, mId + ".json"))
    {
        LevelData levelData{ssvuj::getFromFile(p), mPath, mPackId};
        levelDatas.find(mPackId + "_" + mId)->second = levelData;
        return "level data " + mId + ".json successfully loaded\n";
    }

    return "no matching level data file found\n";
}

std::string HGAssets::reloadMusicData(const std::string& mPackId, const std::string& mPath, const std::string& mId)
{
    // get the styles folder and check it exists
    const std::string path = mPath + "Music/";
    if(!ssvufs::Path{path}.exists<ssvufs::Type::Folder>())
        return "invalid music folder path\n";

    // reload the specific music data
    for(const auto& p : scanSingleByName(path, mId + ".json"))
    {
        // get pointer to updated map pair
        MusicData musicData{Utils::loadMusicFromJson(ssvuj::getFromFile(p))};
        musicDataMap.find(mPackId + "_" + mId)->second = musicData;
        return "music data " + mId + ".json successfully loaded\n";
    }

    return "no matching music data file found\n";
}

std::string HGAssets::reloadStyleData(const std::string& mPackId, const std::string& mPath, const std::string& mId)
{
    // get the styles folder and check it exists
    const std::string path = mPath + "Styles/";
    if(!ssvufs::Path{path}.exists<ssvufs::Type::Folder>())
        return "invalid style folder path\n";

    // reload the style data
    for(const auto& p : scanSingleByName(path, mId + ".json"))
    {
        // update the value
        StyleData styleData{ssvuj::getFromFile(p), p};
        styleDataMap.find(mPackId + "_" + mId)->second = styleData;
        return "style data " + mId + ".json successfully loaded\n";
    }

    return "no matching style data file found\n";
}

std::string HGAssets::reloadMusic(const std::string& mPackId, const std::string& mPath, const std::string& mId)
{
    // get the music folder and check it exists
    const std::string path = mPath + "Music/";
    if(!ssvufs::Path{path}.exists<ssvufs::Type::Folder>())
        return "invalid music folder path\n";

    // check if this music file is already loaded
    const std::string assetId = mPackId + "_" + mId;
    if(!assetManager.has<sf::SoundBuffer>(assetId))
        return "music file " + mId + ".ogg is already loaded\n";

    // load the new music file
    for(const auto& p : scanSingleByName(path, mId + ".ogg"))
    {
        auto& music(assetManager.load<sf::Music>(assetId, p));
        music.setVolume(Config::getMusicVolume());
        music.setLoop(true);
        return "new music file " + mId + ".ogg successfully loaded\n";
    }

    return "no matching music file found\n";
}

std::string HGAssets::reloadCustomSounds(const std::string& mPackId, const std::string& mPath, const std::string& mId)
{
    // get the sound folder and check it exists
    const std::string path = mPath + "Sounds/";
    if(!ssvufs::Path{path}.exists<ssvufs::Type::Folder>())
        return "invalid custom sound folder path\n";

    // check if this custom sound file is already loaded
    const std::string assetId = mPackId + "_" + mId;
    if(!assetManager.has<sf::SoundBuffer>(assetId))
        return "custom sound file " + mId + ".ogg is already loaded\n";

    for(const auto& p : scanSingleByName(path, mId + ".ogg"))
    {
        assetManager.load<sf::SoundBuffer>(assetId, p);
        return "new custom sound file " + mId + ".ogg successfully loaded\n";
    }

    return "no matching custom sound file found\n";
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

ProfileData& SSVU_ATTRIBUTE(pure) HGAssets::getCurrentLocalProfile()
{
    return *currentProfilePtr;
}

const ProfileData& SSVU_ATTRIBUTE(pure) HGAssets::getCurrentLocalProfile() const
{
    return *currentProfilePtr;
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
    ssvuj::writeToFile(root, "Profiles/" + mName + ".json");

    profileDataMap.clear();
    loadLocalProfiles();
}

std::size_t SSVU_ATTRIBUTE(pure) HGAssets::getLocalProfilesSize()
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
    musicPlayer.setVolume(Config::getMusicVolume());
}

void HGAssets::stopMusics()
{
    musicPlayer.stop();
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

void HGAssets::playMusic(
    const std::string& mPackId, const std::string& mId, sf::Time mPlayingOffset)
{
    const auto assetId = mPackId + "_" + mId;

    if(assetManager.has<sf::Music>(assetId))
    {
        musicPlayer.play(assetManager.get<sf::Music>(assetId), mPlayingOffset);
    }
}

} // namespace hg
