#include "Data/ProfileData.h"

using namespace std;

namespace hg
{
	ProfileData::ProfileData(string mId, string mName, Json::Value mScores) : id{mId}, name{mName}, scores{mScores} { }

	string ProfileData::getId() 			{ return id; }
	string ProfileData::getName()			{ return name; }
	Json::Value ProfileData::getScores()	{ return scores; }

	void ProfileData::setScore(string mId, float mScore) 	{ scores[mId] = mScore; }
	float ProfileData::getScore(string mId)					{ return scores[mId].asFloat(); }
}

