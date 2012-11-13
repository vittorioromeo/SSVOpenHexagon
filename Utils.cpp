#include "Utils.h"
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <json/json.h>
#include <json/reader.h>
#include "LevelSettings.h"
#include "boost/filesystem.hpp"
#include "MusicData.h"

namespace hg
{
	Vector2f orbit(const Vector2f& mParent, const float mDegrees, const float mRadius)
	{
		return Vector2f{ mParent.x + cos(toRadians(mDegrees)) * mRadius, mParent.y + sin(toRadians(mDegrees)) * mRadius };
	}
	Vector2f normalize(const Vector2f mVector)
	{
		float length { std::sqrt((mVector.x * mVector.x) + (mVector.y * mVector.y)) };
		return Vector2f{ mVector.x / length, mVector.y / length };
	}
	Color hue2color(double h)
	{
		double s = 1;
		double v = 1;

		double r, g, b;

		int i = floor(h * 6);
		double f = h * 6 - i;
		double p = v * (1 - s);
		double q = v * (1 - f * s);
		double t = v * (1 - (1 - f) * s);

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
	Color darkenColor(Color mColor, float mMultiplier)
	{
		mColor.r /= mMultiplier;
		mColor.b /= mMultiplier;
		mColor.g /= mMultiplier;
		return mColor;
	}

	int rnd(int min, int max)
	{
	   double x = rand()/static_cast<double>(RAND_MAX);
	   int that = min + static_cast<int>( x * (max - min) );
	   return that;
	}
	bool pnpoly(std::vector<Vector2f*> verts, Vector2f test)
	{
		int nvert = verts.size();
		int i, j, c = 0;
		for (i = 0, j = nvert-1; i < nvert; j = i++) {
			if ( ((verts[i]->y>test.y) != (verts[j]->y>test.y)) &&
					(test.x < (verts[j]->x-verts[i]->x) * (test.y-verts[i]->y) / (verts[j]->y-verts[i]->y) + verts[i]->x) )
				c = !c;
		}
		return c;
	}
	float saturate(float x) { return std::max(0.0f, std::min(1.0f, x)); }
	float smootherstep(float edge0, float edge1, float x)
	{
		x = saturate((x - edge0)/(edge1 - edge0));
		return x*x*x*(x*(x*6 - 15) + 10);
	}

	string exePath()
	{
		char buffer[MAX_PATH];
		GetModuleFileName( NULL, buffer, MAX_PATH );
		string::size_type pos = string( buffer ).find_last_of( "\\/" );
		return string( buffer ).substr( 0, pos);
	}

	vector<string> getAllJsonPaths(path mPath)
	{
		vector<string> result;

		for(auto iter = directory_iterator(mPath); iter != directory_iterator(); iter++)
			if(iter->path().extension() == ".json") result.push_back(iter->path().string());

		return result;
	}
	LevelSettings loadLevelFromJson(PatternManager* pm, Json::Value &mRoot)
	{
		string name			 	{ mRoot["name"].asString() };
		string description 		{ mRoot["description"].asString() };
		string author 			{ mRoot["author"].asString() };
		string music_id			{ mRoot["music_id"].asString() };

		float speedMultiplier  	{ mRoot["speed_multiplier"].asFloat() };
		float speedIncrement 	{ mRoot["speed_increment"].asFloat() };
		float rotationSpeed  	{ mRoot["rotation_speed"].asFloat() };
		float rotationIncrement	{ mRoot["rotation_increment"].asFloat() };
		float delayMultiplier	{ mRoot["delay_multiplier"].asFloat() };
		float delayIncrement	{ mRoot["delay_increment"].asFloat() };
		float fastSpin			{ mRoot["fast_spin"].asFloat() };
		int sidesStart			{ mRoot["sides_start"].asInt() };
		int sidesMin 			{ mRoot["sides_min"].asInt() };
		int sidesMax			{ mRoot["sides_max"].asInt() };
		float incrementTime 	{ mRoot["increment_time"].asFloat() };

		auto result = LevelSettings{name, description, author, music_id, speedMultiplier, speedIncrement, rotationSpeed,
			rotationIncrement, delayMultiplier, delayIncrement, fastSpin, sidesStart, sidesMin, sidesMax, incrementTime};

		for (Json::Value pattern : mRoot["patterns"]) parseAndAddPattern(pm, result, pattern);

		return result;
	}
	void parseAndAddPattern(PatternManager* pm, LevelSettings& mLevelSettings, Json::Value &mPatternRoot)
	{
		string type	{ mPatternRoot["type"].asString() };
		int chance	{ mPatternRoot["chance"].asInt() };

		if(type == "alternate_wall_barrage")
		{
			int times{mPatternRoot["times"].asInt()};
			int div{mPatternRoot["div"].asInt()};
			mLevelSettings.addPattern([=]{ pm->alternateWallBarrage(times, div); }, chance);
		}
		else if(type == "barrage_spiral")
		{
			int times{mPatternRoot["times"].asInt()};
			float delayMultiplier{mPatternRoot["delay_multiplier"].asFloat()};
			mLevelSettings.addPattern([=]{ pm->barrageSpiral(times, delayMultiplier); }, chance);
		}
		else if(type == "mirror_spiral")
		{
			int times{mPatternRoot["times"].asInt()};			
			mLevelSettings.addPattern([=]{ pm->mirrorSpiral(times); }, chance);
		}
		else if(type == "extra_wall_vortex")
		{
			int times{mPatternRoot["times"].asInt()};
			int steps{mPatternRoot["steps"].asInt()};			
			mLevelSettings.addPattern([=]{ pm->extraWallVortex(times, steps); }, chance);
		}
		else if(type == "inverse_barrage")
		{
			int times{mPatternRoot["times"].asInt()};
			mLevelSettings.addPattern([=]{ pm->inverseBarrage(times); }, chance);
		}
		else if(type == "mirror_wall_strip")
		{
			int times{mPatternRoot["times"].asInt()};
			mLevelSettings.addPattern([=]{ pm->mirrorWallStrip(times); }, chance);
		}
		else if(type == "tunnel_barrage")
		{
			int times{mPatternRoot["times"].asInt()};
			mLevelSettings.addPattern([=]{ pm->tunnelBarrage(times); }, chance);
		}
	}

	MusicData loadMusicFromJson(Json::Value &mRoot)
	{
		string id				{ mRoot["id"].asString() };
		string fileName			{ mRoot["file_name"].asString() };
		string name			 	{ mRoot["name"].asString() };
		string album	 		{ mRoot["album"].asString() };
		string author 			{ mRoot["author"].asString() };

		auto result = MusicData{id, fileName, name, album, author};

		for (Json::Value segment : mRoot["segments"])		
			result.addSegment(segment["time"].asInt());

		return result;
	}
}
