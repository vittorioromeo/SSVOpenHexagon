#ifndef LEVELSETTINGS_H
#define LEVELSETTINGS_H

#include <vector>
#include <functional>
#include <string>

using namespace std;

namespace hg
{
	class LevelSettings
	{
		private:
			vector<function<void()>> pfuncs;
			int currentPattern	{-1};
			string name			{""};
			string description	{""};
			string author		{""};
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
			LevelSettings(string mName, string mDescription, string mAuthor, string mMusicId,
							float mSpeedStart, float mSpeedIncrement, float mRotationSpeed,
							float mRotationSpeedIncrement, float mDelayMultiplier, float mDelayIncrement,
							float mFastSpin, int mSidesStart, int mSidesMin,
							int mSidesMax, float mIncrementTime);
			
			void addPattern(function<void()> mPatternFunc, int mChance = 1);
			function<void()> getRandomPattern();

			string getName();
			string getDescription();
			string getAuthor();
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
