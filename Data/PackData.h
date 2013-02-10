#ifndef PACKDATA_H
#define PACKDATA_H

#include <string>
#include <json/json.h>

namespace hg
{
	class PackData
	{
		private:
			std::string id;
			std::string name;
			float priority;

		public:
			PackData(std::string mId, std::string mName, float mPriority);
			std::string getId();
			std::string getName();
			float getPriority();
	};
}

#endif // PACKDATA_H
