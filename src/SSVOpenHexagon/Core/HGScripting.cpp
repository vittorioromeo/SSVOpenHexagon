// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <fstream>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Components/CWall.h"

using namespace std;
using namespace sf;
using namespace sses;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{
	void HexagonGame::initLua()
	{
		lua.writeVariable("log", 					[=](string mLog) 						{ log(mLog, "Lua log"); });

		lua.writeVariable("wall", 					[=](int mSide, float mThickness) 		{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, getSpeedMultiplier()); }); });
		lua.writeVariable("getSides", 				[=]() 									{ return levelData.getSides(); });
		lua.writeVariable("getSpeedMult",			[=]() 									{ return getSpeedMultiplier(); });
		lua.writeVariable("getDelayMult", 			[=]() 									{ return getDelayMultiplier(); });
		lua.writeVariable("getDifficultyMult",		[=]() 									{ return difficultyMult; });
		lua.writeVariable("execScript", 			[=](string mName) 						{ runLuaFile(mName); });
		lua.writeVariable("wait", 					[=](float mDuration) 					{ timeline.append<Wait>(mDuration); });

		lua.writeVariable("playSound", 				[=](string mId) 						{ playSound(mId); });
		lua.writeVariable("forceIncrement", 		[=]()			 						{ incrementDifficulty(); });

		lua.writeVariable("eventStopTime",			[=](float mDuration) 					{ eventTimeline.append<Do>([=]{ status.timeStop = mDuration; }); });
		lua.writeVariable("eventStopTimeS",			[=](float mDuration) 					{ eventTimeline.append<Do>([=]{ status.timeStop = mDuration * 60.f; }); });
		lua.writeVariable("eventWait",				[=](float mDuration) 					{ eventTimeline.append<Wait>(mDuration); });
		lua.writeVariable("eventWaitS", 			[=](float mDuration) 					{ eventTimeline.append<Wait>(mDuration * 60.f); });
		lua.writeVariable("eventWaitUntilS", 		[=](float mDuration)
		{
			eventTimeline.append<Wait>(10);
			eventTimeline.append<Do>([=]{ if(status.currentTime < mDuration) eventTimeline.jumpTo(eventTimeline.getCurrentIndex() - 2); });
		});
		lua.writeVariable("messageAdd", 			[=](string mMessage, float mDuration)	{ eventTimeline.append<Do>([=]{ if(firstPlay && getShowMessages()) addMessage(mMessage, mDuration); }); });
		lua.writeVariable("messageImportantAdd",	[=](string mMessage, float mDuration)	{ eventTimeline.append<Do>([=]{ if(getShowMessages()) addMessage(mMessage, mDuration); }); });

		lua.writeVariable("getLevelValueInt", 		[=](string mValueName) 					{ return levelData.getValueInt(mValueName); });
		lua.writeVariable("getLevelValueFloat", 	[=](string mValueName) 					{ return levelData.getValueFloat(mValueName); });
		lua.writeVariable("getLevelValueString", 	[=](string mValueName) 					{ return levelData.getValueString(mValueName); });
		lua.writeVariable("getLevelValueBool", 		[=](string mValueName) 					{ return levelData.getValueBool(mValueName); });
		lua.writeVariable("setLevelValueInt", 		[=](string mValueName, int mValue) 		{ return levelData.setValueInt(mValueName, mValue); });
		lua.writeVariable("setLevelValueFloat", 	[=](string mValueName, float mValue) 	{ return levelData.setValueFloat(mValueName, mValue); });
		lua.writeVariable("setLevelValueString", 	[=](string mValueName, string mValue) 	{ return levelData.setValueString(mValueName, mValue); });
		lua.writeVariable("setLevelValueBool", 		[=](string mValueName, bool mValue) 	{ return levelData.setValueBool(mValueName, mValue); });

		lua.writeVariable("getStyleValueInt", 		[=](string mValueName) 					{ return styleData.getValueInt(mValueName); });
		lua.writeVariable("getStyleValueFloat", 	[=](string mValueName) 					{ return styleData.getValueFloat(mValueName); });
		lua.writeVariable("getStyleValueString", 	[=](string mValueName) 					{ return styleData.getValueString(mValueName); });
		lua.writeVariable("getStyleValueBool", 		[=](string mValueName) 					{ return styleData.getValueBool(mValueName); });
		lua.writeVariable("setStyleValueInt", 		[=](string mValueName, int mValue) 		{ return styleData.setValueInt(mValueName, mValue); });
		lua.writeVariable("setStyleValueFloat", 	[=](string mValueName, float mValue) 	{ return styleData.setValueFloat(mValueName, mValue); });
		lua.writeVariable("setStyleValueString", 	[=](string mValueName, string mValue) 	{ return styleData.setValueString(mValueName, mValue); });
		lua.writeVariable("setStyleValueBool", 		[=](string mValueName, bool mValue) 	{ return styleData.setValueBool(mValueName, mValue); });

		lua.writeVariable("isKeyPressed",			[=](int mKey) 							{ return window.isKeyPressed((Keyboard::Key) mKey); });
		lua.writeVariable("isFastSpinning",			[=]() 									{ return status.fastSpin > 0; });

		lua.writeVariable("wallAdj", [=](int mSide, float mThickness, float mSpeedAdj)
		{
			timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, mSpeedAdj * getSpeedMultiplier()); });
		});
		lua.writeVariable("wallAcc", [=](int mSide, float mThickness, float mSpeedAdj, float mAcceleration, float mMinSpeed, float mMaxSpeed)
		{
			timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, mSpeedAdj * getSpeedMultiplier(), mAcceleration, mMinSpeed * getSpeedMultiplier(), mMaxSpeed * getSpeedMultiplier()); });
		});
		lua.writeVariable("wallHModSpeedData", [=](float mHueModifier, int mSide, float mThickness, float mSpeedAdj, float mSpeedAccel, float mSpeedMin, float mSpeedMax, bool mSpeedPingPong)
		{
			timeline.append<Do>([=]
			{
				factory.createWallHModData(mHueModifier, mSide, mThickness, {mSpeedAdj * getSpeedMultiplier(), mSpeedAccel, mSpeedMin, mSpeedMax, mSpeedPingPong}, {});
			});
		});
		lua.writeVariable("wallHModCurveData", [=](float mHueModifier, int mSide, float mThickness, float mCurveAdj, float mCurveAccel, float mCurveMin, float mCurveMax, bool mCurvePingPong)
		{
			timeline.append<Do>([=]
			{
				factory.createWallHModData(mHueModifier, mSide, mThickness, {getSpeedMultiplier(), 0, 0, 0, false}, {mCurveAdj, mCurveAccel, mCurveMin, mCurveMax, mCurvePingPong});
			});
		});

		lua.writeVariable("tutorialMode", [=]{ status.tutorialMode = true; });
		lua.writeVariable("stopIncrement", [=]{ status.incrementEnabled = false; });
		lua.writeVariable("startIncrement", [=]{ status.incrementEnabled = true; });

		lua.writeVariable("disableSwap", [=]{ status.swapEnabled = false; });
		lua.writeVariable("enableSwap", [=]{ status.swapEnabled = true; });

		lua.writeVariable("disableRandomSideChanges", [=]{ status.randomSideChangesEnabled = false; });
		lua.writeVariable("enableRandomSideChanges", [=]{ status.randomSideChangesEnabled = true; });

		lua.writeVariable("kill", [=]{ timeline.append<Do>([=]{ death(true); }); });
		lua.writeVariable("eventKill", [=]{ eventTimeline.append<Do>([=]{ death(true); }); });
	}
	void HexagonGame::runLuaFile(const string& mFileName)
	{
		ifstream s(levelData.getPackPath() + "Scripts/" + mFileName);
		try { lua.executeCode(s); }
		catch(runtime_error &error)
		{
			cout << mFileName << endl << "LUA execution error: " << endl << "(killing the player...)" << endl << toStr(error.what()) << endl << endl;
			death();
		}
	}
}
