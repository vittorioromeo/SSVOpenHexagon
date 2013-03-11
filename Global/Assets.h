// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ASSETS
#define HG_ASSETS

#include <string>
#include <SSVStart.h>
#include <SFML/Audio.hpp>
#include "Data/EventData.h"
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/PackData.h"
#include "Data/ProfileData.h"
#include "Data/StyleData.h"

namespace hg
{
	void initAssetManager();
	ssvs::AssetManager& getAssetManager();

	void loadAssets();

	void loadMusic(const std::string& mPath);
	void loadMusicData(const std::string& mPath);
	void loadStyleData(const std::string& mPath);
	void loadLevelData(const std::string& mPath);
	void loadCustomSounds(const std::string& mPackName, const std::string& mPath);
	void loadProfiles();
	void loadEvents(const std::string& mPath);

	void saveCurrentProfile();

	void refreshVolumes();
	void stopAllMusic();
	void stopAllSounds();
	void playSound(const std::string& mId);

	sf::Font& getFont(const std::string& mId);
	sf::Sound* getSoundPtr(const std::string& mId);
	sf::Music* getMusicPtr(const std::string& mId);
	MusicData getMusicData(const std::string& mId);
	StyleData getStyleData(const std::string& mId);
	LevelData getLevelData(const std::string& mId);
	PackData getPackData(const std::string& mId);

	std::vector<LevelData> getAllLevelData();
	std::vector<std::string> getAllLevelIds();
	std::vector<std::string> getLevelIdsByPack(std::string mPackPath);
	std::vector<std::string> getPackPaths();
	std::vector<std::string> getPackNames();

	float getScore(const std::string& mId);
	void setScore(const std::string& mId, float mScore);

	void setCurrentProfile(const std::string& mName);
	ProfileData& getCurrentProfile();
	std::string getCurrentProfileFilePath();
	void createProfile(const std::string& mName);
	int getProfilesSize();
	std::vector<std::string> getProfileNames();
	std::string getFirstProfileName();

	EventData* getEventData(const std::string& mId, HexagonGame* mHgPtr);
}

#endif
