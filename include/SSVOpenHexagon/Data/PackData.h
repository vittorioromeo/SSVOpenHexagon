// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_PACKDATA
#define HG_PACKDATA

namespace hg
{
	struct PackData
	{
		std::string id, name, hash; float priority;
		PackData(const std::string& mId, const std::string& mName, float mPriority, const std::string& mHash) : id{mId}, name{mName}, hash{mHash}, priority{mPriority} { }
	};
}

#endif
