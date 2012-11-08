#ifndef LEVELSETTINGS_H
#define LEVELSETTINGS_H

#include <vector>
#include <functional>

using namespace std;

namespace hg
{
	class LevelSettings
	{
		public:
			float speed;
			float speedInc;
			float rotation;
			float rotationInc;
			float delay;
			float fastSpin;
			vector<function<void()>> pfuncs;

			LevelSettings(float, float, float, float, float, float);

			function<void()> getRandomPattern();
	};
}
#endif // LEVELSETTINGS_H
