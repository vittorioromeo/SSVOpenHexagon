// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <utility>

namespace hg
{

struct TrackedVariable
{
    std::string variableName, displayName;
    TrackedVariable(std::string mVariableName, std::string mDisplayName)
        : variableName{std::move(mVariableName)}, displayName{
                                                      std::move(mDisplayName)}
    {
    }
};

} // namespace hg
