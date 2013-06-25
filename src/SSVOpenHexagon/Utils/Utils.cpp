// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Global/Config.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvuj;
using namespace ssvu::FileSystem;
using namespace ssvu;

namespace hg
{
	namespace Utils
	{
		float getSaturated(float mValue) { return max(0.0f, min(1.0f, mValue)); }
		float getSmootherStep(float edge0, float edge1, float x)
		{
			x = getSaturated((x - edge0)/(edge1 - edge0));
			return x * x * x * (x * (x * 6 - 15) + 10);
		}

		Color getColorFromHue(double mHue)
		{
			double s{1}, v{1}, r{0}, g{0}, b{0};
			int i(floor(mHue * 6));
			double f{mHue * 6 - i}, p{v * (1 - s)}, q{v * (1 - f * s)}, t{v * (1 - (1 - f) * s)};

			switch(i % 6)
			{
				case 0: r = v, g = t, b = p; break;
				case 1: r = q, g = v, b = p; break;
				case 2: r = p, g = v, b = t; break;
				case 3: r = p, g = q, b = v; break;
				case 4: r = t, g = p, b = v; break;
				case 5: r = v, g = p, b = q; break;
			}

			return Color(r * 255, g * 255, b * 255, 255);
		}
		Color getColorDarkened(Color mColor, float mMultiplier) { mColor.r /= mMultiplier; mColor.b /= mMultiplier; mColor.g /= mMultiplier; return mColor; }
		Color getColorFromJsonArray(const Json::Value& mArray) { return Color(as<float>(mArray, 0), as<float>(mArray, 1), as<float>(mArray, 2), as<float>(mArray, 3)); }

		LevelData loadLevelFromJson(const Json::Value& mRoot) { LevelData result{mRoot}; for(const auto& event : mRoot["events"]) result.addEvent(event); return result; }
		MusicData loadMusicFromJson(const Json::Value& mRoot)
		{
			MusicData result{as<string>(mRoot, "id"), as<string>(mRoot, "file_name"), as<string>(mRoot, "name"), as<string>(mRoot, "album"), as<string>(mRoot, "author")};
			for(const auto& segment : mRoot["segments"]) result.addSegment(as<int>(segment, "time"));
			return result;
		}
		StyleData loadStyleFromJson(const Json::Value& mRoot) { return {mRoot}; }
		ProfileData loadProfileFromJson(const Json::Value& mRoot) { return {as<float>(mRoot, "version"), as<string>(mRoot, "name"), mRoot["scores"], as<vector<string>>(mRoot, "trackedNames", {})}; }

		string getLocalValidator(const string& mId, float mDifficultyMult) { return mId + "_m_" + toStr(mDifficultyMult); }

		void shakeCamera(TimelineManager& mTimelineManager, Camera& mCamera)
		{
			int s{7};
			Vector2f oldCenter{mCamera.getCenter()};
			Timeline& timeline(mTimelineManager.create());

			for(int i{s}; i > 0; --i)
			{
				timeline.append<Do>([&mCamera, oldCenter, i]{ mCamera.centerOn(oldCenter + Vector2f(getRnd(-i, i), getRnd(-i, i))); });
				timeline.append<Wait>(1); timeline.append<Go>(0, 3);
			}

			timeline.append<Do>([&mCamera, oldCenter]{ mCamera.centerOn(oldCenter); });
		}

		unordered_set<string> getIncludedLuaFileNames(const string& mLuaScript)
		{
			string script{mLuaScript}, toFind{"execScript("};
			unordered_set<string> result;

			size_t pos{script.find(toFind, 0)};
			while(pos != string::npos)
			{
				size_t startPos{pos};
				string untilEnd{script.substr(startPos + toFind.length() + 1, script.length() - startPos)};
				size_t lastPos{untilEnd.find("\"", 0)};
				string luaFileName{untilEnd.substr(0, lastPos)};

				result.insert(luaFileName);
				pos = script.find(toFind, pos + 1);
			}

			return result;
		}
		void recursiveFillIncludedLuaFileNames(unordered_set<string>& mLuaScriptNames, const string& mPackPath, const string& mLuaScript)
		{
			for(const auto& name : getIncludedLuaFileNames(mLuaScript))
			{
				mLuaScriptNames.insert(name);
				recursiveFillIncludedLuaFileNames(mLuaScriptNames, mPackPath, getFileContents(mPackPath + "/Scripts/" + name));
			}
		}
	}
}
