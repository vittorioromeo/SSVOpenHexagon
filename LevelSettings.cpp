#include "LevelSettings.h"
#include "Utils.h"

namespace hg
{
	LevelSettings::LevelSettings(string mName, string mDescription, string mAuthor, string mMusicId,
								float mSpeedStart, float mSpeedIncrement, float mRotationSpeed,
								float mRotationSpeedIncrement, float mDelayMultiplier, float mDelayIncrement,
								float mFastSpin, int mSidesStart, int mSidesMin,
								int mSidesMax, float mIncrementTime) :
		name{mName}, description{mDescription}, author{mAuthor}, musicId{mMusicId},
		speedMultiplier{mSpeedStart}, speedIncrement{mSpeedIncrement}, rotationSpeed{mRotationSpeed},
		rotationSpeedIncrement{mRotationSpeedIncrement}, delayMultiplier{mDelayMultiplier}, delayIncrement{mDelayIncrement},
		fastSpin(mFastSpin), sidesStart{mSidesStart}, sidesMin{mSidesMin},
		sidesMax{mSidesMax}, incrementTime{mIncrementTime} { }

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
	string LevelSettings::getDescription() 				{ return description; }
	string LevelSettings::getAuthor() 					{ return author; }
	string LevelSettings::getMusicId() 					{ return musicId; }
	float LevelSettings::getSpeedMultiplier() 			{ return speedMultiplier; }
	float LevelSettings::getSpeedIncrement() 			{ return speedIncrement; }
	float LevelSettings::getRotationSpeed() 			{ return rotationSpeed; }
	float LevelSettings::getRotationSpeedIncrement() 	{ return rotationSpeedIncrement; }
	float LevelSettings::getDelayMultiplier() 			{ return delayMultiplier; }
	float LevelSettings::getDelayIncrement() 			{ return delayIncrement; }
	float LevelSettings::getFastSpin() 					{ return fastSpin; }
	int LevelSettings::getSidesStart() 					{ return sidesStart; }
	int LevelSettings::getSidesMax() 					{ return sidesMax; }
	int LevelSettings::getSidesMin() 					{ return sidesMin; }
	float LevelSettings::getIncrementTime()				{ return incrementTime; }
}
