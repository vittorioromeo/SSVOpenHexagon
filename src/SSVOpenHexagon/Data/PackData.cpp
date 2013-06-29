// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/PackData.h"

using namespace std;

namespace hg
{
	PackData::PackData(const string& mId, const string& mName, float mPriority, const string& mHash) : id{mId}, name{mName}, hash{mHash}, priority{mPriority} { }

	string PackData::getId() const		{ return id; }
	string PackData::getName() const	{ return name; }
	string PackData::getHash() const	{ return hash; }
	float PackData::getPriority() const	{ return priority; }
}

