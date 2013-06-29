// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_PACKDATA
#define HG_PACKDATA

#include <string>

namespace hg
{
	class PackData
	{
		private:
			std::string id, name, hash;
			float priority;

		public:
			PackData(const std::string& mId, const std::string& mName, float mPriority, const std::string& mHash);
			std::string getId() const;
			std::string getName() const;
			std::string getHash() const;
			float getPriority() const;
	};
}

#endif
