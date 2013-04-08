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
	LevelData::LevelData(Json::Value mRoot) : root{mRoot} { }

	void LevelData::addEvent(Json::Value mEventRoot) { events.push_back(mEventRoot); }

	void LevelData::setPackPath(const string& mPath) 		{ packPath = mPath; }
	void LevelData::setLevelRootPath(const string& mPath) 	{ levelRootPath = mPath; }
	void LevelData::setStyleRootPath(const string& mPath) 	{ styleRootPath = mPath; }
	void LevelData::setLuaScriptPath(const string& mPath)	{ luaScriptPath = mPath; }

	string LevelData::getPackPath() 				{ return packPath; }
	string LevelData::getLevelRootPath()			{ return levelRootPath; }
	string LevelData::getStyleRootPath()			{ return styleRootPath; }
	string LevelData::getLuaScriptPath()			{ return luaScriptPath; }

	Json::Value& LevelData::getRoot()				{ return root; }
	string LevelData::getId() 						{ return getPackPath() + getValue<string>(root, "id"); }
	string LevelData::getName() 					{ return getValue<string>(root, "name"); }
	string LevelData::getDescription() 				{ return getValue<string>(root, "description"); }
	string LevelData::getAuthor() 					{ return getValue<string>(root, "author"); }
	int LevelData::getMenuPriority()				{ return getValue<int>(root, "menu_priority"); }
	bool LevelData::getSelectable()					{ return getValue<bool>(root, "selectable"); }
	string LevelData::getMusicId() 					{ return getValue<string>(root, "music_id"); }
	string LevelData::getStyleId() 					{ return getValue<string>(root, "style_id"); }
	float LevelData::getSpeedMultiplier() 			{ return getValue<float>(root, "speed_multiplier"); }
	float LevelData::getSpeedIncrement() 			{ return getValue<float>(root, "speed_increment"); }
	float LevelData::getRotationSpeed() 			{ return getValue<float>(root, "rotation_speed"); }
	float LevelData::getRotationSpeedIncrement() 	{ return getValue<float>(root, "rotation_increment"); }
	float LevelData::getDelayMultiplier() 			{ return getValue<float>(root, "delay_multiplier"); }
	float LevelData::getDelayIncrement() 			{ return getValue<float>(root, "delay_increment"); }
	float LevelData::getFastSpin() 					{ return getValue<float>(root, "fast_spin"); }
	int LevelData::getSides() 						{ return getValue<int>(root, "sides"); }
	int LevelData::getSidesMax() 					{ return getValue<int>(root, "sides_max"); }
	int LevelData::getSidesMin() 					{ return getValue<int>(root, "sides_min"); }
	float LevelData::getIncrementTime()				{ return getValue<float>(root, "increment_time"); }
	float LevelData::getPulseMin() 					{ return getValueOrDefault(root, "pulse_min", 75.f); }
	float LevelData::getPulseMax() 					{ return getValueOrDefault(root, "pulse_max", 80.f); }
	float LevelData::getPulseSpeed() 				{ return getValueOrDefault(root, "pulse_speed", 0.f); }
	float LevelData::getPulseSpeedR() 				{ return getValueOrDefault(root, "pulse_speed_r", 0.f); }
	float LevelData::getPulseDelayMax() 			{ return getValueOrDefault(root, "pulse_delay_max", 0.f); }
	float LevelData::getPulseDelayHalfMax() 		{ return getValueOrDefault(root, "pulse_delay_half_max", 0.f); }
	float LevelData::getBeatPulseMax() 				{ return getValueOrDefault(root, "beatpulse_max", 0.f); }
	float LevelData::getBeatPulseDelayMax() 		{ return getValueOrDefault(root, "beatpulse_delay_max", 0.f); }
	float LevelData::getRadiusMin() 				{ return getValueOrDefault(root, "radius_min", 72.f); }
	vector<float> LevelData::getDifficultyMultipliers()
	{
		vector<float> result{getContainerOrDefault<vector<float>>(root, "difficulty_multipliers", {})};
		result.push_back(1.0f);
		sort(result); return result;
	}
	vector<Json::Value>& LevelData::getEvents()		{ return events; }

	void LevelData::setSpeedMultiplier(float mSpeedMultiplier)  { root["speed_multiplier"] = mSpeedMultiplier; }
	void LevelData::setDelayMultiplier(float mDelayMultiplier)	{ root["delay_increment"] = mDelayMultiplier; }
	void LevelData::setRotationSpeed(float mRotationSpeed) 		{ root["rotation_speed"] = mRotationSpeed; }

	void LevelData::setValueFloat(const string& mValueName, float mValue)			{ root[mValueName] = mValue; }
	void LevelData::setValueInt(const string& mValueName, int mValue)				{ root[mValueName] = mValue; }
	void LevelData::setValueString(const string& mValueName, const string& mValue)	{ root[mValueName] = mValue; }
	void LevelData::setValueBool(const string& mValueName, bool mValue)				{ root[mValueName] = mValue; }

	float LevelData::getValueFloat(const string& mValueName)						{ return getValue<float>(root, mValueName); }
	float LevelData::getValueInt(const string& mValueName)							{ return getValue<int>(root, mValueName); }
	string LevelData::getValueString(const string& mValueName)						{ return getValue<string>(root, mValueName); }
	bool LevelData::getValueBool(const string& mValueName)							{ return getValue<bool>(root, mValueName); }
}
