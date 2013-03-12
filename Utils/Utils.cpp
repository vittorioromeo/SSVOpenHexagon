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

namespace hg
{
	template<> int getJsonValue(const Json::Value& mRoot, const string& mValue) 		{ return mRoot[mValue].asInt(); }
	template<> float getJsonValue(const Json::Value& mRoot, const string& mValue) 		{ return mRoot[mValue].asFloat(); }
	template<> bool getJsonValue(const Json::Value& mRoot, const string& mValue) 		{ return mRoot[mValue].asBool(); }
	template<> string getJsonValue(const Json::Value& mRoot, const string& mValue) 		{ return mRoot[mValue].asString(); }
	template<> char const* getJsonValue(const Json::Value& mRoot, const string& mValue)	{ return mRoot[mValue].asCString(); }

	Color getColorFromHue(double h)
	{
		double s{1}, v{1}, r{0}, g{0}, b{0};
		int i(floor(h * 6));
		double f{h * 6 - i}, p{v * (1 - s)}, q{v * (1 - f * s)}, t{v * (1 - (1 - f) * s)};

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

	string getFileContents(const string& mFilePath)
	{
		FILE* fptr{fopen(mFilePath.c_str(), "rb")};
		fseek(fptr, 0, SEEK_END);
		size_t fsize(ftell(fptr));
		fseek(fptr, 0, SEEK_SET);
		string content; content.resize(fsize);
		if(fread(const_cast<char*>(content.c_str()), 1, fsize, fptr) != fsize) log("Error: " + mFilePath, "File loading");

		fclose(fptr);
		return content;
	}
	Json::Value getJsonFileRoot(const string& mFilePath)
	{
		Json::Value root;
		Json::Reader reader;
		ifstream stream{mFilePath, ifstream::binary};

		if(!reader.parse(stream, root, false)) log(reader.getFormatedErrorMessages() + " [" + mFilePath + "]", "JSON Error");
		return root;
	}
	Json::Value getJsonFromString(const string& mString) { Json::Value result; Json::Reader reader; reader.parse(mString, result); return result; }

	LevelData loadLevelFromJson(Json::Value mRoot)
	{
		auto result = LevelData{mRoot};
		for(Json::Value event : mRoot["events"]) result.addEvent(event);
		return result;
	}
	MusicData loadMusicFromJson(Json::Value mRoot)
	{
		string id				{ mRoot["id"].asString() };
		string fileName			{ mRoot["file_name"].asString() };
		string name			 	{ mRoot["name"].asString() };
		string album	 		{ mRoot["album"].asString() };
		string author 			{ mRoot["author"].asString() };

		MusicData result{id, fileName, name, album, author};
		for(Json::Value segment : mRoot["segments"]) result.addSegment(segment["time"].asInt());
		return result;
	}
	StyleData loadStyleFromJson(Json::Value mRoot) { return StyleData(mRoot); }
	ProfileData loadProfileFromJson(Json::Value mRoot)
	{
		float version 		{ mRoot["version"].asFloat() };
		string name			{ mRoot["name"].asString() };
		Json::Value scores	{ mRoot["scores"] };

		ProfileData result{version, name, scores};
		return result;
	}

	string getScoreValidator(const string& mId, float mDifficultyMult) { return mId + "_m_" + toStr(mDifficultyMult); }

	void shakeCamera(TimelineManager& mTimelineManager, Camera& mCamera)
	{
		int s{7};
		Vector2f oldCenter{mCamera.getCenter()};
		Timeline& timeline(mTimelineManager.create());

		for(int i{s}; i > 0; --i)
		{
			timeline.append<Do>([&mCamera, oldCenter, i]{ mCamera.centerOn(oldCenter + Vector2f(getRnd(-i, i), getRnd(-i, i))); });
			timeline.append<Wait>(1);
			timeline.append<Go>(0, 3);
		}

		timeline.append<Do>([&mCamera, oldCenter]{ mCamera.centerOn(oldCenter); });
	}

	unordered_set<string> getIncludedLuaFileNames(const string& mLuaScript)
	{
		string script{mLuaScript};
		string toFind{"execScript("};
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
			string luaFilePath{mPackPath + "/Scripts/" + name};
			recursiveFillIncludedLuaFileNames(mLuaScriptNames, mPackPath, getFileContents(luaFilePath));
		}
	}
}
