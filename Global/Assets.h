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
#include "Data/ProfileData.h"
#include "Data/StyleData.h"
#include "Utils.h"

using namespace std;
using namespace sf;

namespace hg
{
	void loadAssets();

	void loadFonts();
	void loadSounds();
	void loadMusic();
	void loadMusicData();
	void loadStyleData();
	void loadLevelData();
	void loadProfiles();

	void saveCurrentProfile();

	void stopAllMusic();
	void stopAllSounds();
	void playSound(string mId);

	Font& getFont(string mId);
	Sound* getSoundPtr(string mId);
	Music* getMusicPtr(string mId);
	MusicData getMusicData(string mId);
	StyleData getStyleData(string mId);
	LevelData getLevelData(string mId);

	vector<LevelData> getAllLevelData();
	vector<string> getAllMenuLevelDataIds();

	float getScore(string mId);
	void setScore(string mId, float mScore);

	void setCurrentProfile(ProfileData& mProfilePair);
	ProfileData& getCurrentProfile();
	string getCurrentProfileFilePath();
}

#endif /* ASSETS_H_ */
