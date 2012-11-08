#include "LevelSettings.h"
#include "Utils.h"

namespace hg
{
	LevelSettings::LevelSettings(float mSpeedStart, float mSpeedIncrement, float mRotationSpeedStart,
		float mRotationSpeedIncrement, float mDelayMultiplier, float mFastSpin) :
		speed{mSpeedStart}, speedInc{mSpeedIncrement}, rotation{mRotationSpeedStart},
		rotationInc{mRotationSpeedIncrement}, delay{mDelayMultiplier}, fastSpin(mFastSpin) { }

	function<void()> LevelSettings::getRandomPattern() { return pfuncs[rnd(0, pfuncs.size())]; }
}
