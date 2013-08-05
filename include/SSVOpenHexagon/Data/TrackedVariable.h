// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_TRACKEDVARIABLE
#define HG_TRACKEDVARIABLE

namespace hg
{
	struct TrackedVariable
	{
		std::string variableName, displayName;
		TrackedVariable(const std::string& mVariableName, const std::string& mDisplayName) : variableName{mVariableName}, displayName{mDisplayName} { }
	};
}

#endif
