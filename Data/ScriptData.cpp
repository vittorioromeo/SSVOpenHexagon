#include "Data/ScriptData.h"
#include "HexagonGame.h"
#include "Utils/Utils.h"

using namespace std;

namespace hg
{
	ScriptData::ScriptData(Json::Value mRoot) : root{mRoot} { }

	string ScriptData::getId() { return root["id"].asString(); }
	bool ScriptData::getFinished() { return finished; }

	void ScriptData::setHexagonGamePtr(HexagonGame* mHgPtr) { hgPtr = mHgPtr; }

	void ScriptData::update(float mFrameTime)
	{
		currentTime += mFrameTime / 60.0f;
		hgPtr->executeEvents(root["events"], currentTime);

		for (Json::Value event : root["events"])
			if(event["time"].asFloat() > currentTime) return;
			
		finished = true;
	}
}

