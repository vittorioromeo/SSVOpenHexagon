// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Data/EventData.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"

using namespace std;
using namespace ssvuj;

namespace hg
{
	EventData::EventData(const ssvuj::Value& mRoot) : root{mRoot} { }

	void EventData::update(float mFrameTime)
	{
		currentTime += mFrameTime / 60.0f;
		hgPtr->executeEvents(root["events"], currentTime);
		for(ssvuj::Value event : root["events"]) if(as<float>(event, "time") > currentTime) return;
		finished = true;
	}
}

