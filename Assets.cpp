#include <iostream>
#include <string>
#include <json/json.h>
#include <json/reader.h>
#include <fstream>
#include <map>
#include "boost/filesystem.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "Assets.h"
#include "MusicData.h"
#include "LevelData.h"
#include "StyleData.h"
#include "Utils.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

namespace hg
{
	map<string, Font> fontsMap;
	map<string, SoundBuffer*> soundBufferPtrsMap;
	map<string, Sound*> soundPtrsMap;
	map<string, Music*> musicPtrsMap;
	map<string, MusicData> musicDataMap;
	map<string, StyleData> styleDataMap;
	map<string, LevelData> levelDataMap;
	Json::Value scoreRoot;
	map<string, Json::Value> configRootsMap;

	void loadAssets()
	{
		log("loading fonts");
		loadFonts();

		log("loading sounds");
		loadSounds();

		log("loading music");
		loadMusic();

		log("loading music data");
		loadMusicData();

		log("loading style data");
		loadStyleData();

		log("loading level data");
		loadLevelData();

		log("loading scores");
		loadScores();
	}

	void loadFonts()
	{
		for(auto filePath : getAllFilePaths("Fonts/", ".ttf"))
		{
			string fileName = path(filePath).stem().string();

			Font font;
			font.loadFromFile(filePath);			
			fontsMap.insert(make_pair(fileName, font));
		}
	}
	void loadSounds()
	{
		for(auto filePath : getAllFilePaths("Sounds/", ".ogg"))
		{
			string fileName = path(filePath).stem().string();

			SoundBuffer* soundBuffer{new SoundBuffer};
			soundBuffer->loadFromFile(filePath);
			soundBufferPtrsMap.insert(make_pair(fileName, soundBuffer));
			
			Sound* soundPtr{new Sound};
			soundPtr->setBuffer(*soundBuffer);
			soundPtr->setVolume(getSoundVolume());
			soundPtrsMap.insert(make_pair(fileName, soundPtr));
		}
	}	
	void loadMusic()
	{
		for(auto filePath : getAllFilePaths("Music/", ".ogg"))
		{
			string fileName = path(filePath).stem().string();

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
	void loadScores()
	{
		scoreRoot = getJsonFileRoot("scores.json");
	}
	void loadConfigs()
	{
		for(auto filePath : getAllFilePaths("Configs/", ".json"))
		{
			string fileName = path(filePath).stem().string();

			Json::Value root{getJsonFileRoot(filePath)};
			configRootsMap.insert(make_pair(fileName, root));
		}
	}

	void saveScores()
	{
		Json::StyledStreamWriter writer;
		ofstream test("scores.json", std::ifstream::binary);

		writer.write(test, scoreRoot);
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
	Json::Value getConfigRoot(string mId)	{ return configRootsMap.find(mId)->second; }

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
		[](LevelData a, LevelData b) -> bool
		{
			return a.getMenuPriority() < b.getMenuPriority(); 
		});

		vector<string> result;
		for(auto levelData : levelDataVector) result.push_back(levelData.getId());
		return result;
	}

	float getScore(string mId)
	{
		return scoreRoot[mId].asFloat();
	}
	void setScore(string mId, float mScore)
	{
		scoreRoot[mId] = mScore;
	}
}
