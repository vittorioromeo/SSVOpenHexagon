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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <iostream>
#include <string>
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include <SSVStart.h>

namespace hg
{
	void loadConfig(std::vector<std::string> mOverridesIds);

	void recalculateSizes();
	void setFullscreen(ssvs::GameWindow& mWindow, bool mFullscreen);

	void setPulse(bool mPulse);

	float getSizeX();
	float getSizeY();
	float getSpawnDistance();
	float getZoomFactor();
	int getPixelMultiplier();
	float getPlayerSpeed();
	float getPlayerFocusSpeed();
	float getPlayerSize();
	bool getNoRotation();
	bool getNoBackground();
	bool getBlackAndWhite();
	bool getNoSound();
	bool getNoMusic();
	int getSoundVolume();
	int getMusicVolume();
	bool getStaticFrameTime();
	float getStaticFrameTimeValue();
	bool getLimitFps();
	bool getVsync();
	bool getAutoZoomFactor();
	bool getFullscreen();
	float getVersion();
	bool getWindowedAutoResolution();
	bool getFullscreenAutoResolution();
	unsigned int getFullscreenWidth();
	unsigned int getFullscreenHeight();
	unsigned int getWindowedWidth();
	unsigned int getWindowedHeight();
	unsigned int getWidth();
	unsigned int getHeight();
	bool getShowMessages();
	bool getChangeStyles();
	bool getChangeMusic();
	bool getDebug();
	bool getPulse();
	bool getBeatPulse();
}

#endif /* CONFIG_H_ */
