// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_PROFILEDATA
#define HG_PROFILEDATA

#include <string>
#include <SSVJsonCpp/SSVJsonCpp.h>

namespace hg
{
	class ProfileData
	{
		private:
			float version;
			std::string name;
			Json::Value scores;
			std::vector<std::string> trackedNames;

		public:
			ProfileData(float mVersion, const std::string& mName, const Json::Value& mScores, const std::vector<std::string>& mTrackedNames);

			float getVersion() const;
			std::string getName() const;
			Json::Value getScores() const;
			const std::vector<std::string>& getTrackedNames() const;

			void setScore(const std::string& mId, float mScore);
			float getScore(const std::string& mId) const;

			void addTrackedName(const std::string& mTrackedName);
			void clearTrackedNames();
	};
}

#endif
