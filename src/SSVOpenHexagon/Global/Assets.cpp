// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/Definitions.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;
using namespace ssvu::FileSystem;

namespace hg
{

HGAssets::HGAssets(bool mLevelsOnly) : levelsOnly{mLevelsOnly}
{
    if(!levelsOnly)
    {
        loadAssetsFromJson(
            assetManager, "Assets/", getFromFile("Assets/assets.json"));
    }

    loadAssets();

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

void HGAssets::loadAssets()
{
    lo("::loadAssets") << "loading local profiles\n";
    loadLocalProfiles();

    for(const auto& packPath : getScan<Mode::Single, Type::Folder>("Packs/"))
    {
        const auto& packPathStr(packPath.getStr());
        string packName{packPathStr.substr(6, packPathStr.size() - 7)};

        string packLua;
        for(const auto& p :
            getScan<Mode::Recurse, Type::File, Pick::ByExt>(packPath, ".lua"))
        {
            packLua.append(p.getContentsAsStr());
        }

        ssvuj::Obj packRoot{getFromFile(packPath + "/pack.json")};
        packDatas.emplace(
            packName, PackData{packName, getExtr<string>(packRoot, "name"),
                          getExtr<float>(packRoot, "priority")});
    }

    for(auto& p : packDatas)
    {
        const auto& pd(p.second);
        const string& packId{pd.id};

        string packPath{"Packs/" + packId + "/"};
        packInfos.emplace_back(PackInfo{packId, packPath});

        try
        {
            if(!levelsOnly)
            {
                lo("::loadAssets") << "loading " << packId << " music\n";
                loadMusic(packPath);

                lo("::loadAssets") << "loading " << packId << " music data\n";
                loadMusicData(packPath);
            }

            lo("::loadAssets") << "loading " << packId << " style data\n";
            loadStyleData(packPath);

            lo("::loadAssets") << "loading " << packId << " level data\n";
            loadLevelData(packPath);

            if(!levelsOnly &&
                Path(packPath + "Sounds/").exists<ssvufs::Type::Folder>())
            {
                lo("::loadAssets")
                    << "loading " << packId << " custom sounds\n";
                loadCustomSounds(packId, packPath);
            }
        }
        catch(const std::runtime_error& mEx)
        {
            ssvu::lo("FATAL ERROR")
                << "Exception during asset loading: " << mEx.what()
                << std::endl;
        }
        catch(...)
        {
            ssvu::lo("FATAL ERROR")
                << "Exception during asset loading: unknown." << std::endl;
        }

        lo().flush();
    }
}

void HGAssets::loadCustomSounds(const string& mPackName, const Path& mPath)
{
    for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(
            mPath + "Sounds/", ".ogg"))
    {
        assetManager.load<SoundBuffer>(mPackName + "_" + p.getFileName(), p);
    }
}
void HGAssets::loadMusic(const Path& mPath)
{
    for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(
            mPath + "Music/", ".ogg"))
    {
        auto& music(assetManager.load<Music>(p.getFileNameNoExtensions(), p));
        music.setVolume(Config::getMusicVolume());
        music.setLoop(true);
    }
}
void HGAssets::loadMusicData(const Path& mPath)
{
    for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(
            mPath + "Music/", ".json"))
    {
        MusicData musicData{loadMusicFromJson(getFromFile(p))};
        musicDataMap.emplace(musicData.id, std::move(musicData));
    }
}
void HGAssets::loadStyleData(const Path& mPath)
{
    for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(
            mPath + "Styles/", ".json"))
    {
        StyleData styleData{getFromFile(p), p};
        styleDataMap.emplace(styleData.id, std::move(styleData));
    }
}
void HGAssets::loadLevelData(const Path& mPath)
{
    for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(
            mPath + "Levels/", ".json"))
    {
        LevelData levelData{getFromFile(p), mPath};
        levelDataIdsByPack[levelData.packPath].emplace_back(levelData.id);
        levelDatas.emplace(levelData.id, std::move(levelData));
    }
}
void HGAssets::loadLocalProfiles()
{
    for(const auto& p :
        getScan<Mode::Single, Type::File, Pick::ByExt>("Profiles/", ".json"))
    {
        ProfileData profileData{loadProfileFromJson(getFromFile(p))};
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

const MusicData& SSVU_ATTRIBUTE(pure) HGAssets::getMusicData(const string& mId)
{
    return musicDataMap.find(mId)->second;
}

const StyleData& SSVU_ATTRIBUTE(pure) HGAssets::getStyleData(const string& mId)
{
    return styleDataMap.find(mId)->second;
}

float HGAssets::getLocalScore(const string& mId)
{
    return getCurrentLocalProfile().getScore(mId);
}

void HGAssets::setLocalScore(const string& mId, float mScore)
{
    getCurrentLocalProfile().setScore(mId, mScore);
}

void HGAssets::setCurrentLocalProfile(const string& mName)
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

string HGAssets::getCurrentLocalProfileFilePath()
{
    return "Profiles/" + currentProfilePtr->getName() + ".json";
}

void HGAssets::createLocalProfile(const string& mName)
{
    ssvuj::Obj root;
    ssvuj::arch(root, "name", mName);
    ssvuj::arch(root, "scores", ssvuj::Obj{});
    ssvuj::writeToFile(root, "Profiles/" + mName + ".json");

    profileDataMap.clear();
    loadLocalProfiles();
}

SizeT SSVU_ATTRIBUTE(pure) HGAssets::getLocalProfilesSize()
{
    return profileDataMap.size();
}

vector<string> HGAssets::getLocalProfileNames()
{
    vector<string> result;
    for(auto& pair : profileDataMap)
    {
        result.emplace_back(pair.second.getName());
    }
    return result;
}

string HGAssets::getFirstLocalProfileName()
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

void HGAssets::playSound(const string& mId, SoundPlayer::Mode mMode)
{
    if(Config::getNoSound() || !assetManager.has<SoundBuffer>(mId))
    {
        return;
    }

    soundPlayer.play(assetManager.get<SoundBuffer>(mId), mMode);
}

void HGAssets::playMusic(const string& mId, Time mPlayingOffset)
{
    if(assetManager.has<Music>(mId))
    {
        musicPlayer.play(assetManager.get<Music>(mId), mPlayingOffset);
    }
}

} // namespace hg
