#include "LevelSettings.h"
#include "Utils.h"

namespace hg
{
	LevelSettings::LevelSettings(string mName, float mSpeedStart, float mSpeedIncrement, float mRotationSpeedStart,
		float mRotationSpeedIncrement, float mDelayMultiplier, float mFastSpin) :
		name{mName}, speed{mSpeedStart}, speedInc{mSpeedIncrement}, rotation{mRotationSpeedStart},
		rotationInc{mRotationSpeedIncrement}, delay{mDelayMultiplier}, fastSpin(mFastSpin) { }

	function<void()> LevelSettings::getRandomPattern()
	{
		currentPattern++;

		if(currentPattern == (int)pfuncs.size()) currentPattern = 0;
		if(currentPattern == 0) random_shuffle(pfuncs.begin(), pfuncs.end());		

		return pfuncs[currentPattern];
	}

	void LevelSettings::addPattern(function<void()> mPatternFunc, int mChance)
	{
		for(int i{0}; i < mChance; i++) pfuncs.push_back(mPatternFunc);
	}
}
