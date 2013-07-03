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

	void LevelData::loadTrackedVariables(const Json::Value& mRoot)
	{
		for(const auto& t : as<Json::Value>(mRoot, "tracked"))
		{
			const string& variableName{as<string>(t, 0)};
			const string& displayName{as<string>(t, 1)};
			bool hasOffset{t.size() == 3};

			if(hasOffset)
			{
				int offset{as<int>(t, 2)};
				trackedVariables.emplace_back(variableName, displayName, offset);
			}
			else trackedVariables.emplace_back(variableName, displayName);
		}
	}

	void LevelData::setPackPath(const string& mPath) 		{ packPath = mPath; }
	void LevelData::setLevelRootPath(const string& mPath) 	{ levelRootPath = mPath; }
	void LevelData::setStyleRootPath(const string& mPath) 	{ styleRootPath = mPath; }
	void LevelData::setLuaScriptPath(const string& mPath)	{ luaScriptPath = mPath; }

	string LevelData::getPackPath() const				{ return packPath; }
	string LevelData::getLevelRootPath() const			{ return levelRootPath; }
	string LevelData::getStyleRootPath() const			{ return styleRootPath; }
	string LevelData::getLuaScriptPath() const			{ return luaScriptPath; }

	Json::Value& LevelData::getRoot()					{ return root; }
	string LevelData::getId() const						{ return getPackPath() + as<string>(root, "id"); }
	string LevelData::getName() const					{ return as<string>(root, "name"); }
	string LevelData::getDescription() const			{ return as<string>(root, "description"); }
	string LevelData::getAuthor() const					{ return as<string>(root, "author"); }
	int LevelData::getMenuPriority() const				{ return as<int>(root, "menu_priority"); }
	bool LevelData::getSelectable() const				{ return as<bool>(root, "selectable"); }
	string LevelData::getMusicId() const				{ return as<string>(root, "music_id"); }
	string LevelData::getStyleId() const				{ return as<string>(root, "style_id"); }
	float LevelData::getSpeedMultiplier() const			{ return as<float>(root, "speed_multiplier"); }
	float LevelData::getSpeedIncrement() const			{ return as<float>(root, "speed_increment"); }
	float LevelData::getRotationSpeed() const			{ return as<float>(root, "rotation_speed"); }
	float LevelData::getRotationSpeedIncrement() const	{ return as<float>(root, "rotation_increment"); }
	float LevelData::getDelayMultiplier() const			{ return as<float>(root, "delay_multiplier"); }
	float LevelData::getDelayIncrement() const			{ return as<float>(root, "delay_increment"); }
	float LevelData::getFastSpin() const				{ return as<float>(root, "fast_spin"); }
	int LevelData::getSides() const						{ return as<int>(root, "sides"); }
	int LevelData::getSidesMax() const					{ return as<int>(root, "sides_max"); }
	int LevelData::getSidesMin() const					{ return as<int>(root, "sides_min"); }
	float LevelData::getIncrementTime() const			{ return as<float>(root, "increment_time"); }
	float LevelData::getPulseMin() const				{ return as<float>(root, "pulse_min", 75.f); }
	float LevelData::getPulseMax() const				{ return as<float>(root, "pulse_max", 80.f); }
	float LevelData::getPulseSpeed() const				{ return as<float>(root, "pulse_speed", 0.f); }
	float LevelData::getPulseSpeedR() const				{ return as<float>(root, "pulse_speed_r", 0.f); }
	float LevelData::getPulseDelayMax() const			{ return as<float>(root, "pulse_delay_max", 0.f); }
	float LevelData::getPulseDelayHalfMax() const		{ return as<float>(root, "pulse_delay_half_max", 0.f); }
	float LevelData::getBeatPulseMax() const			{ return as<float>(root, "beatpulse_max", 0.f); }
	float LevelData::getBeatPulseDelayMax() const		{ return as<float>(root, "beatpulse_delay_max", 0.f); }
	float LevelData::getRadiusMin() const				{ return as<float>(root, "radius_min", 72.f); }
	vector<float> LevelData::getDifficultyMultipliers() const
	{
		auto result(as<vector<float>>(root, "difficulty_multipliers", {}));
		result.push_back(1.0f); sort(result); return result;
	}
	vector<Json::Value>& LevelData::getEvents()			{ return events; }

	void LevelData::setSpeedMultiplier(float mSpeedMultiplier)  { root["speed_multiplier"] = mSpeedMultiplier; }
	void LevelData::setDelayMultiplier(float mDelayMultiplier)	{ root["delay_increment"] = mDelayMultiplier; }
	void LevelData::setRotationSpeed(float mRotationSpeed) 		{ root["rotation_speed"] = mRotationSpeed; }

	void LevelData::setValueFloat(const string& mValueName, float mValue)			{ ssvuj::set(root, mValueName, mValue); }
	void LevelData::setValueInt(const string& mValueName, int mValue)				{ ssvuj::set(root, mValueName, mValue); }
	void LevelData::setValueString(const string& mValueName, const string& mValue)	{ ssvuj::set(root, mValueName, mValue); }
	void LevelData::setValueBool(const string& mValueName, bool mValue)				{ ssvuj::set(root, mValueName, mValue); }

	float LevelData::getValueFloat(const string& mValueName) const					{ return as<float>(root, mValueName); }
	float LevelData::getValueInt(const string& mValueName) const					{ return as<int>(root, mValueName); }
	string LevelData::getValueString(const string& mValueName) const				{ return as<string>(root, mValueName); }
	bool LevelData::getValueBool(const string& mValueName) const					{ return as<bool>(root, mValueName); }

	const vector<TrackedVariable>& LevelData::getTrackedVariables() const			{ return trackedVariables; }
}
