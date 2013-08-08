// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Online/Definitions.h"
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Data/MusicData.h"

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
	HGAssets::HGAssets(bool mLevelsOnly) : levelsOnly{mLevelsOnly}
	{
		if(!levelsOnly) loadAssetsFromJson(assetManager, "Assets/", readFromFile("Assets/assets.json"));
		loadAssets();

		for(auto& v : levelDataIdsByPack) sort(begin(v.second), end(v.second), [&](const string& mA, const string& mB){ return levelDatas[mA]->menuPriority < levelDatas[mB]->menuPriority; });
		sort(begin(packIds), end(packIds), [&](const string& mA, const string& mB){ return packDatas[mA]->priority < packDatas[mB]->priority; });
	}




	void HGAssets::loadAssets()
	{
		lo << lt("::loadAssets") << "loading local profiles" << endl; loadLocalProfiles();

		for(const auto& packPath : getScan<Mode::Single, Type::Folder>("Packs/"))
		{
			string packName{packPath.substr(6, packPath.size() - 7)}, packLua;
			for(const auto& p : getScan<Mode::Recurse, Type::File, Pick::ByExt>(packPath, ".lua")) packLua.append(getFileContents(p));
			string packHash{Online::getMD5Hash(packLua + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};

			ssvuj::Obj packRoot{readFromFile(packPath + "/pack.json")};
			auto packData(new PackData{packName, as<string>(packRoot, "name"), as<float>(packRoot, "priority"), packHash});
			packDatas.insert(make_pair(packName, Uptr<PackData>(packData)));
		}

		for(auto& p : packDatas)
		{
			const auto& pd(p.second);
			string packId{pd->id}, packPath{"Packs/" + packId + "/"};
			packIds.push_back(packId);
			packPaths.push_back(packPath);
			if(!levelsOnly) {	lo << lt("::loadAssets") << "loading " << packId << " music" << endl;			loadMusic(packPath); }
			if(!levelsOnly) {	lo << lt("::loadAssets") << "loading " << packId << " music data" << endl;		loadMusicData(packPath); }
								lo << lt("::loadAssets") << "loading " << packId << " style data" << endl;		loadStyleData(packPath);
								lo << lt("::loadAssets") << "loading " << packId << " level data" << endl;		loadLevelData(packPath);
			if(!levelsOnly) {	lo << lt("::loadAssets") << "loading " << packId << " custom sounds" << endl;	loadCustomSounds(packId, packPath); }
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
			music.setVolume(Config::getMusicVolume());
			music.setLoop(true);
		}
	}
	void HGAssets::loadMusicData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Music/", ".json"))
		{
			MusicData musicData{loadMusicFromJson(readFromFile(p))};
			musicDataMap.insert(make_pair(musicData.id, musicData));
		}
	}
	void HGAssets::loadStyleData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Styles/", ".json"))
		{
			StyleData styleData{readFromFile(p)};
			styleData.setRootPath(p);
			styleDataMap.insert(make_pair(styleData.id, styleData));
		}
	}
	void HGAssets::loadLevelData(const string& mPath)
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>(mPath + "Levels/", ".json"))
		{
			auto levelData(new LevelData{readFromFile(p), mPath});
			levelDataIdsByPack[levelData->packPath].push_back(levelData->id);
			levelDatas.insert(make_pair(levelData->id, Uptr<LevelData>(levelData)));
		}
	}
	void HGAssets::loadLocalProfiles()
	{
		for(const auto& p : getScan<Mode::Single, Type::File, Pick::ByExt>("Profiles/", ".json"))
		{
			string fileName{getNameFromPath(p, "Profiles/", ".json")};

			ProfileData profileData{loadProfileFromJson(readFromFile(p))};
			profileDataMap.insert(make_pair(profileData.getName(), profileData));
		}
	}

	void HGAssets::saveCurrentLocalProfile()
	{
		if(currentProfilePtr == nullptr) return;

		ssvuj::Obj profileRoot;
		profileRoot["version"] = Config::getVersion();
		profileRoot["name"] = getCurrentLocalProfile().getName();
		profileRoot["scores"] = getCurrentLocalProfile().getScores();
		for(const auto& n : getCurrentLocalProfile().getTrackedNames()) profileRoot["trackedNames"].append(n);
		ssvuj::writeToFile(profileRoot, getCurrentLocalProfileFilePath());
	}

	MusicData HGAssets::getMusicData(const string& mId) 		{ return musicDataMap.find(mId)->second; }
	StyleData HGAssets::getStyleData(const string& mId) 		{ return styleDataMap.find(mId)->second; }

	float HGAssets::getLocalScore(const string& mId) 				{ return getCurrentLocalProfile().getScore(mId); }
	void HGAssets::setLocalScore(const string& mId, float mScore)	{ getCurrentLocalProfile().setScore(mId, mScore); }

	void HGAssets::setCurrentLocalProfile(const string& mName) { currentProfilePtr = &profileDataMap.find(mName)->second; }
	ProfileData& HGAssets::getCurrentLocalProfile() { return *currentProfilePtr; }
	const ProfileData& HGAssets::getCurrentLocalProfile() const { return *currentProfilePtr; }
	string HGAssets::getCurrentLocalProfileFilePath() { return "Profiles/" + currentProfilePtr->getName() + ".json"; }
	void HGAssets::createLocalProfile(const string& mName)
	{
		ssvuj::Obj root;
		root["name"] = mName;
		root["scores"] = {};
		ssvuj::writeToFile(root, "Profiles/" + mName + ".json");

		profileDataMap.clear();
		loadLocalProfiles();
	}
	int HGAssets::getLocalProfilesSize() { return profileDataMap.size(); }
	vector<string> HGAssets::getLocalProfileNames()
	{
		vector<string> result;
		for(auto& pair : profileDataMap) result.push_back(pair.second.getName());
		return result;
	}
	string HGAssets::getFirstLocalProfileName() { return begin(profileDataMap)->second.getName(); }

	void HGAssets::refreshVolumes()	{ soundPlayer.setVolume(Config::getSoundVolume()); musicPlayer.setVolume(Config::getMusicVolume()); }
	void HGAssets::stopMusics()	{ musicPlayer.stop(); }
	void HGAssets::stopSounds()	{ soundPlayer.stop(); }
	void HGAssets::playSound(const string& mId, SoundPlayer::Mode mMode)	{ if(Config::getNoSound() || !assetManager.has<SoundBuffer>(mId)) return; soundPlayer.play(assetManager.get<SoundBuffer>(mId), mMode); }
	void HGAssets::playMusic(const std::string& mId, Time mPlayingOffset)	{ if(assetManager.has<Music>(mId)) musicPlayer.play(assetManager.get<Music>(mId), mPlayingOffset); }
}
