#ifndef PROFILEDATA_H
#define PROFILEDATA_H

#include <string>
#include <vector>
#include <json/json.h>

using namespace std;

namespace hg
{
	class ProfileData
	{
		private:
			string id;
			string name;
			Json::Value scores;

		public:
			ProfileData(string mId, string mName, Json::Value mScores);

			string getId();
			string getName();
			Json::Value getScores();

			void setScore(string mId, float mScore);
			float getScore(string mId);
	};
}

#endif // PROFILEDATA_H
