// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <fstream>
#include <map>
#include <SFML/Graphics.hpp>
#include <SSVUtilsJson/SSVUtilsJson.h>
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
	SoundPlayer soundPlayer;
	MusicPlayer musicPlayer;
	map<string, MusicData> musicDataMap;
	map<string, StyleData> styleDataMap;
	map<string, LevelData> levelDataMap;
	map<string, ProfileData> profileDataMap;
	map<string, EventData> eventDataMap;
	map<string, PackData> packDataMap;
	ProfileData* currentProfilePtr{nullptr};
	map<string, vector<string>> levelIdsByPackMap;
	vector<string> packPaths;

	void initAssetManager() { loadAssetsFromJson(assetManager, "Assets/", getRootFromFile("Assets/assets.json")); }
	AssetManager& getAssetManager() { return assetManager; }

	void loadAssets()
	{
		log("loading profiles", "::loadAssets"); loadProfiles();

		for(const auto& packPath : getScan<Mode::Single, Type::Folder>("Packs/"))
		{
			string packName{packPath.substr(6, packPath.length() - 7)}, packLua;
			for(const auto& p : getScan<Mode::Recurse, Type::File, Pick::ByExt>(packPath, ".lua")) packLua.append(getFileContents(p));
			string packHash{Online::getMD5Hash(packLua + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};

			ssvuj::Value packRoot{getRootFromFile(packPath + "/pack.json")};
			PackData packData(packName, as<string>(packRoot, "name"), as<float>(packRoot, "priority"), packHash);
			packDataMap.insert(make_pair(packName, packData));
		}

		vector<PackData> packDatas;
		for(auto& pair : packDataMap) packDatas.push_back(pair.second);
		sort(begin(packDatas), end(packDatas), [](const PackData& a, const PackData& b) { return a.getPriority() < b.getPriority(); });

		for(auto& pd : packDatas)
		{
			string packName{pd.getId()}, packPath{"Packs/" + packName + "/"};
			packPaths.push_back("Packs/" + packName + "/");
			log("loading " + packName + " music", "::loadAssets");			loadMusic(packPath);
			log("loading " + packName + " music data", "::loadAssets");		loadMusicData(packPath);
			log("loading " + packName + " style data", "::loadAssets");		loadStyleData(packPath);
			log("loading " + packName + " level data", "::loadAssets");		loadLevelData(packPath);
			log("loading " + packName + " events", "::loadAssets");			loadEvents(packPath);
			log("loading " + packName + " custom sounds", "::loadAssets");	loadCustomSounds(packName, packPath);
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
			ssvuj::Value root{getRootFromFile(p)};
			string luaScriptPath{mPath + "Scripts/" + as<string>(root, "lua_file")};

			LevelData levelData{loadLevelFromJson(root)};

			string trackedVariablesPath{getReplaced(p, ".json", ".tracked")};
			if(exists(trackedVariablesPath)) levelData.loadTrackedVariables(getRootFromFile(trackedVariablesPath));

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

		ssvuj::Value profileRoot;
		profileRoot["version"] = getVersion();
		profileRoot["name"] = getCurrentProfile().getName();
		profileRoot["scores"] = getCurrentProfile().getScores();
		for(const auto& n : getCurrentProfile().getTrackedNames()) profileRoot["trackedNames"].append(n);
		ssvuj::writeRootToFile(profileRoot, getCurrentProfileFilePath());
	}

	vector<LevelData> getAllLevelData()
	{
		vector<LevelData> result;
		for(const auto& pair : levelDataMap) result.push_back(pair.second);
		return result;
	}
	vector<string> getAllLevelIds()
	{
		vector<LevelData> levelDatas{getAllLevelData()};
		sort(begin(levelDatas), end(levelDatas), [](const LevelData& a, const LevelData& b)
		{
			if(a.getPackPath() == b.getPackPath()) return a.getMenuPriority() < b.getMenuPriority();
			return a.getPackPath() < b.getPackPath();
		});

		vector<string> result;
		for(const auto& l : levelDatas) if(l.getSelectable()) result.push_back(l.getId());
		return result;
	}
	vector<string> getLevelIdsByPack(string mPackPath)
	{
		vector<LevelData> levelDatas;
		for(const auto& id : levelIdsByPackMap[mPackPath]) levelDatas.push_back(getLevelData(id));

		sort(begin(levelDatas), end(levelDatas), [](const LevelData& a, const LevelData& b){ return a.getMenuPriority() < b.getMenuPriority(); });

		vector<string> result;
		for(const auto& l : levelDatas) if(l.getSelectable()) result.push_back(l.getId());
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
		musicPlayer.setVolume(getMusicVolume());
	}
	void stopAllMusic() { musicPlayer.stop(); }
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
		ssvuj::Value root;
		root["name"] = mName;
		root["scores"] = {};
		ssvuj::writeRootToFile(root, "Profiles/" + mName + ".json");

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

	void playMusic(const std::string& mId, Time mPlayingOffset) { Music* music{getMusicPtr(mId)}; if(music != nullptr) musicPlayer.play(*music, mPlayingOffset); }

	EventData* createEventData(const string& mId, HexagonGame* mHgPtr)
	{
		EventData* result{new EventData(eventDataMap.find(mId)->second)};
		result->setHexagonGamePtr(mHgPtr);
		return result;
	}
}
