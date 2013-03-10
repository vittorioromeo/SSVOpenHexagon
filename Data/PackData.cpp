// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "PackData.h"

using namespace std;

namespace hg
{
	PackData::PackData(string mId, const string& mName, float mPriority) : id{mId}, name{mName}, priority{mPriority} { }

	string PackData::getId() 		{ return id; }
	string PackData::getName() 		{ return name; }
	float PackData::getPriority() 	{ return priority; }
}

