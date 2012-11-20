#include "Data/ScriptData.h"
#include "HexagonGame.h"
#include "Utils/Utils.h"

using namespace std;

namespace hg
{
	ScriptData::ScriptData(Json::Value mRoot) : root{mRoot}
	{
	}

	string ScriptData::getId() { return root["id"].asString(); }

	void ScriptData::setHexagonGamePtr(HexagonGame* mHgPtr) { hgPtr = mHgPtr; }

	void ScriptData::update(float mFrameTime)
	{
		currentTime += 1 * mFrameTime;
		hgPtr->executeEvents(root["events"], currentTime);
	}
}

