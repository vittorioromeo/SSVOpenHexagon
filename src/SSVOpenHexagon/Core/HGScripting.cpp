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
		// Utils
		lua.writeVariable("u_log", 					[=](string mLog) 						{ log(mLog, "Lua log"); });
		lua.writeVariable("u_execScript", 			[=](string mName) 						{ runLuaFile(levelData.packPath + "Scripts/" + mName); });
		lua.writeVariable("u_playSound", 			[=](string mId) 						{ playSound(mId); });
		lua.writeVariable("u_isKeyPressed",			[=](int mKey) 							{ return window.isKeyPressed((Keyboard::Key) mKey); });
		lua.writeVariable("u_isFastSpinning",		[=] 									{ return status.fastSpin > 0; });
		lua.writeVariable("u_forceIncrement", 		[=]				 						{ incrementDifficulty(); });
		lua.writeVariable("u_kill",					[=]										{ timeline.append<Do>([=]{ death(true); }); });
		lua.writeVariable("u_eventKill",			[=]										{ eventTimeline.append<Do>([=]{ death(true); }); });
		lua.writeVariable("u_getDifficultyMult",	[=] 									{ return difficultyMult; });
		lua.writeVariable("u_getSpeedMultDM",		[=]										{ return getSpeedMultDM(); });
		lua.writeVariable("u_getDelayMultDM",		[=]										{ return getDelayMultDM(); });

		// Messages
		lua.writeVariable("m_messageAdd", 			[=](string mMsg, float mDuration)		{ eventTimeline.append<Do>([=]{ if(firstPlay && getShowMessages()) addMessage(mMsg, mDuration); }); });
		lua.writeVariable("m_messageAddImportant",	[=](string mMsg, float mDuration)		{ eventTimeline.append<Do>([=]{ if(getShowMessages()) addMessage(mMsg, mDuration); }); });

		// Main timeline control
		lua.writeVariable("t_wait", 				[=](float mDuration) 					{ timeline.append<Wait>(mDuration); });
		lua.writeVariable("t_waitS", 				[=](float mDuration) 					{ timeline.append<Wait>(mDuration * 60.f); });
		lua.writeVariable("t_waitUntilS", 			[=](float mDuration)
		{
			timeline.append<Wait>(10);
			timeline.append<Do>([=]{ if(status.currentTime < mDuration) timeline.jumpTo(timeline.getCurrentIndex() - 2); });
		});

		// Event timeline control
		lua.writeVariable("e_eventStopTime",		[=](float mDuration) 					{ eventTimeline.append<Do>([=]{ status.timeStop = mDuration; }); });
		lua.writeVariable("e_eventStopTimeS",		[=](float mDuration) 					{ eventTimeline.append<Do>([=]{ status.timeStop = mDuration * 60.f; }); });
		lua.writeVariable("e_eventWait",			[=](float mDuration) 					{ eventTimeline.append<Wait>(mDuration); });
		lua.writeVariable("e_eventWaitS", 			[=](float mDuration) 					{ eventTimeline.append<Wait>(mDuration * 60.f); });
		lua.writeVariable("e_eventWaitUntilS", 		[=](float mDuration)
		{
			eventTimeline.append<Wait>(10);
			eventTimeline.append<Do>([=]{ if(status.currentTime < mDuration) eventTimeline.jumpTo(eventTimeline.getCurrentIndex() - 2); });
		});

		// Level control
		lua.writeVariable("l_setSpeedMult",			[=](float mValue)						{ levelData.speedMult = mValue; });
		lua.writeVariable("l_setSpeedInc",			[=](float mValue)						{ levelData.speedInc = mValue; });
		lua.writeVariable("l_setRotationSpeed",		[=](float mValue)						{ levelData.rotationSpeed = mValue; });
		lua.writeVariable("l_setRotationSpeedMax",	[=](float mValue)						{ levelData.rotationSpeedMax = mValue; });
		lua.writeVariable("l_setRotationSpeedInc",	[=](float mValue)						{ levelData.rotationSpeedInc = mValue; });
		lua.writeVariable("l_setDelayMult",			[=](float mValue)						{ levelData.delayMult = mValue; });
		lua.writeVariable("l_setDelayInc",			[=](float mValue)						{ levelData.delayInc = mValue; });
		lua.writeVariable("l_setFastSpin",			[=](float mValue)						{ levelData.fastSpin = mValue; });
		lua.writeVariable("l_setSides",				[=](int mValue)							{ levelData.sides = mValue; });
		lua.writeVariable("l_setSidesMin",			[=](int mValue)							{ levelData.sidesMin = mValue; });
		lua.writeVariable("l_setSidesMax",			[=](int mValue)							{ levelData.sidesMax = mValue; });
		lua.writeVariable("l_setIncTime",			[=](float mValue)						{ levelData.incTime = mValue; });
		lua.writeVariable("l_setPulseMin",			[=](float mValue)						{ levelData.pulseMin = mValue; });
		lua.writeVariable("l_setPulseMax",			[=](float mValue)						{ levelData.pulseMax = mValue; });
		lua.writeVariable("l_setPulseSpeed",		[=](float mValue)						{ levelData.pulseSpeed = mValue; });
		lua.writeVariable("l_setPulseSpeedR",		[=](float mValue)						{ levelData.pulseSpeedR = mValue; });
		lua.writeVariable("l_setPulseDelayMax",		[=](float mValue)						{ levelData.pulseDelayMax = mValue; });
		lua.writeVariable("l_setBeatPulseMax",		[=](float mValue)						{ levelData.beatPulseMax = mValue; });
		lua.writeVariable("l_setBeatPulseDelayMax",	[=](float mValue)						{ levelData.beatPulseDelayMax = mValue; });
		lua.writeVariable("l_setWallSkewLeft",		[=](float mValue)						{ levelData.wallSkewLeft = mValue; });
		lua.writeVariable("l_setWallSkewRight",		[=](float mValue)						{ levelData.wallSkewRight = mValue; });
		lua.writeVariable("l_setWallAngleLeft",		[=](float mValue)						{ levelData.wallAngleLeft = mValue; });
		lua.writeVariable("l_setWallAngleRight",	[=](float mValue)						{ levelData.wallAngleRight = mValue; });
		lua.writeVariable("l_setRadiusMin",			[=](float mValue)						{ levelData.radiusMin = mValue; });
		lua.writeVariable("l_setSwapEnabled",		[=](bool mValue)						{ levelData.swapEnabled = mValue; });
		lua.writeVariable("l_setTutorialMode",		[=](bool mValue)						{ levelData.tutorialMode = mValue; });
		lua.writeVariable("l_setIncEnabled",		[=](bool mValue)						{ levelData.incEnabled = mValue; });
		lua.writeVariable("l_addTracked",			[=](string mVar, string mName)			{ levelData.trackedVariables.emplace_back(mVar, mName); });
		lua.writeVariable("l_enableRndSideChanges", [=](bool mValue)						{ levelData.rndSideChangesEnabled = mValue; });
		lua.writeVariable("l_getRotationSpeed",		[=]										{ return levelData.rotationSpeed; });
		lua.writeVariable("l_getSides",				[=]										{ return levelData.sides; });
		lua.writeVariable("l_getSpeedMult",			[=]										{ return levelData.speedMult; });
		lua.writeVariable("l_getDelayMult",			[=]										{ return levelData.delayMult; });

		// Style control
		lua.writeVariable("s_setPulseInc",			[=](float mValue)						{ styleData.pulseIncrement = mValue; });
		lua.writeVariable("s_setHueInc",			[=](float mValue)						{ styleData.hueIncrement = mValue; });
		lua.writeVariable("s_getHueInc",			[=]										{ return styleData.hueIncrement; });

		// Wall creation
		lua.writeVariable("w_wall",					[=](int mSide, float mThickness)																					{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {getSpeedMultDM()}); }); });
		lua.writeVariable("w_wallAdj",				[=](int mSide, float mThickness, float mSpeedAdj)																	{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, mSpeedAdj * getSpeedMultDM()); }); });
		lua.writeVariable("w_wallAcc",				[=](int mSide, float mThickness, float mSpeedAdj, float mAcceleration, float mMinSpeed, float mMaxSpeed)			{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {mSpeedAdj * getSpeedMultDM(), mAcceleration, mMinSpeed * getSpeedMultDM(), mMaxSpeed * getSpeedMultDM()}); }); });
		lua.writeVariable("w_wallHModSpeedData",	[=](float mHMod, int mSide, float mThickness, float mSAdj, float mSAcc, float mSMin, float mSMax, bool mSPingPong)	{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {mSAdj * getSpeedMultDM(), mSAcc, mSMin, mSMax, mSPingPong}, mHMod); }); });
		lua.writeVariable("w_wallHModCurveData",	[=](float mHMod, int mSide, float mThickness, float mCAdj, float mCAcc, float mCMin, float mCMax, bool mCPingPong)	{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {getSpeedMultDM()}, {mCAdj, mCAcc, mCMin, mCMax, mCPingPong}, mHMod); }); });
	}
	void HexagonGame::runLuaFile(const string& mFileName)
	{
		ifstream s(mFileName);
		try { lua.executeCode(s); }
		catch(runtime_error &error)
		{
			log("Fatal error, killing the player", "hg::HexagonGame::runLuaFile");
			log("Filename: " + mFileName, "hg::HexagonGame::runLuaFile");
			log("Error: " + toStr(error.what()), "hg::HexagonGame::runLuaFile");
			log("");

			death();
		}
	}
}
