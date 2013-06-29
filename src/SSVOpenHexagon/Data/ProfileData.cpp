// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVUtils/SSVUtils.h>
#include "SSVOpenHexagon/Data/ProfileData.h"

using namespace std;
using namespace ssvu;

namespace hg
{
	ProfileData::ProfileData(float mVersion, const string& mName, const Json::Value& mScores, const vector<string>& mTrackedNames) : version{mVersion}, name{mName}, scores{mScores}, trackedNames{mTrackedNames} { }

	float ProfileData::getVersion() const							{ return version; }
	string ProfileData::getName() const								{ return toLower(name); }
	Json::Value ProfileData::getScores() const						{ return scores; }
	const vector<string>& ProfileData::getTrackedNames() const		{ return trackedNames; }

	void ProfileData::setScore(const string& mId, float mScore)		{ scores[mId] = mScore; }
	float ProfileData::getScore(const string& mId) const			{ return scores[mId].asFloat(); }
	void ProfileData::addTrackedName(const string& mTrackedName)	{ trackedNames.push_back(toLower(mTrackedName)); }
	void ProfileData::clearTrackedNames()							{ trackedNames.clear(); }
}

