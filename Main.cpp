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

#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include <SSVEntitySystem.h>
#include <SSVStart.h>
#include "Components/CPlayer.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "MenuGame.h"

using namespace std;
using namespace sses;
using namespace ssvs;
using namespace sf;
using namespace hg;

int main(int argc, char* argv[])
{
	//for(string x : getAllSubFolderNames(".")) cout << x << endl;

	vector<string> overrideIds;
	for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});

	loadConfig(overrideIds);
	loadAssets();

	srand(unsigned(time(NULL)));

	string title{"Open Hexagon " + toStr<float>(getVersion()) + " - vee software"};
	
	GameWindow window{title, getWidth(), getHeight(), getPixelMultiplier(), getLimitFps(), getFullscreen()};
	window.setStaticFrameTime(getStaticFrameTime());
	window.setStaticFrameTimeValue(getStaticFrameTimeValue());
	window.setVsync(getVsync());
	window.setMouseCursorVisible(false);

	MenuGame mg{window};
	HexagonGame hg{window};

	mg.hgPtr = &hg;
	hg.mgPtr = &mg;

	window.setGame(&mg.getGame());
	mg.init();

	window.run();

	saveCurrentProfile();

	ofstream o;
	o.open("log.txt");
	for(string logEntry : getLogEntries()) 	o << logEntry << endl;
	o.flush();
	o.close();

	return 0;
}
