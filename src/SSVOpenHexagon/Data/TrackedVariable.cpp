// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/TrackedVariable.h"

using namespace std;

namespace hg
{
	TrackedVariable::TrackedVariable(const string& mVariableName, const string& mDisplayName) : variableName{mVariableName}, displayName{mDisplayName}, hasOffset{false} { }
	TrackedVariable::TrackedVariable(const string& mVariableName, const string& mDisplayName, int mOffset) : variableName{mVariableName}, displayName{mDisplayName}, hasOffset{true}, offset{mOffset} { }
}
