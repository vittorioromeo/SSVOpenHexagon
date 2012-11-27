#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <SFML/Audio.hpp>
#include <SSVStart.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Data/StyleData.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Global/Factory.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"

namespace hg
{
	void HexagonGame::executeEvents(Json::Value& mRoot, float mTime)
	{
		for (Json::Value& eventRoot : mRoot)
		{
			if(eventRoot["time"].asFloat() >  mTime) continue;
			if(eventRoot["executed"].asBool()) continue;
			eventRoot["executed"] = true;

			string type		{eventRoot["type"].asString()};
			float duration	{eventRoot["duration"].asFloat()};
			string valueName{eventRoot["value_name"].asString()};
			float value		{eventRoot["value"].asFloat()};
			string message	{eventRoot["message"].asString()};
			string id		{eventRoot["id"].asString()};

			if 		(type == "level_change")			{ mustRestart = true; restartId = id; restartFirstTime = true; return; }
			else if (type == "menu") 					goToMenu();
			else if (type == "message_add")				{ if(firstPlay && getShowMessages()) addMessage(message, duration); }
			else if (type == "message_important_add")	{ if(getShowMessages()) addMessage(message, duration); }
			else if (type == "message_clear") 			clearMessage();
			else if (type == "time_stop")				timeStop = duration;
			else if (type == "timeline_wait") 			timeline.push_back(new Wait(duration));
			else if (type == "timeline_clear") 			clearAndResetTimeline(timeline);

			else if (type == "level_float_set") 		levelData.setValueFloat(valueName, value);
			else if (type == "level_float_add") 		levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) + value);
			else if (type == "level_float_subtract") 	levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) - value);
			else if (type == "level_float_multiply") 	levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) * value);
			else if (type == "level_float_divide") 		levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) / value);
			else if (type == "level_int_set") 			levelData.setValueInt(valueName, value);
			else if (type == "level_int_add") 			levelData.setValueInt(valueName, levelData.getValueFloat(valueName) + value);
			else if (type == "level_int_subtract")		levelData.setValueInt(valueName, levelData.getValueFloat(valueName) - value);
			else if (type == "level_int_multiply") 		levelData.setValueInt(valueName, levelData.getValueFloat(valueName) * value);
			else if (type == "level_int_divide") 		levelData.setValueInt(valueName, levelData.getValueFloat(valueName) / value);

			else if (type == "style_float_set") 		styleData.setValueFloat(valueName, value);
			else if (type == "style_float_add") 		styleData.setValueFloat(valueName, levelData.getValueFloat(valueName) + value);
			else if (type == "style_float_subtract") 	styleData.setValueFloat(valueName, levelData.getValueFloat(valueName) - value);
			else if (type == "style_float_multiply") 	styleData.setValueFloat(valueName, levelData.getValueFloat(valueName) * value);
			else if (type == "style_float_divide") 		styleData.setValueFloat(valueName, levelData.getValueFloat(valueName) / value);
			else if (type == "style_int_set") 			styleData.setValueInt(valueName, value);
			else if (type == "style_int_add") 			styleData.setValueInt(valueName, levelData.getValueFloat(valueName) + value);
			else if (type == "style_int_subtract")		styleData.setValueInt(valueName, levelData.getValueFloat(valueName) - value);
			else if (type == "style_int_multiply") 		styleData.setValueInt(valueName, levelData.getValueFloat(valueName) * value);
			else if (type == "style_int_divide") 		styleData.setValueInt(valueName, levelData.getValueFloat(valueName) / value);

			else if (type == "music_set")				{ if(getChangeMusic()) { stopLevelMusic(); musicData = getMusicData(id); musicData.playRandomSegment(musicPtr); } }
			else if (type == "music_set_segment")		{ if(getChangeMusic()) { stopLevelMusic(); musicData = getMusicData(id); musicData.playSegment(musicPtr, eventRoot["segment_index"].asInt()); } }
			else if (type == "music_set_seconds")		{ if(getChangeMusic()) { stopLevelMusic(); musicData = getMusicData(id); musicData.playSeconds(musicPtr, eventRoot["seconds"].asInt()); } }
			else if (type == "style_set")				{ if(getChangeStyles()) styleData = getStyleData(id); }
			else if (type == "side_changing_stop")		randomSideChangesEnabled = false;
			else if (type == "side_changing_start")		randomSideChangesEnabled = true;
			else if (type == "increment_stop")			incrementEnabled = false;
			else if (type == "increment_start")			incrementEnabled = true;
			else if (type == "pulse_max_set")			maxPulse = value;
			else if (type == "pulse_min_set")			minPulse = value;
			else if (type == "pulse_speed_set")			pulseSpeed = value;
			else if (type == "pulse_speed_b_set")		pulseSpeedBackwards = value;
			else if (type == "event_exec")				events.push_back(getEventData(id, this));
			else if (type == "event_enqueue")			eventQueue.push(getEventData(id, this));
			else if (type == "script_exec")				runLuaFile(valueName);
			else if (type == "play_sound")				playSound(id);
			else										log("unknown event type: " + type);
		}
	}

	void HexagonGame::initLua()
	{
		lua.writeVariable("log", 					[=](string mLog) 						{ log("LUA log: " + mLog); });

		lua.writeVariable("wall", 					[=](int mSide, float mThickness) 					{ timeline.push_back(new Do{[=]{ pm->wall(mSide, mThickness); }}); });
		lua.writeVariable("wallAdj", 				[=](int mSide, float mThickness, float mSpeedAdj) 	{ timeline.push_back(new Do{[=]{ pm->wallAdj(mSide, mThickness, mSpeedAdj); }}); });
		lua.writeVariable("getSides", 				[=]() 									{ return levelData.getSides(); });
		lua.writeVariable("getSpeedMult",			[=]() 									{ return getSpeedMultiplier(); });
		lua.writeVariable("getDelayMult", 			[=]() 									{ return getDelayMultiplier(); });
		lua.writeVariable("getDifficultyMult",		[=]() 									{ return difficultyMult; });
		lua.writeVariable("execScript", 			[=](string mName) 						{ runLuaFile(mName); });
		lua.writeVariable("execEvent", 				[=](string mId) 						{ events.push_back(getEventData(mId, this)); });
		lua.writeVariable("enqueueEvent", 			[=](string mId) 						{ eventQueue.push(getEventData(mId, this)); });
		lua.writeVariable("wait", 					[=](float mDuration) 					{ timeline.push_back(new Wait(mDuration)); });

		lua.writeVariable("playSound", 				[=](string mId) 						{ playSound(mId); });
		lua.writeVariable("forceIncrement", 		[=]()			 						{ incrementDifficulty(); });

		lua.writeVariable("messageAdd", 			[=](string mMessage, float mDuration)	{ if(firstPlay && getShowMessages()) addMessage(mMessage, mDuration); });
		lua.writeVariable("messageImportantAdd",	[=](string mMessage, float mDuration)	{ if(getShowMessages()) addMessage(mMessage, mDuration); });

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
	}
	void HexagonGame::runLuaFile(string mFileName)
	{
		ifstream s("Scripts/" + mFileName);
		lua.executeCode(s);
	}
}
