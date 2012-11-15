#include <iostream>
#include <string>
#include <json/json.h>
#include <json/reader.h>
#include <fstream>
#include <map>
#include "boost/filesystem.hpp"
#include <SFML/Audio.hpp>
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
	map<string, SoundBuffer*> soundBufferPtrsMap;
	map<string, Sound*> soundPtrsMap;
	map<string, Music*> musicPtrsMap;
	map<string, MusicData> musicDataMap;
	map<string, StyleData> styleDataMap;
	map<string, LevelData> levelDataMap;

	void loadAssets()
	{
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
			levelDataMap.insert(make_pair(levelData.getName(), levelData)); // replace with getId
		}
	}

	void stopAllSounds() { for(auto pair : soundPtrsMap) pair.second->stop(); }
	void playSound(string mId) { if(!getNoSound()) getSoundPtr(mId)->play(); }

	Sound* getSoundPtr(string mId) { return soundPtrsMap.find(mId)->second; }
	Music* getMusicPtr(string mId) { return musicPtrsMap.find(mId)->second; }
	MusicData getMusicData(string mId) { return musicDataMap.find(mId)->second; }
	StyleData getStyleData(string mId) { return styleDataMap.find(mId)->second; }
	LevelData getLevelData(string mId) { return levelDataMap.find(mId)->second; }
}
