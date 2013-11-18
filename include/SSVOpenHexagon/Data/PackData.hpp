// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_PACKDATA
#define HG_PACKDATA

namespace hg
{
	struct PackData
	{
		std::string id, name; float priority;
		PackData(const std::string& mId, const std::string& mName, float mPriority) : id{mId}, name{mName}, priority{mPriority} { }
	};
}

#endif
