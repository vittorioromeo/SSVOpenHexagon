// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS
#define HG_UTILS

#include <string>
#include <sstream>
#include <set>
#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Data/ProfileData.h"
#include "SSVOpenHexagon/Data/MusicData.h"
#include "SSVOpenHexagon/Data/StyleData.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	namespace Utils
	{
		template<typename T> float getGlobalLeft(const T& mElement)		{ return mElement.getGlobalBounds().left; }
		template<typename T> float getGlobalRight(const T& mElement)	{ return mElement.getGlobalBounds().left + mElement.getGlobalBounds().width; }
		template<typename T> float getGlobalTop(const T& mElement)		{ return mElement.getGlobalBounds().top; }
		template<typename T> float getGlobalBottom(const T& mElement)	{ return mElement.getGlobalBounds().top + mElement.getGlobalBounds().height; }
		template<typename T> float getGlobalWidth(const T& mElement)	{ return mElement.getGlobalBounds().width; }
		template<typename T> float getGlobalHeight(const T& mElement)	{ return mElement.getGlobalBounds().height; }

		template<typename T> float getLocalLeft(const T& mElement)		{ return mElement.getLocalBounds().left; }
		template<typename T> float getLocalRight(const T& mElement)		{ return mElement.getLocalBounds().left + mElement.getLocalBounds().width; }
		template<typename T> float getLocalTop(const T& mElement)		{ return mElement.getLocalBounds().top; }
		template<typename T> float getLocalBottom(const T& mElement)	{ return mElement.getLocalBounds().top + mElement.getLocalBounds().height; }
		template<typename T> float getLocalWidth(const T& mElement)		{ return mElement.getLocalBounds().width; }
		template<typename T> float getLocalHeight(const T& mElement)	{ return mElement.getLocalBounds().height; }

		inline float getSaturated(float mValue) { return std::max(0.f, std::min(1.f, mValue)); }
		inline float getSmootherStep(float edge0, float edge1, float x)
		{
			x = getSaturated((x - edge0)/(edge1 - edge0));
			return x * x * x * (x * (x * 6 - 15) + 10);
		}

		sf::Color getColorFromHue(double mHue);
		sf::Color getColorDarkened(sf::Color mColor, float mMultiplier);

		MusicData loadMusicFromJson(const ssvuj::Obj& mRoot);
		ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot);

		std::string getLocalValidator(const std::string& mId, float mDifficultyMult);

		void shakeCamera(ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

		std::set<std::string> getIncludedLuaFileNames(const std::string& mLuaScript);
		void recursiveFillIncludedLuaFileNames(std::set<std::string>& mLuaScriptNames, const Path& mPackPath, const std::string& mLuaScript);

		sf::Color transformHue(const sf::Color& in, float H);

		inline void runLuaFile(Lua::LuaContext& mLua, const std::string& mFileName)
		{
			std::ifstream s{mFileName};
			try { mLua.executeCode(s); }
			catch(std::runtime_error& mError)
			{
				ssvu::lo << ssvu::lt("hg::Utils::runLuaFile") << "Fatal lua error" << std::endl
					<< ssvu::lt("hg::Utils::runLuaFile") << "Filename: " << mFileName << std::endl
					<< ssvu::lt("hg::Utils::runLuaFile") << "Error: " << mError.what() << std::endl << std::endl;
			}
		}
		template<typename R, typename... Args> inline R runLuaFunction(Lua::LuaContext& mLua, const std::string& mName, const Args&... mArgs)
		{
			try { return mLua.callLuaFunction<R>(mName, std::make_tuple(mArgs...)); }
			catch(std::runtime_error& mError)
			{
				std::cout << mName << std::endl << "LUA runtime error: " << std::endl << ssvu::toStr(mError.what()) << std::endl << std::endl;
			}

			return R();
		}
	}
}

#endif
