#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <json/json.h>
#include <json/reader.h>
#include "Data/MusicData.h"
#include "Data/LevelData.h"
#include "Data/StyleData.h"
#include "Global/Assets.h"
#include "Utils/Utils.h"

using namespace std;
using namespace sf;

namespace hg
{
	map<string, Font> fontsMap;
	map<string, SoundBuffer*> soundBufferPtrsMap;
	map<string, Sound*> soundPtrsMap;
	map<string, Music*> musicPtrsMap;
	map<string, MusicData> musicDataMap;
	map<string, StyleData> styleDataMap;
	map<string, LevelData> levelDataMap;
	map<string, ProfileData> profileDataMap;
	ProfileData* currentProfilePtr;

	void loadAssets()
	{
		log("loading fonts"); 		loadFonts();
		log("loading sounds"); 		loadSounds();
		log("loading music"); 		loadMusic();
		log("loading music data"); 	loadMusicData();
		log("loading style data"); 	loadStyleData();
		log("loading level data");	loadLevelData();
		log("loading profiles"); 	loadProfiles();
	}

	void loadFonts()
	{
		for(auto filePath : getAllFilePaths("Fonts/", ".ttf"))
		{
			string fileName{getFileNameFromFilePath(filePath, "Fonts/", ".ttf")};

			Font font;
			font.loadFromFile(filePath);			
			fontsMap.insert(make_pair(fileName, font));
		}
	}
	void loadSounds()
	{
		Json::Value soundsRoot = getJsonFileRoot("Sounds/sounds.json");

		for(Json::ValueIterator itr{soundsRoot.begin()}; itr != soundsRoot.end(); itr++)
		{
			SoundBuffer* soundBuffer{new SoundBuffer};
			soundBuffer->loadFromFile("Sounds/" + (*itr).asString());
			soundBufferPtrsMap.insert(make_pair(itr.key().asString(), soundBuffer));

			Sound* soundPtr{new Sound};
			soundPtr->setBuffer(*soundBuffer);
			soundPtr->setVolume(getSoundVolume());
			soundPtrsMap.insert(make_pair(itr.key().asString(), soundPtr));

		}
	}	
	void loadMusic()
	{
		for(auto filePath : getAllFilePaths("Music/", ".ogg"))
		{
			string fileName{getFileNameFromFilePath(filePath, "Music/", ".ogg")};

			Music* music{new Music};
			music->openFromFile(filePath);
			music->setVolume(getMusicVolume());
			musicPtrsMap.insert(make_pair(fileName, music));
		}
	}
	void loadMusicData()
	{
		for(auto filePath : getAllFilePaths("Music/", ".json"))
		{
			MusicData musicData{loadMusicFromJson(getJsonFileRoot(filePath))};
			musicDataMap.insert(make_pair(musicData.getId(), musicData));
		}
	}
	void loadStyleData()
	{
		for(auto filePath : getAllFilePaths("Styles/", ".json"))
		{
			StyleData styleData{loadStyleFromJson(getJsonFileRoot(filePath))};
			styleDataMap.insert(make_pair(styleData.getId(), styleData));
		}
	}
	void loadLevelData()
	{
		for(auto filePath : getAllFilePaths("Levels/", ".json"))
		{
			LevelData levelData{loadLevelFromJson(getJsonFileRoot(filePath))};
			levelDataMap.insert(make_pair(levelData.getId(), levelData)); // replace with getId
		}
	}
	void loadProfiles()
	{
		if (getAllFilePaths("Profiles/", ".json").empty())
		{
			log("no profiles found, creating one");

			ofstream o{"Profiles/default.json"};
			Json::Value defaultProfileRoot;
			Json::StyledStreamWriter writer;

			defaultProfileRoot["name"] = "default";
			defaultProfileRoot["scores"] = Json::objectValue;
			writer.write(o, defaultProfileRoot);
		}

		for(auto filePath : getAllFilePaths("Profiles/", ".json"))
		{
			string fileName{getFileNameFromFilePath(filePath, "Profiles/", ".json")};

			ProfileData profileData{loadProfileFromJson(fileName, getJsonFileRoot(filePath))};
			profileDataMap.insert(make_pair(profileData.getId(), profileData));
		}
		
		setCurrentProfile(profileDataMap.begin()->second);
	}

	void saveCurrentProfile()
	{
		Json::StyledStreamWriter writer;
		ofstream o{getCurrentProfileFilePath(), std::ifstream::binary};

		Json::Value profileRoot;
		profileRoot["name"] = getCurrentProfile().getName();
		profileRoot["scores"] = getCurrentProfile().getScores();

		writer.write(o, profileRoot);
	}

	vector<LevelData> getAllLevelData()
	{
		vector<LevelData> result;
		for(auto pair : levelDataMap) result.push_back(pair.second);
		return result;
	}
	vector<string> getAllLevelDataIds()
	{
		vector<LevelData> levelDataVector{getAllLevelData()};
		sort(begin(levelDataVector), end(levelDataVector),
		[](LevelData a, LevelData b) -> bool { return a.getMenuPriority() < b.getMenuPriority(); });

		vector<string> result;
		for(auto levelData : levelDataVector) result.push_back(levelData.getId());
		return result;
	}

	void stopAllMusic() { for(auto pair : musicPtrsMap) pair.second->stop(); }
	void stopAllSounds() { for(auto pair : soundPtrsMap) pair.second->stop(); }
	void playSound(string mId) { if(!getNoSound()) getSoundPtr(mId)->play(); }

	Font& getFont(string mId) 				{ return fontsMap.find(mId)->second; }
	Sound* getSoundPtr(string mId) 			{ return soundPtrsMap.find(mId)->second; }
	Music* getMusicPtr(string mId) 			{ return musicPtrsMap.find(mId)->second; }
	MusicData getMusicData(string mId) 		{ return musicDataMap.find(mId)->second; }
	StyleData getStyleData(string mId) 		{ return styleDataMap.find(mId)->second; }
	LevelData getLevelData(string mId) 		{ return levelDataMap.find(mId)->second; }

	float getScore(string mId) 				{ return getCurrentProfile().getScore(mId); }
	void setScore(string mId, float mScore) { getCurrentProfile().setScore(mId, mScore); }

	void setCurrentProfile(ProfileData& mProfilePair) { currentProfilePtr = &mProfilePair; }
	ProfileData& getCurrentProfile() { return *currentProfilePtr; }
	string getCurrentProfileFilePath() { return "Profiles/" + currentProfilePtr->getId() + ".json"; }
}
