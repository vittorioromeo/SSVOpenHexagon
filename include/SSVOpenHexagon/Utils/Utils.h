// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS
#define HG_UTILS

#include <string>
#include <sstream>
#include <unordered_set>
#include <SSVUtilsJson/SSVUtilsJson.h>
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
		inline float getSaturated(float mValue) { return std::max(0.f, std::min(1.f, mValue)); }
		inline float getSmootherStep(float edge0, float edge1, float x)
		{
			x = getSaturated((x - edge0)/(edge1 - edge0));
			return x * x * x * (x * (x * 6 - 15) + 10);
		}

		sf::Color getColorFromHue(double mHue);
		sf::Color getColorDarkened(sf::Color mColor, float mMultiplier);

		MusicData loadMusicFromJson(const ssvuj::Value& mRoot);
		ProfileData loadProfileFromJson(const ssvuj::Value& mRoot);

		std::string getLocalValidator(const std::string& mId, float mDifficultyMult);

		void shakeCamera(ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

		std::unordered_set<std::string> getIncludedLuaFileNames(const std::string& mLuaScript);
		void recursiveFillIncludedLuaFileNames(std::unordered_set<std::string>& mLuaScriptNames, const std::string& mPackPath, const std::string& mLuaScript);

		sf::Color transformHue(const sf::Color& in, float H);
	}
}

#endif
