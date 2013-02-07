// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

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
			float version;
			string name;
			Json::Value scores;

		public:
			ProfileData(float mVersion, string mName, Json::Value mScores);

			float getVersion();
			string getName();
			Json::Value getScores();

			void setScore(string mId, float mScore);
			float getScore(string mId);
	};
}

#endif // PROFILEDATA_H
