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
	LevelData::LevelData(const Json::Value& mRoot) : root{mRoot} { }

	void LevelData::addEvent(const Json::Value& mEventRoot) { events.push_back(mEventRoot); }

	void LevelData::setPackPath(const string& mPath) 		{ packPath = mPath; }
	void LevelData::setLevelRootPath(const string& mPath) 	{ levelRootPath = mPath; }
	void LevelData::setStyleRootPath(const string& mPath) 	{ styleRootPath = mPath; }
	void LevelData::setLuaScriptPath(const string& mPath)	{ luaScriptPath = mPath; }

	string LevelData::getPackPath() 				{ return packPath; }
	string LevelData::getLevelRootPath()			{ return levelRootPath; }
	string LevelData::getStyleRootPath()			{ return styleRootPath; }
	string LevelData::getLuaScriptPath()			{ return luaScriptPath; }

	Json::Value& LevelData::getRoot()				{ return root; }
	string LevelData::getId() 						{ return getPackPath() + as<string>(root, "id"); }
	string LevelData::getName() 					{ return as<string>(root, "name"); }
	string LevelData::getDescription() 				{ return as<string>(root, "description"); }
	string LevelData::getAuthor() 					{ return as<string>(root, "author"); }
	int LevelData::getMenuPriority()				{ return as<int>(root, "menu_priority"); }
	bool LevelData::getSelectable()					{ return as<bool>(root, "selectable"); }
	string LevelData::getMusicId() 					{ return as<string>(root, "music_id"); }
	string LevelData::getStyleId() 					{ return as<string>(root, "style_id"); }
	float LevelData::getSpeedMultiplier() 			{ return as<float>(root, "speed_multiplier"); }
	float LevelData::getSpeedIncrement() 			{ return as<float>(root, "speed_increment"); }
	float LevelData::getRotationSpeed() 			{ return as<float>(root, "rotation_speed"); }
	float LevelData::getRotationSpeedIncrement() 	{ return as<float>(root, "rotation_increment"); }
	float LevelData::getDelayMultiplier() 			{ return as<float>(root, "delay_multiplier"); }
	float LevelData::getDelayIncrement() 			{ return as<float>(root, "delay_increment"); }
	float LevelData::getFastSpin() 					{ return as<float>(root, "fast_spin"); }
	int LevelData::getSides() 						{ return as<int>(root, "sides"); }
	int LevelData::getSidesMax() 					{ return as<int>(root, "sides_max"); }
	int LevelData::getSidesMin() 					{ return as<int>(root, "sides_min"); }
	float LevelData::getIncrementTime()				{ return as<float>(root, "increment_time"); }
	float LevelData::getPulseMin() 					{ return as<float>(root, "pulse_min", 75.f); }
	float LevelData::getPulseMax() 					{ return as<float>(root, "pulse_max", 80.f); }
	float LevelData::getPulseSpeed() 				{ return as<float>(root, "pulse_speed", 0.f); }
	float LevelData::getPulseSpeedR() 				{ return as<float>(root, "pulse_speed_r", 0.f); }
	float LevelData::getPulseDelayMax() 			{ return as<float>(root, "pulse_delay_max", 0.f); }
	float LevelData::getPulseDelayHalfMax() 		{ return as<float>(root, "pulse_delay_half_max", 0.f); }
	float LevelData::getBeatPulseMax() 				{ return as<float>(root, "beatpulse_max", 0.f); }
	float LevelData::getBeatPulseDelayMax() 		{ return as<float>(root, "beatpulse_delay_max", 0.f); }
	float LevelData::getRadiusMin() 				{ return as<float>(root, "radius_min", 72.f); }
	vector<float> LevelData::getDifficultyMultipliers()
	{
		vector<float> result{as<vector<float>>(root, "difficulty_multipliers", {})};
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

	float LevelData::getValueFloat(const string& mValueName)						{ return as<float>(root, mValueName); }
	float LevelData::getValueInt(const string& mValueName)							{ return as<int>(root, mValueName); }
	string LevelData::getValueString(const string& mValueName)						{ return as<string>(root, mValueName); }
	bool LevelData::getValueBool(const string& mValueName)							{ return as<bool>(root, mValueName); }
}
