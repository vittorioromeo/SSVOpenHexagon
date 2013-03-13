// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <json/json.h>
#include <json/reader.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include "Utils/Utils.h"
#include "Global/Config.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace hg::UtilsJson;

namespace hg
{
	namespace UtilsJson
	{
		using namespace Json;

		template<> int getValue(const Value& mRoot, const string& mValue) 			{ return mRoot[mValue].asInt(); }
		template<> float getValue(const Value& mRoot, const string& mValue) 		{ return mRoot[mValue].asFloat(); }
		template<> bool getValue(const Value& mRoot, const string& mValue) 			{ return mRoot[mValue].asBool(); }
		template<> string getValue(const Value& mRoot, const string& mValue) 		{ return mRoot[mValue].asString(); }
		template<> char const* getValue(const Value& mRoot, const string& mValue)	{ return mRoot[mValue].asCString(); }

		Value getRootFromFile(const string& mPath) { return getRootFromString(getFileContents(mPath)); }
		Value getRootFromString(const string& mString)
		{
			Value result; Reader reader;
			if(!reader.parse(mString, result, false)) log(reader.getFormatedErrorMessages() + "\n" + "From: [" + mString + "]", "JSON Error");
			return result;
		}
	}

	namespace Utils
	{
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
		Color getColorFromJsonArray(Json::Value mArray) { return Color(mArray[0].asFloat(), mArray[1].asFloat(), mArray[2].asFloat(), mArray[3].asFloat()); }

		string getFileContents(const string& mPath)
		{
			FILE* fptr{fopen(mPath.c_str(), "rb")};
			fseek(fptr, 0, SEEK_END);
			size_t fsize(ftell(fptr));
			fseek(fptr, 0, SEEK_SET);
			string content; content.resize(fsize);
			if(fread(const_cast<char*>(content.c_str()), 1, fsize, fptr) != fsize) log("Error: " + mPath, "File loading");
			fclose(fptr); return content;
		}

		LevelData loadLevelFromJson(Json::Value mRoot) { LevelData result{mRoot}; for(auto event : mRoot["events"]) result.addEvent(event); return result; }
		MusicData loadMusicFromJson(const Json::Value& mRoot)
		{
			MusicData result{getValue<string>(mRoot, "id"), getValue<string>(mRoot, "file_name"), getValue<string>(mRoot, "name"), getValue<string>(mRoot, "album"),
				getValue<string>(mRoot, "author")};
			for(auto segment : mRoot["segments"]) result.addSegment(segment["time"].asInt());
			return result;
		}
		StyleData loadStyleFromJson(Json::Value mRoot) { return {mRoot}; }
		ProfileData loadProfileFromJson(const Json::Value& mRoot) { return {getValue<float>(mRoot, "version"), getValue<string>(mRoot, "name"), mRoot["scores"]}; }

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
			unordered_set<string> current{getIncludedLuaFileNames(mLuaScript)};
			for(auto& name : current)
			{
				mLuaScriptNames.insert(name);
				recursiveFillIncludedLuaFileNames(mLuaScriptNames, mPackPath, getFileContents(mPackPath + "/Scripts/" + name));
			}
		}
	}
}
