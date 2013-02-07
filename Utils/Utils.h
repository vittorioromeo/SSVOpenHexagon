// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef UTILS_H_HG
#define UTILS_H_HG

#include <string>
#include <sstream>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <SFML/Graphics.hpp>
#include <SSVStart.h>
#include "Data/LevelData.h"
#include "Data/ProfileData.h"
#include "Data/MusicData.h"
#include "Data/StyleData.h"
#include "Global/Config.h"

namespace hg
{
	sf::Color getColorFromHue(double);
	sf::Color getColorDarkened(sf::Color, float);
	sf::Color getColorFromJsonArray(Json::Value mArray);

	Json::Value getJsonFileRoot(std::string mFilePath);

	LevelData loadLevelFromJson(Json::Value mRoot);
	MusicData loadMusicFromJson(Json::Value mRoot);
	StyleData loadStyleFromJson(Json::Value mRoot);
	ProfileData loadProfileFromJson(Json::Value mRoot);

	std::string getScoreValidator(std::string mId, float mDifficultyMult);
}

#endif /* UTILS_H_HG */
