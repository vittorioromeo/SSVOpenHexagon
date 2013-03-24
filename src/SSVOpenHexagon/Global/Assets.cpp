// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <fstream>
#include <map>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SFML/Graphics.hpp>
#include <jsoncpp/json.h>
#include <jsoncpp/reader.h>
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Online/Definitions.h"
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;
using namespace ssvu::FileSystem;

namespace hg
{
	AssetManager assetManager;
	map<string, Music*> musicPtrsMap;
	map<string, MusicData> musicDataMap;
	map<string, StyleData> styleDataMap;
	map<string, LevelData> levelDataMap;
	map<string, ProfileData> profileDataMap;
	map<string, EventData> eventDataMap;
	map<string, PackData> packDataMap;
	ProfileData* currentProfilePtr{nullptr};
	map<string, vector<string>> levelIdsByPackMap;
	vector<string> packPaths;

	void initAssetManager() { assetManager.loadFolder("Assets/"); }
	AssetManager& getAssetManager() { return assetManager; }

	void loadAssets()
	{
		log("loading profiles", "ASSETS"); 	loadProfiles();

		for(string packPath : getFolders("Packs/"))
		{
			string packName{packPath.substr(6, packPath.length() - 6)};

			string packLua{""};
			for(auto& path : getRecursiveFilesByExtension(packPath, ".lua")) packLua.append(getFileContents(path));
			string packHash{Online::getMD5Hash(packLua + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};

			Json::Value packRoot{getRootFromFile(packPath + "/pack.json")};
			PackData packData(packName, packRoot["name"].asString(), packRoot["priority"].asFloat(), packHash);
			packDataMap.insert(make_pair(packName, packData));
		}

		vector<PackData> packDatasToQuery;
		for(pair<string, PackData> packDataPair : packDataMap) packDatasToQuery.push_back(packDataPair.second);
		sort(begin(packDatasToQuery), end(packDatasToQuery), [](PackData a, PackData b) -> bool { return a.getPriority() < b.getPriority(); });

		for(PackData packData : packDatasToQuery)
		{
			string packName{packData.getId()};
			packPaths.push_back("Packs/" + packName + "/");
			log("loading " + packName + " music", "ASSETS"); 		loadMusic("Packs/" + packName + "/");
			log("loading " + packName + " music data", "ASSETS"); 	loadMusicData("Packs/" + packName + "/");
			log("loading " + packName + " style data", "ASSETS"); 	loadStyleData("Packs/" + packName + "/");
			log("loading " + packName + " level data", "ASSETS");	loadLevelData("Packs/" + packName + "/");
			log("loading " + packName + " events", "ASSETS"); 		loadEvents("Packs/" + packName + "/");
			log("loading " + packName + " custom sounds", "ASSETS");loadCustomSounds(packName, "Packs/" + packName + "/");
		}
	}

	void loadCustomSounds(const string& mPackName, const string& mPath)
	{
		for(auto filePath : getFilesByExtension(mPath + "Sounds/", ".ogg"))
		{
			string fileName{getNameFromPath(filePath, mPath + "Sounds/", "")};
			assetManager.loadSound(mPackName + "_" + fileName, filePath);
			assetManager.getSound(mPackName + "_" + fileName).setVolume(getSoundVolume());
		}
	}
	void loadMusic(const string& mPath)
	{
		for(auto filePath : getFilesByExtension(mPath + "Music/", ".ogg"))
		{
			string fileName{getNameFromPath(filePath, mPath + "Music/", ".ogg")};

			Music* music{new Music};
			music->openFromFile(filePath);
			music->setVolume(getMusicVolume());
			musicPtrsMap.insert(make_pair(fileName, music));
		}
	}
	void loadMusicData(const string& mPath)
	{
		for(auto filePath : getFilesByExtension(mPath + "Music/", ".json"))
		{
			MusicData musicData{loadMusicFromJson(getRootFromFile(filePath))};
			musicDataMap.insert(make_pair(musicData.getId(), musicData));
		}
	}
	void loadStyleData(const string& mPath)
	{
		for(auto filePath : getFilesByExtension(mPath + "Styles/", ".json"))
		{
			StyleData styleData{loadStyleFromJson(getRootFromFile(filePath))};
			styleData.setRootPath(filePath);
			styleDataMap.insert(make_pair(styleData.getId(), styleData));
		}
	}
	void loadLevelData(const string& mPath)
	{
		for(auto filePath : getFilesByExtension(mPath + "Levels/", ".json"))
		{
			Json::Value root{getRootFromFile(filePath)};
			string luaScriptPath{mPath + "Scripts/" + root["lua_file"].asString()};
			
			LevelData levelData{loadLevelFromJson(root)};
			levelData.setPackPath(mPath);
			levelData.setLevelRootPath(filePath);
			levelData.setStyleRootPath(getStyleData(levelData.getStyleId()).getRootPath());
			levelData.setLuaScriptPath(luaScriptPath);
			levelDataMap.insert(make_pair(levelData.getId(), levelData));
			levelIdsByPackMap[levelData.getPackPath()].push_back(levelData.getId());
		}
	}
	void loadProfiles()
	{
		for(auto filePath : getFilesByExtension("Profiles/", ".json"))
		{
			string fileName{getNameFromPath(filePath, "Profiles/", ".json")};

			ProfileData profileData{loadProfileFromJson(getRootFromFile(filePath))};
			profileDataMap.insert(make_pair(profileData.getName(), profileData));
		}
	}
	void loadEvents(const string& mPath)
	{
		for(auto filePath : getFilesByExtension(mPath + "Events/", ".json"))
		{
			EventData eventData{getRootFromFile(filePath)};
			eventDataMap.insert(make_pair(eventData.getId(), eventData));
		}
	}

	void saveCurrentProfile()
	{
		if(currentProfilePtr == nullptr) return;

		Json::StyledStreamWriter writer;
		ofstream o{getCurrentProfileFilePath(), std::ifstream::binary};

		Json::Value profileRoot;
		profileRoot["version"] = getVersion();
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
	vector<string> getAllLevelIds()
	{
		vector<LevelData> levelDataVector{getAllLevelData()};
		sort(begin(levelDataVector), end(levelDataVector),
		[](LevelData a, LevelData b) -> bool
		{
			if(a.getPackPath() == b.getPackPath()) return a.getMenuPriority() < b.getMenuPriority();
			return a.getPackPath() < b.getPackPath();
		});

		vector<string> result;
		for(auto levelData : levelDataVector) if(levelData.getSelectable()) result.push_back(levelData.getId());
		return result;
	}
	vector<string> getLevelIdsByPack(string mPackPath)
	{
		vector<LevelData> levelDataVector;
		for(string id : levelIdsByPackMap[mPackPath]) levelDataVector.push_back(getLevelData(id));

		sort(begin(levelDataVector), end(levelDataVector),
		[](LevelData a, LevelData b) -> bool
		{
			return a.getMenuPriority() < b.getMenuPriority();
		});

		vector<string> result;
		for(auto levelData : levelDataVector) if(levelData.getSelectable()) result.push_back(levelData.getId());
		return result;
	}
	vector<string> getPackPaths() { return packPaths; }
	vector<string> getPackNames()
	{
		vector<string> result;
		for(auto& packPair : packDataMap) result.push_back(packPair.first);
		return result;
	}

	void refreshVolumes()
	{
		for(auto& pair : assetManager.getSounds()) pair.second->setVolume(getSoundVolume());
		for(auto& pair : musicPtrsMap) pair.second->setVolume(getMusicVolume());
	}
	void stopAllMusic() { for(auto pair : musicPtrsMap) pair.second->stop(); }
	void stopAllSounds() { assetManager.stopSounds(); }
	void playSound(const string& mId) { if(!getNoSound()) getSoundPtr(mId)->play(); }

	Font& getFont(const string& mId) 				{ return assetManager.getFont(mId); }
	Sound* getSoundPtr(const string& mId) 			{ return &assetManager.getSound(mId); }
	Music* getMusicPtr(const string& mId) 			{ return musicPtrsMap.find(mId)->second; }
	MusicData getMusicData(const string& mId) 		{ return musicDataMap.find(mId)->second; }
	StyleData getStyleData(const string& mId) 		{ return styleDataMap.find(mId)->second; }
	LevelData getLevelData(const string& mId) 		{ return levelDataMap.find(mId)->second; }
	PackData getPackData(const string& mId) 		{ return packDataMap.find(mId)->second; }

	float getScore(const string& mId) 				{ return getCurrentProfile().getScore(mId); }
	void setScore(const string& mId, float mScore) { getCurrentProfile().setScore(mId, mScore); }

	void setCurrentProfile(const string& mName) { currentProfilePtr = &profileDataMap.find(mName)->second; }
	ProfileData& getCurrentProfile() { return *currentProfilePtr; }
	string getCurrentProfileFilePath() { return "Profiles/" + currentProfilePtr->getName() + ".json"; }
	void createProfile(const string& mName)
	{
		ofstream o{"Profiles/" + mName + ".json"};
		Json::Value root;
		Json::StyledStreamWriter writer;

		root["name"] = mName;
		root["scores"] = Json::objectValue;
		writer.write(o, root);
		o.flush(); o.close();

		profileDataMap.clear();
		loadProfiles();
	}
	int getProfilesSize() { return profileDataMap.size(); }
	vector<string> getProfileNames()
	{
		vector<string> result;
		for(auto pair : profileDataMap) result.push_back(pair.second.getName());
		return result;
	}
	string getFirstProfileName() { return profileDataMap.begin()->second.getName(); }

	EventData* getEventData(const string& mId, HexagonGame* mHgPtr)
	{
		EventData* result = new EventData(eventDataMap.find(mId)->second);
		result->setHexagonGamePtr(mHgPtr);
		return result;
	}
}
