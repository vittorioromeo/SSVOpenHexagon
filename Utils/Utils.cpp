// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <json/json.h>
#include <json/reader.h>
#include <dirent.h>
#include <sys/stat.h>
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/ProfileData.h"
#include "Data/StyleData.h"
#include "Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace ssvs::Utils;

namespace hg
{
	Color getColorFromHue(double h)
	{
		double s{1};
		double v{1};

		double r{0}, g{0}, b{0};

		int i = floor(h * 6);
		double f{h * 6 - i};
		double p{v * (1 - s)};
		double q{v * (1 - f * s)};
		double t{v * (1 - (1 - f) * s)};

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
	Color getColorDarkened(Color mColor, float mMultiplier)
	{
		mColor.r /= mMultiplier;
		mColor.b /= mMultiplier;
		mColor.g /= mMultiplier;
		return mColor;
	}
	Color getColorFromJsonArray(Json::Value mArray)
	{
		return Color(mArray[0].asFloat(), mArray[1].asFloat(), mArray[2].asFloat(), mArray[3].asFloat());
	}

	Json::Value getJsonFileRoot(string mFilePath)
	{
		Json::Value root;
		Json::Reader reader;
		ifstream stream(mFilePath, std::ifstream::binary);

		bool parsingSuccessful = reader.parse( stream, root, false );
		if (!parsingSuccessful) cout << reader.getFormatedErrorMessages() << endl;

		return root;
	}

	LevelData loadLevelFromJson(Json::Value mRoot)
	{
		auto result = LevelData{mRoot};
		for (Json::Value event : mRoot["events"]) result.addEvent(event);
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
		for (Json::Value segment : mRoot["segments"]) result.addSegment(segment["time"].asInt());
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

	string getScoreValidator(string mId, float mDifficultyMult) { return mId + "_m_" + toStr(mDifficultyMult); }
}
