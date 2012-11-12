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
			float speedMultiplier;
			float speedIncrement;
			float rotationSpeed;
			float rotationSpeedIncrement;
			float delayMultiplier;
			float fastSpin;
			int sidesStart;
			int sidesMin;
			int sidesMax;

		public:					
			LevelSettings(string, float, float, float, float, float, float, int, int, int);
			
			void addPattern(function<void()> mPatternFunc, int mChance = 1);
			function<void()> getRandomPattern();

			string getName();
			float getSpeedMultiplier();
			float getSpeedIncrement();
			float getRotationSpeed();
			float getRotationSpeedIncrement();
			float getDelayMultiplier();
			float getFastSpin();
			int getSidesStart();
			int getSidesMax();
			int getSidesMin();
	};
}
#endif // LEVELSETTINGS_H
