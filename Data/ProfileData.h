// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef PROFILEDATA_H
#define PROFILEDATA_H

#include <string>
#include <json/json.h>

namespace hg
{
	class ProfileData
	{
		private:
			float version;
			std::string name;
			Json::Value scores;

		public:
			ProfileData(float mVersion, const std::string& mName, Json::Value mScores);

			float getVersion();
			std::string getName();
			Json::Value getScores();

			void setScore(const std::string& mId, float mScore);
			float getScore(const std::string& mId);
	};
}

#endif // PROFILEDATA_H
