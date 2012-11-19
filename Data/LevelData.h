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
