// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

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
			string packPath;

		public:
			LevelData() = default;
			LevelData(Json::Value mRoot);
			
			void addEvent(Json::Value mEventRoot);

			Json::Value& getRoot();

			void setPackPath(string mPackPath);
			string getPackPath();

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
			float getPulseMin();
			float getPulseMax();
			float getPulseSpeed();
			float getPulseSpeedR();
			float getPulseDelayMax();
			float getPulseDelayHalfMax();
			float getBeatPulseMax();
			float getBeatPulseDelayMax();
			float getRadiusMin();
			vector<float> getDifficultyMultipliers();
			vector<Json::Value>& getEvents();

			void setSpeedMultiplier(float mSpeedMultiplier);
			void setDelayMultiplier(float mDelayMultiplier);
			void setRotationSpeed(float mRotationSpeed);

			void setValueFloat(string mValueName, float mValue);
			float getValueFloat(string mValueName);

			void setValueInt(string mValueName, int mValue);
			float getValueInt(string mValueName);

			void setValueString(string mValueName, string mValue);
			string getValueString(string mValueName);

			void setValueBool(string mValueName, bool mValue);
			bool getValueBool(string mValueName);

	};
}
#endif // LEVELSETTINGS_H
