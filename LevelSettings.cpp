#include "LevelSettings.h"
#include "Utils.h"

namespace hg
{
	LevelSettings::LevelSettings(string mName, float mSpeedStart, float mSpeedIncrement, float mRotationSpeed,
		float mRotationSpeedIncrement, float mDelayMultiplier, float mFastSpin,
		int mSidesStart, int mSidesMin, int mSidesMax) :
		name{mName}, speedMultiplier{mSpeedStart}, speedIncrement{mSpeedIncrement}, rotationSpeed{mRotationSpeed},
		rotationSpeedIncrement{mRotationSpeedIncrement}, delayMultiplier{mDelayMultiplier}, fastSpin(mFastSpin),
		sidesStart{mSidesStart}, sidesMin{mSidesMin}, sidesMax{mSidesMax} { }

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

	string LevelSettings::getName() 					{ return name; }
	float LevelSettings::getSpeedMultiplier() 			{ return speedMultiplier; }
	float LevelSettings::getSpeedIncrement() 			{ return speedIncrement; }
	float LevelSettings::getRotationSpeed() 			{ return rotationSpeed; }
	float LevelSettings::getRotationSpeedIncrement() 	{ return rotationSpeedIncrement; }
	float LevelSettings::getDelayMultiplier() 			{ return delayMultiplier; }
	float LevelSettings::getFastSpin() 					{ return fastSpin; }
	int LevelSettings::getSidesStart() 					{ return sidesStart; }
	int LevelSettings::getSidesMax() 					{ return sidesMax; }
	int LevelSettings::getSidesMin() 					{ return sidesMin; }
}
