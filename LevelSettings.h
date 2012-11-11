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
			int currentPattern{-1};

		public:
			string name;
			float speed;
			float speedInc;
			float rotation;
			float rotationInc;
			float delay;
			float fastSpin;
			
			LevelSettings(string, float, float, float, float, float, float);

			function<void()> getRandomPattern();
			void addPattern(function<void()> mPatternFunc, int mChance = 1);
	};
}
#endif // LEVELSETTINGS_H
