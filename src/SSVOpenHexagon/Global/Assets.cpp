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
	HGAssets::HGAssets()
	{
		loadAssetsFromJson(assetManager, "Assets/", getRootFromFile("Assets/assets.json"));
		loadAssets();

		for(auto& v : levelDataIdsByPack) sort(begin(v.second), end(v.second), [&](const string& mA, const string& mB){ return levelDatas[mA]->menuPriority < levelDatas[mB]->menuPriority; });
	}




	void HGAssets::loadAssets()
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
		sort(begin(packDatas), end(packDatas), [](const PackData& a, const PackData& b) { return a.priority < b.priority; });

		for(auto& pd : packDatas)
		{
			string packName{pd.id}, packPath{"Packs/" + packName + "/"};
			packPaths.push_back("Packs/" + packName + "/");
			log("loading " + packName + " music", "::loadAssets");			loadMusic(packPath);
			log("loading " + packName + " music data", "::loadAssets");		loadMusicData(packPath);
			log("loading " + packName + " style data", "::loadAssets");		loadStyleData(packPath);
			log("loading " + packName + " level data", "::loadAssets");		loadLevelData(packPath);
			log("loading " + packName + " custom sounds", "::loadAssets");	loadCustomSounds(packName, packPath);
		}


	}

	void HGAssets::loadCustomSounds(const string& mPackName, const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Sounds/", ".ogg"))
		{
			const auto& fileName(getNameFromPath(p, mPath + "Sounds/", ""));
			assetManager.load<SoundBuffer>(mPackName + "_" + fileName, p);
		}
	}
	void HGAssets::loadMusic(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Music/", ".ogg"))
		{
			const auto& fileName(getNameFromPath(p, mPath + "Music/", ".ogg"));

			auto& music(assetManager.load<Music>(fileName, p));
			music.setVolume(getMusicVolume());
			music.setLoop(true);
		}
	}
	void HGAssets::loadMusicData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Music/", ".json"))
		{
			MusicData musicData{loadMusicFromJson(getRootFromFile(p))};
			musicDataMap.insert(make_pair(musicData.id, musicData));
		}
	}
	void HGAssets::loadStyleData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Styles/", ".json"))
		{
			StyleData styleData{getRootFromFile(p)};
			styleData.setRootPath(p);
			styleDataMap.insert(make_pair(styleData.id, styleData));
		}
	}
	void HGAssets::loadLevelData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Levels/", ".json"))
		{
			auto levelData(new LevelData{getRootFromFile(p), mPath});
			levelDataIdsByPack[levelData->packPath].push_back(levelData->id);
			levelDatas.insert(make_pair(levelData->id, Uptr<LevelData>(levelData)));
		}
	}
	void HGAssets::loadProfiles()
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>("Profiles/", ".json"))
		{
			string fileName{getNameFromPath(p, "Profiles/", ".json")};

			ProfileData profileData{loadProfileFromJson(getRootFromFile(p))};
			profileDataMap.insert(make_pair(profileData.getName(), profileData));
		}
	}

	void HGAssets::saveCurrentProfile()
	{
		if(currentProfilePtr == nullptr) return;

		ssvuj::Value profileRoot;
		profileRoot["version"] = getVersion();
		profileRoot["name"] = getCurrentProfile().getName();
		profileRoot["scores"] = getCurrentProfile().getScores();
		for(const auto& n : getCurrentProfile().getTrackedNames()) profileRoot["trackedNames"].append(n);
		ssvuj::writeRootToFile(profileRoot, getCurrentProfileFilePath());
	}

	vector<string> HGAssets::getPackPaths() { return packPaths; }
	vector<string> HGAssets::getPackNames()
	{
		vector<string> result;
		for(const auto& packPair : packDataMap) result.push_back(packPair.first);
		sort(begin(result), end(result), [&](const string& mA, const string& mB)
		{
			return packDataMap.at(mA).priority < packDataMap.at(mB).priority;
		});
		return result;
	}




	MusicData HGAssets::getMusicData(const string& mId) 		{ return musicDataMap.find(mId)->second; }
	StyleData HGAssets::getStyleData(const string& mId) 		{ return styleDataMap.find(mId)->second; }
	PackData HGAssets::getPackData(const string& mId) 		{ return packDataMap.find(mId)->second; }

	float HGAssets::getScore(const string& mId) 				{ return getCurrentProfile().getScore(mId); }
	void HGAssets::setScore(const string& mId, float mScore)	{ getCurrentProfile().setScore(mId, mScore); }

	void HGAssets::setCurrentProfile(const string& mName) { currentProfilePtr = &profileDataMap.find(mName)->second; }
	ProfileData& HGAssets::getCurrentProfile() { return *currentProfilePtr; }
	string HGAssets::getCurrentProfileFilePath() { return "Profiles/" + currentProfilePtr->getName() + ".json"; }
	void HGAssets::createProfile(const string& mName)
	{
		ssvuj::Value root;
		root["name"] = mName;
		root["scores"] = {};
		ssvuj::writeRootToFile(root, "Profiles/" + mName + ".json");

		profileDataMap.clear();
		loadProfiles();
	}
	int HGAssets::getProfilesSize() { return profileDataMap.size(); }
	vector<string> HGAssets::getProfileNames()
	{
		vector<string> result;
		for(auto& pair : profileDataMap) result.push_back(pair.second.getName());
		return result;
	}
	string HGAssets::getFirstProfileName() { return profileDataMap.begin()->second.getName(); }

	void HGAssets::refreshVolumes()	{ soundPlayer.setVolume(getSoundVolume()); musicPlayer.setVolume(getMusicVolume()); }
	void HGAssets::stopMusics()	{ musicPlayer.stop(); }
	void HGAssets::stopSounds()	{ soundPlayer.stop(); }
	void HGAssets::playSound(const string& mId, SoundPlayer::Mode mMode)	{ if(getNoSound() || !assetManager.has<SoundBuffer>(mId)) return; soundPlayer.play(assetManager.get<SoundBuffer>(mId), mMode); }
	void HGAssets::playMusic(const std::string& mId, Time mPlayingOffset)	{ if(assetManager.has<Music>(mId)) musicPlayer.play(assetManager.get<Music>(mId), mPlayingOffset); }
}
