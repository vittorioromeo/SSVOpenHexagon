// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Data/LoadInfo.hpp"
#include "SSVOpenHexagon/Data/PackInfo.hpp"

#include <SSVStart/Assets/Assets.hpp>
#include <SSVStart/Assets/AssetManager.hpp>

#include <SSVUtils/Core/FileSystem/Path.hpp>

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <cstddef>

namespace sf {

class SoundBuffer;
class Music;

} // namespace sf

namespace hg {

namespace Steam {
class steam_manager;
}

class MusicData;

class HGAssets
{
private:
    Steam::steam_manager* steamManager;

    bool levelsOnly{false};

    ssvs::AssetManager<> assetManager;

    std::unordered_map<std::string, LevelData> levelDatas;
    std::unordered_map<std::string, std::vector<std::string>>
        levelDataIdsByPack;

    std::unordered_map<std::string, PackData> packDatas;

    std::vector<PackInfo> packInfos;
    std::vector<PackInfo> selectablePackInfos;

    std::map<std::string, MusicData> musicDataMap;
    std::map<std::string, StyleData> styleDataMap;
    std::map<std::string, ProfileData> profileDataMap;
    ProfileData* currentProfilePtr{nullptr};

    [[nodiscard]] bool loadPackData(const ssvufs::Path& packPath);
    [[nodiscard]] bool loadPackInfo(const PackData& packData);

private:
    LoadInfo loadInfo;

public:
    HGAssets(Steam::steam_manager* mSteamManager, bool mHeadless,
        bool mLevelsOnly = false);

    ~HGAssets();

    [[nodiscard]] LoadInfo& getLoadResults();

    [[nodiscard]] auto& operator()();

    template <typename T>
    [[nodiscard]] T& get(const std::string& mId);

    [[nodiscard]] const std::unordered_map<std::string, LevelData>&
    getLevelDatas();

    [[nodiscard]] bool isValidLevelId(
        const std::string& mLevelId) const noexcept;

    [[nodiscard]] const LevelData& getLevelData(
        const std::string& mAssetId) const;

    [[nodiscard]] bool packHasLevels(const std::string& mPackId);

    [[nodiscard]] const std::vector<std::string>& getLevelIdsByPack(
        const std::string& mPackId);

    [[nodiscard]] const std::unordered_map<std::string, PackData>&
    getPacksData();

    [[nodiscard]] bool isValidPackId(const std::string& mPackId) const noexcept;

    [[nodiscard]] const PackData& getPackData(const std::string& mPackId);

    [[nodiscard]] const std::vector<PackInfo>& getPackInfos() const noexcept;

    [[nodiscard]] const std::vector<PackInfo>&
    getSelectablePackInfos() const noexcept;

    [[nodiscard]] const std::unordered_map<std::string, PackData>&
    getPackDatas() const noexcept;

    [[nodiscard]] const PackData* findPackData(
        const std::string& mPackDisambiguator, const std::string& mPackName,
        const std::string& mPackAuthor) const noexcept;

    [[nodiscard]] bool loadAssets();

    void loadMusic(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadMusicData(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadStyleData(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadLevelData(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadCustomSounds(
        const std::string& mPackId, const ssvufs::Path& mPath);

    [[nodiscard]] const MusicData& getMusicData(
        const std::string& mPackId, const std::string& mId);
    [[nodiscard]] const StyleData& getStyleData(
        const std::string& mPackId, const std::string& mId);

    [[nodiscard]] std::string reloadPack(
        const std::string& mPackId, const std::string& mPath);
    [[nodiscard]] std::string reloadLevel(const std::string& mPackId,
        const std::string& mPath, const std::string& mId);

    float getLocalScore(const std::string& mId);
    void setLocalScore(const std::string& mId, float mScore);

    void loadLocalProfiles();
    void saveCurrentLocalProfile();
    void saveAllProfiles();
    void setCurrentLocalProfile(const std::string& mName);

    [[nodiscard]] bool anyLocalProfileActive() const;
    [[nodiscard]] ProfileData& getCurrentLocalProfile();
    [[nodiscard]] const ProfileData& getCurrentLocalProfile() const;
    [[nodiscard]] std::string getCurrentLocalProfileFilePath();
    [[nodiscard]] ProfileData* getLocalProfileByName(const std::string& mName);
    [[nodiscard]] const ProfileData* getLocalProfileByName(
        const std::string& mName) const;
    void createLocalProfile(const std::string& mName);
    [[nodiscard]] std::size_t getLocalProfilesSize();
    [[nodiscard]] std::vector<std::string> getLocalProfileNames();

    [[nodiscard]] bool pIsValidLocalProfile() const;
    [[nodiscard]] std::string pGetName() const;
    [[nodiscard]] const std::vector<std::string>& pGetTrackedNames() const;

    void pClearTrackedNames();
    void pAddTrackedName(const std::string& mName);
    void pSaveCurrent();
    void pSaveAll();
    void pSetCurrent(const std::string& mName);
    void pCreate(const std::string& mName);

    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(const std::string& assetId);
    [[nodiscard]] sf::Music* getMusic(const std::string& assetId);
};

} // namespace hg
