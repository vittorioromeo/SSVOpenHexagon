// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

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
        const ssvuj::Obj& mScores,
        const std::vector<std::string>& mTrackedNames)
        : version{mVersion}, name{mName}, scores{mScores}, trackedNames{
                                                               mTrackedNames}
    {
    }

    float getVersion() const
    {
        return version;
    }
    const std::string& getName() const
    {
        return name;
    }
    const ssvuj::Obj& getScores() const
    {
        return scores;
    }
    const std::vector<std::string>& getTrackedNames() const
    {
        return trackedNames;
    }

    void setScore(const std::string& mId, float mScore)
    {
        ssvuj::arch(scores, mId, mScore);
    }
    float getScore(const std::string& mId) const
    {
        return ssvuj::getExtr<float>(scores, mId);
    }

    void addTrackedName(const std::string& mTrackedName)
    {
        trackedNames.emplace_back(ssvu::toLower(mTrackedName));
    }
    void clearTrackedNames()
    {
        trackedNames.clear();
    }
};

} // namespace hg
