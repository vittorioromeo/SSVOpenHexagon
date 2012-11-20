/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <json/json.h>
#include <json/reader.h>
#include <dirent.h>
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/ProfileData.h"
#include "Data/StyleData.h"
#include "Utils/Utils.h"

namespace hg
{
	int getRnd(int min, int max)
	{
	   double x = rand()/static_cast<double>(RAND_MAX);
	   int that = min + static_cast<int>( x * (max - min) );
	   return that;
	}
	float getSaturated(float x) { return std::max(0.0f, std::min(1.0f, x)); }
	float getSmootherStep(float edge0, float edge1, float x)
	{
		x = getSaturated((x - edge0)/(edge1 - edge0));
		return x*x*x*(x*(x*6 - 15) + 10);
	}
	bool isPointInPolygon(std::vector<Vector2f*> verts, Vector2f test)
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

	Vector2f getOrbit(const Vector2f& mParent, const float mDegrees, const float mRadius)
	{
		return Vector2f{ mParent.x + cos(toRadians(mDegrees)) * mRadius, mParent.y + sin(toRadians(mDegrees)) * mRadius };
	}
	Vector2f getNormalized(const Vector2f mVector)
	{
		float length { std::sqrt((mVector.x * mVector.x) + (mVector.y * mVector.y)) };
		return Vector2f{ mVector.x / length, mVector.y / length };
	}

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

	vector<string> getAllFilePaths(string mFolderPath, string mExtension)
	{
		vector<string> result;
		struct dirent *foundFile;
		DIR *directoryHandle;

		directoryHandle = opendir(mFolderPath.c_str());
		if (directoryHandle == NULL)
		{
			ostringstream fail;
			fail << "Error querying directory " << mFolderPath;
			log(fail.str());
			return result;
		}
		while ((foundFile = readdir(directoryHandle)))
		{
			const char *dotCheck = strrchr(foundFile->d_name, '.');
			if (dotCheck == NULL || dotCheck == foundFile->d_name) continue; // No extension?
			if (strcmp(mExtension.c_str(), dotCheck) != 0) continue; // Mismatch.
			ostringstream pass;
			pass << mFolderPath << foundFile->d_name;
			result.push_back(pass.str());
		}
		closedir(directoryHandle);
			
		return result;
	}
	string getFileNameFromFilePath(string mFilePath, string mPrefix, string mSuffix)
	{
		return mFilePath.substr(mPrefix.length(), mFilePath.length() - mPrefix.length() - mSuffix.length());
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
		LevelData result = LevelData{mRoot};

		for (Json::Value pattern : mRoot["patterns"]) parseAndAddPattern(result, pattern);
		for (Json::Value event : mRoot["events"]) parseAndAddEvent(result, event);

		return result;
	}
	MusicData loadMusicFromJson(Json::Value mRoot)
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
	StyleData loadStyleFromJson(Json::Value mRoot)
	{
		string id					{ mRoot["id"].asString() };
		float hueMin  				{ mRoot["hue_min"].asFloat() };
		float hueMax  				{ mRoot["hue_max"].asFloat() };
		bool huePingPong			{ mRoot["hue_ping_pong"].asBool() };
		float hueIncrement 			{ mRoot["hue_increment"].asFloat() };
		bool huePulse				{ mRoot["hue_pulse"].asBool() };
		bool mainDynamic			{ mRoot["main_dynamic"].asBool() };
		float mainDynamicDarkness	{ mRoot["main_dynamic_darkness"].asFloat() };
		Json::Value mainStatic		{ mRoot["main_static"] };
		bool aDynamic				{ mRoot["a_dynamic"].asBool() };
		float aDynamicDarkness		{ mRoot["a_dynamic_darkness"].asFloat() };
		Json::Value aStatic			{ mRoot["a_static"] };
		bool bDynamic				{ mRoot["b_dynamic"].asBool() };
		bool bDynamicOffset			{ mRoot["b_dynamic_offset"].asBool() };
		float bDynamicDarkness		{ mRoot["b_dynamic_darkness"].asFloat() };
		Json::Value bStatic			{ mRoot["b_static"] };

		Color mainStaticColor(mainStatic[0].asInt(), mainStatic[1].asInt(), mainStatic[2].asInt(), mainStatic[3].asInt());
		Color aStaticColor(aStatic[0].asInt(), aStatic[1].asInt(), aStatic[2].asInt(), aStatic[3].asInt());
		Color bStaticColor(bStatic[0].asInt(), bStatic[1].asInt(), bStatic[2].asInt(), bStatic[3].asInt());

		return StyleData(id, hueMin, hueMax, huePingPong, hueIncrement, huePulse, mainDynamic, mainDynamicDarkness, mainStaticColor,
						aDynamic, aDynamicDarkness, aStaticColor, bDynamic, bDynamicOffset, bDynamicDarkness, bStaticColor);
	}
	ProfileData loadProfileFromJson(string mId, Json::Value mRoot)
	{
		string name			{ mRoot["name"].asString() };
		Json::Value scores	{ mRoot["scores"] };

		auto result = ProfileData{mId, name, scores};
		return result;
	}

	void parseAndAddPattern(LevelData& mLevelData, Json::Value &mPatternRoot)
	{
		string type	{ mPatternRoot["type"].asString() };
		int chance	{ mPatternRoot["chance"].asInt() };
		float adjDelay { mPatternRoot["adj_delay"].asFloat() };
		float adjSpeed { mPatternRoot["adj_speed"].asFloat() };
		float adjThickness { mPatternRoot["adj_thickness"].asFloat() };

		function<void(PatternManager*)> func;

		if(type == "alternate_wall_barrage")
		{
			int times{mPatternRoot["times"].asInt()};
			int div{mPatternRoot["div"].asInt()};
			func = [=](PatternManager* pm){ pm->alternateWallBarrage(times, div); };
		}
		else if(type == "barrage_spiral")
		{
			int times{mPatternRoot["times"].asInt()};
			float delayMultiplier{mPatternRoot["delay_multiplier"].asFloat()};
			func = [=](PatternManager* pm){ pm->barrageSpiral(times, delayMultiplier); };
		}
		else if(type == "mirror_spiral")
		{
			int times{mPatternRoot["times"].asInt()};
			int extra{mPatternRoot["extra"].asInt()};
			func = [=](PatternManager* pm){ pm->mirrorSpiral(times, extra); };
		}
		else if(type == "extra_wall_vortex")
		{
			int times{mPatternRoot["times"].asInt()};
			int steps{mPatternRoot["steps"].asInt()};
			func = [=](PatternManager* pm){ pm->extraWallVortex(times, steps); };
		}
		else if(type == "inverse_barrage")
		{
			int times{mPatternRoot["times"].asInt()};
			func = [=](PatternManager* pm){ pm->inverseBarrage(times); };
		}
		else if(type == "mirror_wall_strip")
		{
			int times{mPatternRoot["times"].asInt()};
			int extra{mPatternRoot["extra"].asInt()};
			func = [=](PatternManager* pm){ pm->mirrorWallStrip(times, extra); };
		}
		else if(type == "tunnel_barrage")
		{
			int times{mPatternRoot["times"].asInt()};
			func = [=](PatternManager* pm){ pm->tunnelBarrage(times); };
		}

		mLevelData.addPattern(getAdjPatternFunc(func, adjDelay, adjSpeed, adjThickness), chance);
	}
	void parseAndAddEvent(LevelData& mLevelData, Json::Value &mEventRoot)
	{
		mLevelData.addEvent(mEventRoot);
	}

	function<void(PatternManager*)> getAdjPatternFunc(function<void(PatternManager*)> mFunction, float mAdjDelay, float mAdjSpeed, float mAdjThickness)
	{
		if(mAdjDelay == 0.0f) mAdjDelay = 1.0f;
		if(mAdjSpeed == 0.0f) mAdjSpeed = 1.0f;
		if(mAdjThickness == 0.0f) mAdjThickness = 1.0f;

		return [=](PatternManager* pm){ pm->setAdj(mAdjDelay, mAdjSpeed, mAdjThickness); mFunction(pm); pm->resetAdj(); };
	}

	void clearAndResetTimeline(Timeline& mTimeline)
	{
		mTimeline.clear();
		mTimeline.reset();
	}
}
