/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
#include "Data/EventData.h"
#include "Data/ProfileData.h"
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
	map<string, EventData> eventDataMap;
	ProfileData* currentProfilePtr{nullptr};

	void loadAssets()
	{
		log("loading profiles"); 	loadProfiles	();
		log("loading fonts"); 		loadFonts		();
		log("loading sounds"); 		loadSounds		();

		for(string packName : getAllSubFolderNames("Packs/"))
		{
			log("loading " + packName + " music"); 		loadMusic		("Packs/" + packName + "/");
			log("loading " + packName + " music data"); loadMusicData	("Packs/" + packName + "/");
			log("loading " + packName + " style data"); loadStyleData	("Packs/" + packName + "/");
			log("loading " + packName + " level data");	loadLevelData	("Packs/" + packName + "/");
			log("loading " + packName + " events"); 	loadEvents		("Packs/" + packName + "/");
		}
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
	void loadMusic(string mPath)
	{
		for(auto filePath : getAllFilePaths(mPath + "Music/", ".ogg"))
		{
			string fileName{getFileNameFromFilePath(filePath, mPath + "Music/", ".ogg")};

			Music* music{new Music};
			music->openFromFile(filePath);
			music->setVolume(getMusicVolume());
			musicPtrsMap.insert(make_pair(fileName, music));
		}
	}
	void loadMusicData(string mPath)
	{
		for(auto filePath : getAllFilePaths(mPath + "Music/", ".json"))
		{
			MusicData musicData{loadMusicFromJson(getJsonFileRoot(filePath))};
			musicDataMap.insert(make_pair(musicData.getId(), musicData));
		}
	}
	void loadStyleData(string mPath)
	{
		for(auto filePath : getAllFilePaths(mPath + "Styles/", ".json"))
		{
			StyleData styleData{loadStyleFromJson(getJsonFileRoot(filePath))};
			styleDataMap.insert(make_pair(styleData.getId(), styleData));
		}
	}
	void loadLevelData(string mPath)
	{
		for(auto filePath : getAllFilePaths(mPath + "Levels/", ".json"))
		{
			LevelData levelData{loadLevelFromJson(getJsonFileRoot(filePath))};
			levelData.setPackPath(mPath);
			levelDataMap.insert(make_pair(levelData.getId(), levelData));
		}
	}
	void loadProfiles()
	{
		for(auto filePath : getAllFilePaths("Profiles/", ".json"))
		{
			string fileName{getFileNameFromFilePath(filePath, "Profiles/", ".json")};

			ProfileData profileData{loadProfileFromJson(getJsonFileRoot(filePath))};
			profileDataMap.insert(make_pair(profileData.getName(), profileData));
		}
	}
	void loadEvents(string mPath)
	{
		for(auto filePath : getAllFilePaths(mPath + "Events/", ".json"))
		{
			EventData eventData{getJsonFileRoot(filePath)};
			eventDataMap.insert(make_pair(eventData.getId(), eventData));
		}
	}

	void saveCurrentProfile()
	{
		if(currentProfilePtr == nullptr) return;

		Json::StyledStreamWriter writer;
		ofstream o{getCurrentProfileFilePath(), std::ifstream::binary};

		Json::Value profileRoot;
		profileRoot["name"] = getCurrentProfile().getName();
		profileRoot["scores"] = getCurrentProfile().getScores();

		writer.write(o, profileRoot);
		o.flush();
		o.close();
	}

	vector<LevelData> getAllLevelData()
	{
		vector<LevelData> result;
		for(auto pair : levelDataMap) result.push_back(pair.second);
		return result;
	}
	vector<string> getAllMenuLevelDataIds()
	{
		vector<LevelData> levelDataVector{getAllLevelData()};
		sort(begin(levelDataVector), end(levelDataVector),
		[](LevelData a, LevelData b) -> bool { return a.getMenuPriority() < b.getMenuPriority(); });

		vector<string> result;
		for(auto levelData : levelDataVector) if(levelData.getSelectable()) result.push_back(levelData.getId());
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

	void setCurrentProfile(string mName) { currentProfilePtr = &profileDataMap.find(mName)->second; }
	ProfileData& getCurrentProfile() { return *currentProfilePtr; }
	string getCurrentProfileFilePath() { return "Profiles/" + currentProfilePtr->getName() + ".json"; }
	void createProfile(string mName)
	{
		ofstream o{"Profiles/" + mName + ".json"};
		Json::Value root;
		Json::StyledStreamWriter writer;

		root["name"] = mName;
		root["scores"] = Json::objectValue;
		writer.write(o, root);
		o.flush();
		o.close();

		profileDataMap.clear();
		loadProfiles();
	}
	int getProfilesSize() { return profileDataMap.size(); }
	vector<string> getProfileNames()
	{
		vector<string> result;

		for(auto pair : profileDataMap)
			result.push_back(pair.second.getName());

		return result;
	}
	string getFirstProfileName() { return profileDataMap.begin()->second.getName(); }

	EventData getEventData(string mId, HexagonGame* mHgPtr)
	{
		EventData result{eventDataMap.find(mId)->second};
		result.setHexagonGamePtr(mHgPtr);
		return result;
	}
}
