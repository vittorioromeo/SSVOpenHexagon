// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg
{

struct TrackedVariable
{
    std::string variableName, displayName;
    TrackedVariable(std::string mVariableName, std::string mDisplayName)
        : variableName{ssvu::mv(mVariableName)}, displayName{
                                                     ssvu::mv(mDisplayName)}
    {
    }
};

} // namespace hg
