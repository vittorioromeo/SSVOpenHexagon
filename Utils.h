#ifndef UTILS_H_HG
#define UTILS_H_HG

#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include "LevelSettings.h"
#include "PatternManager.h"
#include "boost/filesystem.hpp"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

namespace hg
{	
	template<class T> T toRadians(const T mValue) { return mValue / 57.3f; }
	template <class T> int sign(T value) { if (value > 0) return 1; else return -1; }

	template<class T>
	string toStr(const T &t)
	{
		ostringstream oss;
		oss << t;
		return string(oss.str());
	}

	Vector2f orbit(const Vector2f&, const float, const float);
	Vector2f normalize(const Vector2f);
	Color hue2color(double);
	Color darkenColor(Color, float);

	bool pnpoly(std::vector<Vector2f*>, Vector2f);
	int rnd(int, int);
	float saturate(float);
	float smootherstep(float, float, float);
	
	string exePath(); // windows-only!

	vector<string> getAllJsonPaths(path mPath);

	LevelSettings loadLevelFromJson(PatternManager* pm, Json::Value &mRoot);
	void parseAndAddPattern(PatternManager* pm, LevelSettings& mLevelSettings, Json::Value &mPatternRoot);

	MusicData loadMusicFromJson(Json::Value &mRoot);
}

#endif /* UTILS_H_HG */
