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
#include "Data/StyleData.h"
#include "PatternManager.h"

using namespace std;
using namespace sf;

namespace hg
{
	template<class T> void log(T mValue) { cout << toStr(mValue) << endl; }
	template<class T> T toRadians(const T mValue) { return mValue / 57.3f; }
	template<class T> string toStr(const T &t) { ostringstream oss; oss << t; return string(oss.str()); }
	template<class T> int getSign(T value) { if (value > 0) return 1; else return -1; }
	template<class T> int countNewLines(T mValue)
	{
		int result{0};
		for(auto c : mValue) if (c == '\n') result++;
		return result;
	}

	int getRnd(int, int);
	float getSaturated(float);
	float getSmootherStep(float, float, float);
	bool isPointInPolygon(std::vector<Vector2f*>, Vector2f);

	Vector2f getOrbit(const Vector2f&, const float, const float);
	Vector2f getNormalized(const Vector2f);

	Color getColorFromHue(double);
	Color getColorDarkened(Color, float);

	vector<string> getAllFilePaths(string mFolderPath, string mExtension);
	string getFileNameFromFilePath(string mFilePath, string mPrefix, string mSuffix);
	Json::Value getJsonFileRoot(string mFilePath);

	LevelData loadLevelFromJson(Json::Value mRoot);
	MusicData loadMusicFromJson(Json::Value mRoot);
	StyleData loadStyleFromJson(Json::Value mRoot);
	ProfileData loadProfileFromJson(string mId, Json::Value mRoot);

	void parseAndAddPattern(LevelData& mLevelData, Json::Value &mPatternRoot);
	void parseAndAddEvent(LevelData& mLevelData, Json::Value &mEventRoot);
	function<void(PatternManager*)> getAdjPatternFunc(function<void(PatternManager*)> mFunction, float mAdjDelay, float mAdjSpeed, float mAdjThickness);

	void clearAndResetTimeline(Timeline& mTimeline);
}

#endif /* UTILS_H_HG */
