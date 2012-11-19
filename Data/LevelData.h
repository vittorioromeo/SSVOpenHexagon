#ifndef LEVELSETTINGS_H
#define LEVELSETTINGS_H

#include <vector>
#include <functional>
#include <string>

using namespace std;

namespace hg
{
	class PatternManager;

	class LevelData
	{
		private:
			vector<function<void(PatternManager* pm)>> pfuncs;
			int currentPattern	{-1};
			string id			{""};
			string name			{""};
			string description	{""};
			string author		{""};
			int menuPriority    {0};
			string styleId		{""};
			string musicId		{""};
			float speedMultiplier;
			float speedIncrement;
			float rotationSpeed;
			float rotationSpeedIncrement;
			float delayMultiplier;
			float delayIncrement;
			float fastSpin;
			int sidesStart;
			int sidesMin;
			int sidesMax;
			float incrementTime;

		public:
			LevelData() = default;
			LevelData(string mId, string mName, string mDescription, string mAuthor, int mMenuPriority, string mStyleId, string mMusicId,
							float mSpeedStart, float mSpeedIncrement, float mRotationSpeed,
							float mRotationSpeedIncrement, float mDelayMultiplier, float mDelayIncrement,
							float mFastSpin, int mSidesStart, int mSidesMin,
							int mSidesMax, float mIncrementTime);
			
			void addPattern(function<void(PatternManager* pm)> mPatternFunc, int mChance = 1);
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
			int getSidesStart();
			int getSidesMax();
			int getSidesMin();
			float getIncrementTime();
	};
}
#endif // LEVELSETTINGS_H
