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
#include "Global/Config.h"

using namespace std;
using namespace sf;

namespace hg
{
	template<class T> void log(T mValue) { if(!getDebug()) return; cout << toStr(mValue) << endl; }

	void read(char* path);

	Color getColorFromHue(double);
	Color getColorDarkened(Color, float);
	Color getColorFromJsonArray(Json::Value mArray);

	vector<string> getAllFilePaths(string mFolderPath, string mExtension);
	string getFileNameFromFilePath(string mFilePath, string mPrefix, string mSuffix);
	Json::Value getJsonFileRoot(string mFilePath);

	LevelData loadLevelFromJson(Json::Value mRoot);
	MusicData loadMusicFromJson(Json::Value mRoot);
	StyleData loadStyleFromJson(Json::Value mRoot);
	ProfileData loadProfileFromJson(Json::Value mRoot);
}

#endif /* UTILS_H_HG */
