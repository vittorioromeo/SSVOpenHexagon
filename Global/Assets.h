// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef ASSETS_H_
#define ASSETS_H_

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <SFML/Audio.hpp>
#include <json/json.h>
#include <json/reader.h>
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/PackData.h"
#include "Data/ProfileData.h"
#include "Data/StyleData.h"
#include "Utils.h"
#include "HexagonGame.h"

namespace hg
{	
	void loadAssets();

	void loadFonts();
	void loadSounds();
	void loadMusic(std::string mPath);
	void loadMusicData(std::string mPath);
	void loadStyleData(std::string mPath);
	void loadLevelData(std::string mPath);
	void loadCustomSounds(std::string mPackName, std::string mPath);
	void loadProfiles();
	void loadEvents(std::string mPath);

	void saveCurrentProfile();

	void stopAllMusic();
	void stopAllSounds();
	void playSound(std::string mId);

	sf::Font& getFont(std::string mId);
	sf::Sound* getSoundPtr(std::string mId);
	sf::Music* getMusicPtr(std::string mId);
	MusicData getMusicData(std::string mId);
	StyleData getStyleData(std::string mId);
	LevelData getLevelData(std::string mId);
	PackData getPackData(std::string mId);

	std::vector<LevelData> getAllLevelData();
	std::vector<std::string> getAllMenuLevelDataIds();
	std::vector<std::string> getMenuLevelDataIdsByPack(std::string mPackPath);
	std::vector<std::string> getPackPaths();

	float getScore(std::string mId);
	void setScore(std::string mId, float mScore);

	void setCurrentProfile(std::string mName);
	ProfileData& getCurrentProfile();
	std::string getCurrentProfileFilePath();
	void createProfile(std::string mName);
	int getProfilesSize();
	std::vector<std::string> getProfileNames();
	std::string getFirstProfileName();

	EventData* getEventData(std::string mId, HexagonGame* mHgPtr);
}

#endif /* ASSETS_H_ */
