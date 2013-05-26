// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS
#define HG_UTILS

#include <string>
#include <sstream>
#include <unordered_set>
#include <SSVJsonCpp/SSVJsonCpp.h>
#include <SFML/Graphics.hpp>
#include <SSVStart/SSVStart.h>
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Data/ProfileData.h"
#include "SSVOpenHexagon/Data/MusicData.h"
#include "SSVOpenHexagon/Data/StyleData.h"

namespace hg
{
	namespace Utils
	{
		sf::Color getColorFromHue(double mHue);
		sf::Color getColorDarkened(sf::Color mColor, float mMultiplier);
		sf::Color getColorFromJsonArray(Json::Value mArray);

		LevelData loadLevelFromJson(const Json::Value& mRoot);
		MusicData loadMusicFromJson(const Json::Value& mRoot);
		StyleData loadStyleFromJson(const Json::Value& mRoot);
		ProfileData loadProfileFromJson(const Json::Value& mRoot);

		std::string getLocalValidator(const std::string& mId, float mDifficultyMult);

		void shakeCamera(ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

		std::unordered_set<std::string> getIncludedLuaFileNames(const std::string& mLuaScript);
		void recursiveFillIncludedLuaFileNames(std::unordered_set<std::string>& mLuaScriptNames, const std::string& mPackPath, const std::string& mLuaScript);
	}
}

#endif
