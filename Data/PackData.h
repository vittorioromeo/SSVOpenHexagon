#ifndef PACKDATA_H
#define PACKDATA_H

#include <string>
#include <json/json.h>

namespace hg
{
	class PackData
	{
		private:
			std::string id, name;
			float priority;

		public:
			PackData(std::string mId, const std::string& mName, float mPriority);
			std::string getId();
			std::string getName();
			float getPriority();
	};
}

#endif // PACKDATA_H
