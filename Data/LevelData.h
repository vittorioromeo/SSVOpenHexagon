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

#ifndef LEVELSETTINGS_H
#define LEVELSETTINGS_H

#include <vector>
#include <functional>
#include <string>
#include <map>
#include <json/json.h>

using namespace std;

namespace hg
{
	class PatternManager;

	class LevelData
	{
		private:
			Json::Value root;
			vector<function<void(PatternManager* pm)>> pfuncs;
			int currentPattern	{-1};
			vector<Json::Value> events;

		public:
			LevelData() = default;
			LevelData(Json::Value mRoot);
			
			void addPattern(function<void(PatternManager* pm)> mPatternFunc, int mChance = 1);
			void addEvent(Json::Value mEventRoot);

			function<void(PatternManager* pm)> getRandomPattern();

			string getId();
			string getName();
			string getDescription();
			string getAuthor();
			int getMenuPriority();
			bool getSelectable();
			string getStyleId();
			string getMusicId();
			float getSpeedMultiplier();
			float getSpeedIncrement();
			float getRotationSpeed();
			float getRotationSpeedIncrement();
			float getDelayMultiplier();
			float getDelayIncrement();
			float getFastSpin();
			int getSides();
			int getSidesMax();
			int getSidesMin();
			float getIncrementTime();
			vector<Json::Value>& getEvents();

			void setSpeedMultiplier(float mSpeedMultiplier);
			void setDelayMultiplier(float mDelayMultiplier);
			void setRotationSpeed(float mRotationSpeed);

			void setValueFloat(string mValueName, float mValue);
			float getValueFloat(string mValueName);

			void setValueInt(string mValueName, int mValue);
			float getValueInt(string mValueName);
	};
}
#endif // LEVELSETTINGS_H
