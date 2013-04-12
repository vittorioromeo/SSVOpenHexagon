// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_PACKDATA
#define HG_PACKDATA

#include <string>
#include "json/json.h"

namespace hg
{
	class PackData
	{
		private:
			std::string id, name;
			float priority;
			std::string hash;

		public:
			PackData(const std::string& mId, const std::string& mName, float mPriority, const std::string& mHash);
			std::string getId();
			std::string getName();
			std::string getHash();
			float getPriority();
	};
}

#endif
