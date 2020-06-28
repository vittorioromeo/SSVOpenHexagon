// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/Definitions.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"

namespace hg
{

[[nodiscard]] static auto scanSingleByExt(
    const ssvufs::Path& path, const std::string& extension)
{
    return ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::File,
        ssvufs::Pick::ByExt>(path, extension);
}

HGAssets::HGAssets(bool mLevelsOnly) : levelsOnly{mLevelsOnly}
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
        return packDatas.at(mA.id).priority < packDatas.at(mB.id).priority;
    });
}

[[nodiscard]] bool HGAssets::loadAssets()
{
    ssvu::lo("::loadAssets") << "loading local profiles\n";
    loadLocalProfiles();

    for(const auto& packPath :
        ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::Folder>("Packs/"))
    {
        const auto& packPathStr(packPath.getStr());
        std::string packName{packPathStr.substr(6, packPathStr.size() - 7)};

        std::string packLua;
        for(const auto& p : ssvufs::getScan<ssvufs::Mode::Recurse,
                ssvufs::Type::File, ssvufs::Pick::ByExt>(packPath, ".lua"))
        {
            packLua.append(p.getContentsAsStr());
        }

        ssvuj::Obj packRoot{ssvuj::getFromFile(packPath + "/pack.json")};
        packDatas.emplace(packName,
            PackData{packName, ssvuj::getExtr<std::string>(packRoot, "name"),
                ssvuj::getExtr<float>(packRoot, "priority")});
    }

    for(auto& p : packDatas)
    {
        const auto& pd(p.second);
        const std::string& packId{pd.id};

        std::string packPath{"Packs/" + packId + "/"};
        packInfos.emplace_back(PackInfo{packId, packPath});

        try
        {
            const bool hasMusicFolder = ssvufs::Path{packPath + "Music/"}
                                            .exists<ssvufs::Type::Folder>();

            if(!hasMusicFolder)
            {
                ssvu::lo("::loadAssets")
                    << "Warning - " << packId << " has no 'Music' folder\n";
            }

            if(!levelsOnly && hasMusicFolder)
            {
                ssvu::lo("::loadAssets") << "loading " << packId << " music\n";
                loadMusic(packPath);

                ssvu::lo("::loadAssets")
                    << "loading " << packId << " music data\n";
                loadMusicData(packPath);
            }

            ssvu::lo("::loadAssets") << "loading " << packId << " style data\n";
            loadStyleData(packPath);

            ssvu::lo("::loadAssets") << "loading " << packId << " level data\n";
            loadLevelData(packPath);

            if(!levelsOnly &&
                Path(packPath + "Sounds/").exists<ssvufs::Type::Folder>())
            {
                ssvu::lo("::loadAssets")
                    << "loading " << packId << " custom sounds\n";
                loadCustomSounds(packId, packPath);
            }
        }
        catch(const std::runtime_error& mEx)
        {
            ssvu::lo("FATAL ERROR")
                << "Exception during asset loading: " << mEx.what()
                << std::endl;

            return false;
        }
        catch(...)
        {
            ssvu::lo("FATAL ERROR")
                << "Exception during asset loading: unknown." << std::endl;

            return false;
        }
    }

    return true;
}

void HGAssets::loadCustomSounds(const std::string& mPackName, const Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Sounds/", ".ogg"))
    {
        assetManager.load<sf::SoundBuffer>(
            mPackName + "_" + p.getFileName(), p);
    }
}
void HGAssets::loadMusic(const Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".ogg"))
    {
        auto& music(
            assetManager.load<sf::Music>(p.getFileNameNoExtensions(), p));
        music.setVolume(Config::getMusicVolume());
        music.setLoop(true);
    }
}
void HGAssets::loadMusicData(const Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Music/", ".json"))
    {
        MusicData musicData{Utils::loadMusicFromJson(ssvuj::getFromFile(p))};
        musicDataMap.emplace(musicData.id, std::move(musicData));
    }
}
void HGAssets::loadStyleData(const Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Styles/", ".json"))
    {
        StyleData styleData{ssvuj::getFromFile(p), p};
        styleDataMap.emplace(styleData.id, std::move(styleData));
    }
}
void HGAssets::loadLevelData(const Path& mPath)
{
    for(const auto& p : scanSingleByExt(mPath + "Levels/", ".json"))
    {
        LevelData levelData{ssvuj::getFromFile(p), mPath};
        levelDataIdsByPack[levelData.packPath].emplace_back(levelData.id);
        levelDatas.emplace(levelData.id, std::move(levelData));
    }
}
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

const MusicData& SSVU_ATTRIBUTE(pure) HGAssets::getMusicData(
    const std::string& mId)
{
    return musicDataMap.find(mId)->second;
}

const StyleData& SSVU_ATTRIBUTE(pure) HGAssets::getStyleData(
    const std::string& mId)
{
    return styleDataMap.find(mId)->second;
}

float HGAssets::getLocalScore(const std::string& mId)
{
    return getCurrentLocalProfile().getScore(mId);
}

void HGAssets::setLocalScore(const std::string& mId, float mScore)
{
    getCurrentLocalProfile().setScore(mId, mScore);
}

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
    if(Config::getNoSound() || !assetManager.has<sf::SoundBuffer>(mId))
    {
        return;
    }

    soundPlayer.play(assetManager.get<sf::SoundBuffer>(mId), mMode);
}

void HGAssets::playMusic(const std::string& mId, sf::Time mPlayingOffset)
{
    if(assetManager.has<sf::Music>(mId))
    {
        musicPlayer.play(assetManager.get<sf::Music>(mId), mPlayingOffset);
    }
}

} // namespace hg
