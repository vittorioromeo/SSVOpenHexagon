#include "LevelData.h"
#include "Utils.h"

namespace hg
{
	LevelData::LevelData(string mName, string mDescription, string mAuthor, string mStyleId, string mMusicId,
								float mSpeedStart, float mSpeedIncrement, float mRotationSpeed,
								float mRotationSpeedIncrement, float mDelayMultiplier, float mDelayIncrement,
								float mFastSpin, int mSidesStart, int mSidesMin,
								int mSidesMax, float mIncrementTime) :
		name{mName}, description{mDescription}, author{mAuthor}, styleId{mStyleId}, musicId{mMusicId},
		speedMultiplier{mSpeedStart}, speedIncrement{mSpeedIncrement}, rotationSpeed{mRotationSpeed},
		rotationSpeedIncrement{mRotationSpeedIncrement}, delayMultiplier{mDelayMultiplier}, delayIncrement{mDelayIncrement},
		fastSpin(mFastSpin), sidesStart{mSidesStart}, sidesMin{mSidesMin},
		sidesMax{mSidesMax}, incrementTime{mIncrementTime} { }

	function<void(PatternManager* pm)> LevelData::getRandomPattern()
	{
		currentPattern++;

		if(currentPattern == (int)pfuncs.size()) currentPattern = 0;
		if(currentPattern == 0) random_shuffle(pfuncs.begin(), pfuncs.end());

		return pfuncs[currentPattern];
	}

	void LevelData::addPattern(function<void(PatternManager* pm)> mPatternFunc, int mChance)
	{
		for(int i{0}; i < mChance; i++) pfuncs.push_back(mPatternFunc);
	}

	string LevelData::getName() 					{ return name; }
	string LevelData::getDescription() 				{ return description; }
	string LevelData::getAuthor() 					{ return author; }
	string LevelData::getMusicId() 					{ return musicId; }
	string LevelData::getStyleId() 					{ return styleId; }
	float LevelData::getSpeedMultiplier() 			{ return speedMultiplier; }
	float LevelData::getSpeedIncrement() 			{ return speedIncrement; }
	float LevelData::getRotationSpeed() 			{ return rotationSpeed; }
	float LevelData::getRotationSpeedIncrement() 	{ return rotationSpeedIncrement; }
	float LevelData::getDelayMultiplier() 			{ return delayMultiplier; }
	float LevelData::getDelayIncrement() 			{ return delayIncrement; }
	float LevelData::getFastSpin() 					{ return fastSpin; }
	int LevelData::getSidesStart() 					{ return sidesStart; }
	int LevelData::getSidesMax() 					{ return sidesMax; }
	int LevelData::getSidesMin() 					{ return sidesMin; }
	float LevelData::getIncrementTime()				{ return incrementTime; }
}
