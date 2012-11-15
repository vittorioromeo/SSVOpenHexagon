#ifndef ASSETS_H_
#define ASSETS_H_

#include <iostream>
#include <string>
#include <json/json.h>
#include <json/reader.h>
#include <fstream>
#include <map>
#include "boost/filesystem.hpp"
#include <SFML/Audio.hpp>
#include "MusicData.h"
#include "LevelData.h"
#include "StyleData.h"
#include "Utils.h"

using namespace std;
using namespace sf;

namespace hg
{
	void loadAssets();

	void loadSounds();
	void loadMusic();
	void loadMusicData();
	void loadStyleData();
	void loadLevelData();

	void stopAllSounds();
	void playSound(string mId);

	Sound* getSoundPtr(string mId);
	Music* getMusicPtr(string mId);
	MusicData getMusicData(string mId);
	StyleData getStyleData(string mId);
	LevelData getLevelData(string mId);
}

#endif /* ASSETS_H_ */
