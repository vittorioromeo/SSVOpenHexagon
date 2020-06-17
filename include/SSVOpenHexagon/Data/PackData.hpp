// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg
{


struct PackData
{
    std::string id, name;
    float priority;
    PackData(const std::string& mId, const std::string& mName, float mPriority)
        : id{mId}, name{mName}, priority{mPriority}
    {
    }
};

} // namespace hg
