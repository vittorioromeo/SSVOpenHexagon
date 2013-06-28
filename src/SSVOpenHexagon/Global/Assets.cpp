// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <fstream>
#include <map>
#include <SFML/Graphics.hpp>
#include <SSVJsonCpp/SSVJsonCpp.h>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
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
	map<string, MusicData> musicDataMap;
	map<string, StyleData> styleDataMap;
	map<string, LevelData> levelDataMap;
	map<string, ProfileData> profileDataMap;
	map<string, EventData> eventDataMap;
	map<string, PackData> packDataMap;
	ProfileData* currentProfilePtr{nullptr};
	map<string, vector<string>> levelIdsByPackMap;
	vector<string> packPaths;
	SoundPlayer soundPlayer;

	void initAssetManager() { loadAssetsFromJson(assetManager, "Assets/", getRootFromFile("Assets/assets.json")); }
	AssetManager& getAssetManager() { return assetManager; }

	void loadAssets()
	{
		log("loading profiles", "LoadAssets"); 	loadProfiles();

		for(string packPath : getScan<Mode::Single, Type::Folder>("Packs/"))
		{
			string packName{packPath.substr(6, packPath.length() - 6)}, packLua{""};
			for(const auto& p : getScan<Mode::Recurse, Type::File, Pick::ByExt>(packPath, ".lua")) packLua.append(getFileContents(p));
			string packHash{Online::getMD5Hash(packLua + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};

			Json::Value packRoot{getRootFromFile(packPath + "/pack.json")};
			PackData packData(packName, as<string>(packRoot, "name"), as<float>(packRoot, "priority"), packHash);
			packDataMap.insert(make_pair(packName, packData));
		}

		vector<PackData> packDatasToQuery;
		for(pair<string, PackData> packDataPair : packDataMap) packDatasToQuery.push_back(packDataPair.second);
		sort(begin(packDatasToQuery), end(packDatasToQuery), [](PackData a, PackData b) { return a.getPriority() < b.getPriority(); });

		for(PackData packData : packDatasToQuery)
		{
			string packName{packData.getId()}, packPath{"Packs/" + packName + "/"};
			packPaths.push_back("Packs/" + packName + "/");
			log("loading " + packName + " music", "LoadAssets");			loadMusic(packPath);
			log("loading " + packName + " music data", "LoadAssets");		loadMusicData(packPath);
			log("loading " + packName + " style data", "LoadAssets");		loadStyleData(packPath);
			log("loading " + packName + " level data", "LoadAssets");		loadLevelData(packPath);
			log("loading " + packName + " events", "LoadAssets");			loadEvents(packPath);
			log("loading " + packName + " custom sounds", "LoadAssets");	loadCustomSounds(packName, packPath);
		}
	}

	void loadCustomSounds(const string& mPackName, const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Sounds/", ".ogg"))
		{
			string fileName{getNameFromPath(p, mPath + "Sounds/", "")};
			assetManager.loadSoundBuffer(mPackName + "_" + fileName, p);
		}
	}
	void loadMusic(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Music/", ".ogg"))
		{
			string fileName{getNameFromPath(p, mPath + "Music/", ".ogg")};

			auto& music(assetManager.loadMusic(fileName, p));
			music.openFromFile(p);
			music.setVolume(getMusicVolume());
			music.setLoop(true);
		}
	}
	void loadMusicData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Music/", ".json"))
		{
			MusicData musicData{loadMusicFromJson(getRootFromFile(p))};
			musicDataMap.insert(make_pair(musicData.getId(), musicData));
		}
	}
	void loadStyleData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Styles/", ".json"))
		{
			StyleData styleData{loadStyleFromJson(getRootFromFile(p))};
			styleData.setRootPath(p);
			styleDataMap.insert(make_pair(styleData.getId(), styleData));
		}
	}
	void loadLevelData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Levels/", ".json"))
		{
			Json::Value root{getRootFromFile(p)};
			string luaScriptPath{mPath + "Scripts/" + as<string>(root, "lua_file")};

			LevelData levelData{loadLevelFromJson(root)};
			levelData.setPackPath(mPath);
			levelData.setLevelRootPath(p);
			levelData.setStyleRootPath(getStyleData(levelData.getStyleId()).getRootPath());
			levelData.setLuaScriptPath(luaScriptPath);
			levelDataMap.insert(make_pair(levelData.getId(), levelData));
			levelIdsByPackMap[levelData.getPackPath()].push_back(levelData.getId());
		}
	}
	void loadProfiles()
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>("Profiles/", ".json"))
		{
			string fileName{getNameFromPath(p, "Profiles/", ".json")};

			ProfileData profileData{loadProfileFromJson(getRootFromFile(p))};
			profileDataMap.insert(make_pair(profileData.getName(), profileData));
		}
	}
	void loadEvents(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Events/", ".json"))
		{
			EventData eventData{getRootFromFile(p)};
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
		for(const auto& n : getCurrentProfile().getTrackedNames()) profileRoot["trackedNames"].append(n);

		writer.write(o, profileRoot); o.flush(); o.close();
	}

	vector<LevelData> getAllLevelData()
	{
		vector<LevelData> result;
		for(const auto& pair : levelDataMap) result.push_back(pair.second);
		return result;
	}
	vector<string> getAllLevelIds()
	{
		vector<LevelData> levelDataVector{getAllLevelData()};
		sort(begin(levelDataVector), end(levelDataVector),
		[](LevelData a, LevelData b)
		{
			if(a.getPackPath() == b.getPackPath()) return a.getMenuPriority() < b.getMenuPriority();
			return a.getPackPath() < b.getPackPath();
		});

		vector<string> result;
		for(auto& ld : levelDataVector) if(ld.getSelectable()) result.push_back(ld.getId());
		return result;
	}
	vector<string> getLevelIdsByPack(string mPackPath)
	{
		vector<LevelData> levelDataVector;
		for(string id : levelIdsByPackMap[mPackPath]) levelDataVector.push_back(getLevelData(id));

		sort(begin(levelDataVector), end(levelDataVector),
		[](LevelData a, LevelData b)
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
		for(const auto& packPair : packDataMap) result.push_back(packPair.first);
		return result;
	}

	void refreshVolumes()
	{
		soundPlayer.setVolume(getSoundVolume());
		for(const auto& pair : assetManager.getMusics()) pair.second->setVolume(getMusicVolume());
	}
	void stopAllMusic() { assetManager.stopMusics(); }
	void stopAllSounds() { soundPlayer.stop(); }
	void playSound(const string& mId)
	{
		if(getNoSound()) return;
		auto soundPtr(getSoundPtr(mId));
		if(soundPtr != nullptr) soundPtr->play();
	}

	Font& getFont(const string& mId) 				{ return assetManager.getFont(mId); }
	Sound* getSoundPtr(const string& mId) 			{ return assetManager.hasSoundBuffer(mId) ? &soundPlayer.create(assetManager.getSoundBuffer(mId)).getSound() : nullptr; }
	Music* getMusicPtr(const string& mId) 			{ return assetManager.hasMusic(mId) ? &assetManager.getMusic(mId) : nullptr; }
	MusicData getMusicData(const string& mId) 		{ return musicDataMap.find(mId)->second; }
	StyleData getStyleData(const string& mId) 		{ return styleDataMap.find(mId)->second; }
	LevelData getLevelData(const string& mId) 		{ return levelDataMap.find(mId)->second; }
	PackData getPackData(const string& mId) 		{ return packDataMap.find(mId)->second; }

	float getScore(const string& mId) 				{ return getCurrentProfile().getScore(mId); }
	void setScore(const string& mId, float mScore)	{ getCurrentProfile().setScore(mId, mScore); }

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
		for(auto& pair : profileDataMap) result.push_back(pair.second.getName());
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
