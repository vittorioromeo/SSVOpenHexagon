// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace std;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{
	LevelData::LevelData(const ssvuj::Value& mRoot) : root{mRoot}
	{
		difficultyMultipliers.push_back(1.0f); sort(difficultyMultipliers);
	}

	void LevelData::loadTrackedVariables(const ssvuj::Value& mRoot)
	{
		for(const auto& t : as<ssvuj::Value>(mRoot, "tracked"))
		{
			const string& variableName{as<string>(t, 0)};
			const string& displayName{as<string>(t, 1)};
			bool hasOffset{t.size() == 3};

			if(hasOffset)
			{
				int offset{as<int>(t, 2)};
				trackedVariables.emplace_back(variableName, displayName, offset);
			}
			else trackedVariables.emplace_back(variableName, displayName);
		}
	}
}
