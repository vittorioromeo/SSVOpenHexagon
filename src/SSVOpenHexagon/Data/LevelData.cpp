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
}
