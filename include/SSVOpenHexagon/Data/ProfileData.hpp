// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_PROFILEDATA
#define HG_PROFILEDATA

#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{
class ProfileData
{
private:
    float version;
    std::string name;
    ssvuj::Obj scores;
    std::vector<std::string> trackedNames;

public:
    ProfileData(float mVersion, const std::string& mName,
    const ssvuj::Obj& mScores, const std::vector<std::string>& mTrackedNames)
        : version{mVersion}, name{mName}, scores{mScores},
          trackedNames{mTrackedNames}
    {
    }

    inline float getVersion() const { return version; }
    inline const std::string& getName() const { return name; }
    inline const ssvuj::Obj& getScores() const { return scores; }
    inline const std::vector<std::string>& getTrackedNames() const
    {
        return trackedNames;
    }

    inline void setScore(const std::string& mId, float mScore)
    {
        ssvuj::arch(scores, mId, mScore);
    }
    inline float getScore(const std::string& mId) const
    {
        return ssvuj::getExtr<float>(scores, mId);
    }

    inline void addTrackedName(const std::string& mTrackedName)
    {
        trackedNames.emplace_back(ssvu::toLower(mTrackedName));
    }
    inline void clearTrackedNames() { trackedNames.clear(); }
};
}

#endif
