// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Online/OHServer.hpp"

namespace hg
{

class MusicData;

class HGAssets
{
private:
    bool playingLocally{true};
    bool levelsOnly{false};

    ssvs::AssetManager<> assetManager;
    ssvs::SoundPlayer soundPlayer;

public:
    ssvs::MusicPlayer musicPlayer;

private:
    std::unordered_map<std::string, LevelData> levelDatas;
    std::unordered_map<std::string, std::vector<std::string>>
        levelDataIdsByPack;

    std::unordered_map<std::string, PackData> packDatas;

    struct PackInfo
    {
        std::string id;
        Path path;
    };

    std::vector<PackInfo> packInfos;

    std::map<std::string, MusicData> musicDataMap;
    std::map<std::string, StyleData> styleDataMap;
    std::map<std::string, ProfileData> profileDataMap;
    ProfileData* currentProfilePtr{nullptr};

public:
    float playedSeconds{0};

    HGAssets(bool mLevelsOnly = false);

    auto& operator()()
    {
        return assetManager;
    }

    template <typename T>
    T& get(const std::string& mId)
    {
        return assetManager.get<T>(mId);
    }

    const std::unordered_map<std::string, LevelData>& getLevelDatas()
    {
        return levelDatas;
    }

    const LevelData& getLevelData(const std::string& mId)
    {
        return levelDatas.at(mId);
    }

    const std::vector<std::string>& getLevelIdsByPack(const Path& mPackPath)
    {
        return levelDataIdsByPack.at(mPackPath);
    }

    const PackData& getPackData(const std::string& mId)
    {
        return packDatas.at(mId);
    }

    const std::vector<PackInfo>& getPackInfos()
    {
        return packInfos;
    }

    [[nodiscard]] bool loadAssets();

    void loadMusic(const Path& mPath);
    void loadMusicData(const Path& mPath);
    void loadStyleData(const Path& mPath);
    void loadLevelData(const Path& mPath);
    void loadCustomSounds(const std::string& mPackName, const Path& mPath);
    void loadLocalProfiles();

    void saveCurrentLocalProfile();

    const MusicData& getMusicData(const std::string& mId);
    const StyleData& getStyleData(const std::string& mId);

    float getLocalScore(const std::string& mId);
    void setLocalScore(const std::string& mId, float mScore);
    void setCurrentLocalProfile(const std::string& mName);
    ProfileData& getCurrentLocalProfile();
    const ProfileData& getCurrentLocalProfile() const;
    std::string getCurrentLocalProfileFilePath();
    void createLocalProfile(const std::string& mName);
    std::size_t getLocalProfilesSize();
    std::vector<std::string> getLocalProfileNames();
    std::string getFirstLocalProfileName();

    bool pIsValidLocalProfile()
    {
        return currentProfilePtr != nullptr;
    }

    std::string pGetName() const
    {
        if(!playingLocally)
        {
            return Online::getCurrentUsername();
        }

        return getCurrentLocalProfile().getName();
    }

    const std::vector<std::string>& pGetTrackedNames() const
    {
        if(!playingLocally)
        {
            return Online::getUserStats().trackedNames;
        }

        return getCurrentLocalProfile().getTrackedNames();
    }

    void pClearTrackedNames()
    {
        if(!playingLocally)
        {
            Online::trySendClearFriends();
            return;
        }

        getCurrentLocalProfile().clearTrackedNames();
    }

    void pAddTrackedName(const std::string& mName)
    {
        if(!playingLocally)
        {
            Online::trySendAddFriend(mName);
            return;
        }

        getCurrentLocalProfile().addTrackedName(mName);
    }

    void pSaveCurrent()
    {
        if(!playingLocally)
        {
            return;
        }

        saveCurrentLocalProfile();
    }

    void pSetCurrent(const std::string& mName)
    {
        if(!playingLocally)
        {
            throw;
        }

        setCurrentLocalProfile(mName);
    }

    void pCreate(const std::string& mName)
    {
        if(!playingLocally)
        {
            throw;
        }

        createLocalProfile(mName);
    }

    bool pIsLocal() const
    {
        return playingLocally;
    }

    void pSetPlayingLocally(bool mPlayingLocally)
    {
        playingLocally = mPlayingLocally;
    }

    void refreshVolumes();
    void stopMusics();
    void stopSounds();

    void playSound(const std::string& mId,
        ssvs::SoundPlayer::Mode mMode = ssvs::SoundPlayer::Mode::Override);

    void playMusic(
        const std::string& mId, sf::Time mPlayingOffset = sf::seconds(0));

    ssvs::SoundPlayer& getSoundPlayer() noexcept
    {
        return soundPlayer;
    }

    ssvs::MusicPlayer& getMusicPlayer() noexcept
    {
        return musicPlayer;
    }
};

} // namespace hg
