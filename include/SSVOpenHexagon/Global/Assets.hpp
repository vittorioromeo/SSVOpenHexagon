// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/StringHash.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <unordered_map>
#include <unordered_set>

namespace sf
{
class Font;
class Shader;
class SoundBuffer;
class Texture;
} // namespace sf

namespace ssvu::FileSystem
{
class Path;
}

namespace hg
{

namespace Steam
{
class steam_manager;
}

class ProfileData;
struct LoadInfo;
class MusicData;
class AssetStorage;
struct LevelData;
struct PackData;
struct PackInfo;
class StyleData;

class HGAssets
{
private:
    class HGAssetsImpl;
    sf::base::UniquePtr<HGAssetsImpl> _impl;

public:
    HGAssets(Steam::steam_manager* mSteamManager, bool mHeadless, bool mLevelsOnly = false);

    ~HGAssets();

    [[nodiscard]] bool isHeadless() const;

    [[nodiscard]] LoadInfo& getLoadResults();

    [[nodiscard]] bool         hasTexture(const sf::base::String& mId);
    [[nodiscard]] sf::Texture& getTexture(const sf::base::String& mId);

    [[nodiscard]] sf::Font& getFont(const sf::base::String& mId);

    [[nodiscard]] bool isValidLevelId(const sf::base::String& mLevelId) const noexcept;

    [[nodiscard]] const LevelData& getLevelData(const sf::base::String& mAssetId) const;

    [[nodiscard]] bool packHasLevels(const sf::base::String& mPackId);

    [[nodiscard]] const sf::base::Vector<sf::base::String>& getLevelIdsByPack(const sf::base::String& mPackId);

    [[nodiscard]] const std::unordered_map<sf::base::String, PackData>& getPackDatas();

    [[nodiscard]] bool isValidPackId(const sf::base::String& mPackId) const noexcept;

    [[nodiscard]] const PackData& getPackData(const sf::base::String& mPackId);

    [[nodiscard]] const sf::base::Vector<PackInfo>& getSelectablePackInfos() const noexcept;

    // Counter bumped whenever the level/pack list changes (initial load,
    // hot-install, hot-uninstall, dev-time reload). Cheap to read; the new
    // UI uses it as a "should I rebuild my filtered/sorted list?" signal.
    [[nodiscard]] sf::base::U64 packListVersion() const noexcept;

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
    [[nodiscard]] sf::base::String reloadPack(const sf::base::String& mPackId, const sf::base::String& mPath);
    [[nodiscard]] sf::base::String reloadLevel(const sf::base::String& mPackId,
                                               const sf::base::String& mPath,
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

    [[nodiscard]] const std::unordered_map<sf::base::String, LevelData>& getLevelDatas() const noexcept;

    [[nodiscard]] const std::unordered_set<sf::base::String>& getPackIdsWithMissingDependencies() const noexcept;

    void addLocalProfile(ProfileData&& profileData);

    [[nodiscard]] std::unordered_map<sf::base::String, sf::base::String>& getLuaFileCache();

    [[nodiscard]] const std::unordered_map<sf::base::String, sf::base::String>& getLuaFileCache() const;
};

} // namespace hg
