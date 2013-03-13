// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS
#define HG_UTILS

#include <string>
#include <sstream>
#include <unordered_set>
#include <json/json.h>
#include <json/reader.h>
#include <SFML/Graphics.hpp>
#include <SSVStart.h>
#include "Data/LevelData.h"
#include "Data/ProfileData.h"
#include "Data/MusicData.h"
#include "Data/StyleData.h"

namespace hg
{
	namespace UtilsJson
	{
		template<typename T> T getValue(const Json::Value& mRoot, const std::string& mValue);
		template<typename T> T getValueOrDefault(const Json::Value& mRoot, const std::string& mValue, T mDefault) { return mRoot.isMember(mValue) ? getValue<T>(mRoot, mValue) : mDefault; }
		Json::Value getRootFromFile(const std::string& mPath);
		Json::Value getRootFromString(const std::string& mString);
	}

	namespace Utils
	{
		sf::Color getColorFromHue(double mHue);
		sf::Color getColorDarkened(sf::Color mColor, float mMultiplier);
		sf::Color getColorFromJsonArray(Json::Value mArray);

		std::string getFileContents(const std::string& mPath);

		LevelData loadLevelFromJson(Json::Value mRoot);
		MusicData loadMusicFromJson(const Json::Value& mRoot);
		StyleData loadStyleFromJson(Json::Value mRoot);
		ProfileData loadProfileFromJson(const Json::Value& mRoot);

		std::string getLocalValidator(const std::string& mId, float mDifficultyMult);

		void shakeCamera(ssvs::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

		std::unordered_set<std::string> getIncludedLuaFileNames(const std::string& mLuaScript);
		void recursiveFillIncludedLuaFileNames(std::unordered_set<std::string>& mLuaScriptNames, const std::string& mPackPath, const std::string& mLuaScript);
	}
}

#endif
