#ifndef PACKDATA_H
#define PACKDATA_H

#include <string>
#include <vector>
#include <json/json.h>

using namespace std;

namespace hg
{
	class PackData
	{
		private:
			string id;
			string name;
			float priority;

		public:
			PackData(string mId, string mName, float mPriority);
			string getId();
			string getName();
			float getPriority();
	};
}

#endif // PACKDATA_H
