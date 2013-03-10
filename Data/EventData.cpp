// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Data/EventData.h"
#include "HexagonGame.h"

using namespace std;

namespace hg
{
	EventData::EventData(Json::Value mRoot) : root{mRoot} { }

	string EventData::getId() { return root["id"].asString(); }
	bool EventData::getFinished() { return finished; }

	void EventData::setHexagonGamePtr(HexagonGame* mHgPtr) { hgPtr = mHgPtr; }

	void EventData::update(float mFrameTime)
	{
		currentTime += mFrameTime / 60.0f;
		hgPtr->executeEvents(root["events"], currentTime);
		for (Json::Value event : root["events"]) if(event["time"].asFloat() > currentTime) return;
		finished = true;
	}
}

