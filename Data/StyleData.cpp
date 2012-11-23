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

#include "Data/StyleData.h"
#include "Utils/Utils.h"

namespace hg
{	
	Color StyleData::calculateColor(Json::Value mColorRoot)
	{
		Color color{getColorFromJsonArray(mColorRoot["value"])};

		if(mColorRoot["dynamic"].asBool())
		{
			Color dynamicColor{getColorFromHue((currentHue + mColorRoot["hue_shift"].asFloat()) / 360.0f)};

			if(mColorRoot["main"].asBool()) color = dynamicColor;
			else
			{
				if(!mColorRoot["dynamic_offset"].asBool()) color = getColorDarkened(dynamicColor, mColorRoot["dynamic_darkness"].asFloat());
				else
				{
					color.r += dynamicColor.r / mColorRoot["offset"].asFloat();
					color.g += dynamicColor.g / mColorRoot["offset"].asFloat();
					color.b += dynamicColor.b / mColorRoot["offset"].asFloat();
					color.a += dynamicColor.a;
				}
			}
		}

		Color pulse{getColorFromJsonArray(mColorRoot["pulse"])};
		return Color(clamp(color.r + pulse.r * pulseFactor, 0.f, 255.f),
					 clamp(color.g + pulse.g * pulseFactor, 0.f, 255.f),
					 clamp(color.b + pulse.b * pulseFactor, 0.f, 255.f),
					 clamp(color.a + pulse.a * pulseFactor, 0.f, 255.f));
	}

	StyleData::StyleData(Json::Value mRoot) : root{mRoot}
	{
		currentHue = getHueMin();
		currentSwapTime = 0;
	}

	void StyleData::update(float mFrameTime)
	{
		currentSwapTime += 1.f * mFrameTime;
		if(currentSwapTime > 100) currentSwapTime = 0;

		currentHue += getHueIncrement() * mFrameTime;
				
		if(currentHue < getHueMin())
		{
			if(getHuePingPong())
			{
				currentHue = getHueMin();
				root["hue_increment"] = getHueIncrement() * -1;
			}
			else currentHue = getHueMax();
		}
		if(currentHue > getHueMax())
		{
			if(getHuePingPong())
			{
				currentHue = getHueMax();
				root["hue_increment"] = getHueIncrement() * -1;
			}
			else currentHue = getHueMin();
		}

		pulseFactor += root["pulse_increment"].asFloat() * mFrameTime;

		if(pulseFactor < root["pulse_min"].asFloat())
		{
			root["pulse_increment"] = root["pulse_increment"].asFloat() * -1;
			pulseFactor = root["pulse_min"].asFloat();
		}
		if(pulseFactor > root["pulse_max"].asFloat())
		{
			root["pulse_increment"] = root["pulse_increment"].asFloat() * -1;
			pulseFactor = root["pulse_max"].asFloat();
		}
	}

	string StyleData::getId() 					{ return root["id"].asString(); }
	float StyleData::getHueMin() 				{ return root["hue_min"].asFloat(); }
	float StyleData::getHueMax()				{ return root["hue_max"].asFloat(); }
	bool StyleData::getHuePingPong()			{ return root["hue_ping_pong"].asBool(); }
	float StyleData::getHueIncrement()			{ return root["hue_increment"].asFloat(); }

	float StyleData::getCurrentHue() 			{ return currentHue; }
	float StyleData::getCurrentSwapTime() 		{ return currentSwapTime; }
	Color StyleData::getMainColor()				{ return calculateColor(root["main"]); }
	vector<Color> StyleData::getColors()
	{
		vector<Color> result;

		for(unsigned int i{0}; i < root["colors"].size(); i++) result.push_back(calculateColor(root["colors"][i]));
		std::rotate(result.begin(), result.begin() + currentSwapTime / 50, result.end());

		return result;
	}

	void StyleData::setValueFloat(string mValueName, float mValue)	{ root[mValueName] = mValue; }
	float StyleData::getValueFloat(string mValueName)				{ return root[mValueName].asFloat(); }

	void StyleData::setValueInt(string mValueName, int mValue)		{ root[mValueName] = mValue; }
	float StyleData::getValueInt(string mValueName)					{ return root[mValueName].asInt(); }

	void StyleData::setValueString(string mValueName, string mValue){ root[mValueName] = mValue; }
	string StyleData::getValueString(string mValueName)				{ return root[mValueName].asString(); }

	void StyleData::setValueBool(string mValueName, bool mValue)	{ root[mValueName] = mValue; }
	bool StyleData::getValueBool(string mValueName)					{ return root[mValueName].asBool(); }
}

