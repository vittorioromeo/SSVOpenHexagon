// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"

#include <SSVStart/Assets/Assets.hpp>

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <SFML/System.hpp>

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <cstddef>

namespace sf
{

class SoundBuffer;
class Music;

} // namespace sf

namespace hg
{

namespace Steam
{

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

    struct PackInfo
    {
        std::string id;
        ssvufs::Path path;
    };

    std::vector<PackInfo> packInfos;
    std::vector<PackInfo> selectablePackInfos;

    std::map<std::string, MusicData> musicDataMap;
    std::map<std::string, StyleData> styleDataMap;
    std::map<std::string, ProfileData> profileDataMap;
    ProfileData* currentProfilePtr{nullptr};

    [[nodiscard]] bool loadPackData(const ssvufs::Path& packPath);
    [[nodiscard]] bool loadPackInfo(const PackData& packData);

public:
    struct LoadInfo
    {
        unsigned int packs{0};
        unsigned int levels{0};
        unsigned int assets{0};
        std::vector<std::string> errorMessages;

        void addFormattedError(std::string& error)
        {
            if(error.empty())
            {
                return;
            }

            // Remove the first two characters
            error.erase(0, 2);

            // Replace first newline with '-', place a space before it,
            // and remove a space after it.
            std::size_t i = error.find('\n');
            error.insert(i, " ");
            error[++i] = '-';
            error.erase(++i, 1);

            // Remove all other newlines.
            while((i = error.find('\n', i)) != std::string::npos)
            {
                error.erase(i, 1);
            }

            errorMessages.emplace_back(error);
        }
    };

private:
    LoadInfo loadInfo;

    [[nodiscard]] LevelData& getEditLevelData(const std::string& mAssetId)
    {
        const auto it = levelDatas.find(mAssetId);
        if(it == levelDatas.end())
        {
            ssvu::lo("getLevelData")
                << "Asset '" << mAssetId << "' not found\n";

            SSVOH_ASSERT(!levelDatas.empty());
            return levelDatas.begin()->second;
        }

        return it->second;
    }

public:
    HGAssets(Steam::steam_manager* mSteamManager, bool mHeadless,
        bool mLevelsOnly = false);

    ~HGAssets();

    [[nodiscard]] LoadInfo& getLoadResults()
    {
        return loadInfo;
    }

    [[nodiscard]] auto& operator()()
    {
        return assetManager;
    }

    template <typename T>
    [[nodiscard]] T& get(const std::string& mId)
    {
        return assetManager.get<T>(mId);
    }

    [[nodiscard]] const std::unordered_map<std::string, LevelData>&
    getLevelDatas()
    {
        return levelDatas;
    }

    [[nodiscard]] bool isValidLevelId(
        const std::string& mLevelId) const noexcept
    {
        return levelDatas.find(mLevelId) != levelDatas.end();
    }

    [[nodiscard]] const LevelData& getLevelData(const std::string& mAssetId)
    {
        SSVOH_ASSERT(isValidLevelId(mAssetId));
        return getEditLevelData(mAssetId);
    }

    [[nodiscard]] const LevelData& getLevelData(
        const std::string& mPackId, const std::string& mId)
    {
        return getLevelData(mPackId + "_" + mId);
    }

    [[nodiscard]] bool packHasLevels(const std::string& mPackId)
    {
        return levelDataIdsByPack.count(mPackId) > 0;
    }

    [[nodiscard]] const std::vector<std::string>& getLevelIdsByPack(
        const std::string& mPackId)
    {
        SSVOH_ASSERT(levelDataIdsByPack.count(mPackId) > 0);
        return levelDataIdsByPack.at(mPackId);
    }

    [[nodiscard]] const std::unordered_map<std::string, PackData>&
    getPacksData()
    {
        return packDatas;
    }

    [[nodiscard]] bool isValidPackId(const std::string& mPackId) const noexcept
    {
        return packDatas.find(mPackId) != packDatas.end();
    }

    [[nodiscard]] const PackData& getPackData(const std::string& mPackId)
    {
        SSVOH_ASSERT(isValidPackId(mPackId));
        return packDatas.at(mPackId);
    }

    [[nodiscard]] const std::vector<PackInfo>& getPackInfos() const noexcept
    {
        return packInfos;
    }

    [[nodiscard]] const std::vector<PackInfo>&
    getSelectablePackInfos() const noexcept
    {
        return selectablePackInfos;
    }

    [[nodiscard]] const std::unordered_map<std::string, PackData>&
    getPackDatas() const noexcept
    {
        return packDatas;
    }

    [[nodiscard]] const PackData* findPackData(
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
    [[nodiscard]] std::string getFirstLocalProfileName();

    [[nodiscard]] bool pIsValidLocalProfile() const
    {
        return currentProfilePtr != nullptr;
    }

    [[nodiscard]] std::string pGetName() const
    {
        return getCurrentLocalProfile().getName();
    }

    [[nodiscard]] const std::vector<std::string>& pGetTrackedNames() const
    {
        return getCurrentLocalProfile().getTrackedNames();
    }

    void pClearTrackedNames()
    {
        getCurrentLocalProfile().clearTrackedNames();
    }

    void pAddTrackedName(const std::string& mName)
    {
        getCurrentLocalProfile().addTrackedName(mName);
    }

    void pSaveCurrent()
    {
        saveCurrentLocalProfile();
    }

    void pSaveAll()
    {
        saveAllProfiles();
    }

    void pSetCurrent(const std::string& mName)
    {
        setCurrentLocalProfile(mName);
    }

    void pCreate(const std::string& mName)
    {
        createLocalProfile(mName);
    }

    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(const std::string& assetId);

    [[nodiscard]] sf::Music* getMusic(const std::string& assetId);
};

} // namespace hg
