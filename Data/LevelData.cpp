// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Data/LevelData.h"
#include "Utils/Utils.h"

using namespace std;

namespace hg
{
	LevelData::LevelData(const string& mValidator, Json::Value mRoot) : validator{mValidator}, root{mRoot} { }

	void LevelData::addEvent(Json::Value mEventRoot) { events.push_back(mEventRoot); }

	void LevelData::setPackPath(string mPackPath) 	{ packPath = mPackPath; }
	string LevelData::getPackPath() 				{ return packPath; }

	string LevelData::getValidator() 				{ return validator; }
	Json::Value& LevelData::getRoot()				{ return root; }
	string LevelData::getId() 						{ return getPackPath() + root["id"].asString(); }
	string LevelData::getName() 					{ return root["name"].asString(); }
	string LevelData::getDescription() 				{ return root["description"].asString(); }
	string LevelData::getAuthor() 					{ return root["author"].asString(); }
	int LevelData::getMenuPriority()				{ return root["menu_priority"].asInt(); }
	bool LevelData::getSelectable()					{ return root["selectable"].asBool(); }
	string LevelData::getMusicId() 					{ return root["music_id"].asString(); }
	string LevelData::getStyleId() 					{ return root["style_id"].asString(); }
	float LevelData::getSpeedMultiplier() 			{ return root["speed_multiplier"].asFloat(); }
	float LevelData::getSpeedIncrement() 			{ return root["speed_increment"].asFloat(); }
	float LevelData::getRotationSpeed() 			{ return root["rotation_speed"].asFloat(); }
	float LevelData::getRotationSpeedIncrement() 	{ return root["rotation_increment"].asFloat(); }
	float LevelData::getDelayMultiplier() 			{ return root["delay_multiplier"].asFloat(); }
	float LevelData::getDelayIncrement() 			{ return root["delay_increment"].asFloat(); }
	float LevelData::getFastSpin() 					{ return root["fast_spin"].asFloat(); }
	int LevelData::getSides() 						{ return root["sides"].asInt(); }
	int LevelData::getSidesMax() 					{ return root["sides_max"].asInt(); }
	int LevelData::getSidesMin() 					{ return root["sides_min"].asInt(); }
	float LevelData::getIncrementTime()				{ return root["increment_time"].asFloat(); }
	float LevelData::getPulseMin() 					{ return getJsonValueOrDefault(root, "pulse_min", 75.f); }
	float LevelData::getPulseMax() 					{ return getJsonValueOrDefault(root, "pulse_max", 80.f); }
	float LevelData::getPulseSpeed() 				{ return getJsonValueOrDefault(root, "pulse_speed", 0.f); }
	float LevelData::getPulseSpeedR() 				{ return getJsonValueOrDefault(root, "pulse_speed_r", 0.f); }
	float LevelData::getPulseDelayMax() 			{ return getJsonValueOrDefault(root, "pulse_delay_max", 0.f); }
	float LevelData::getPulseDelayHalfMax() 		{ return getJsonValueOrDefault(root, "pulse_delay_half_max", 0.f); }
	float LevelData::getBeatPulseMax() 				{ return getJsonValueOrDefault(root, "beatpulse_max", 0.f); }
	float LevelData::getBeatPulseDelayMax() 		{ return getJsonValueOrDefault(root, "beatpulse_delay_max", 0.f); }
	float LevelData::getRadiusMin() 				{ return getJsonValueOrDefault(root, "radius_min", 72.f); }
	vector<float> LevelData::getDifficultyMultipliers()
	{
		vector<float> result{1.0f};
		if(root.isMember("difficulty_multipliers")) for(Json::Value f : root["difficulty_multipliers"]) result.push_back(f.asFloat());
		sort(begin(result), end(result));
		return result;
	}

	vector<Json::Value>& LevelData::getEvents()		{ return events; }

	void LevelData::setSpeedMultiplier(float mSpeedMultiplier)  { root["speed_multiplier"] = mSpeedMultiplier; }
	void LevelData::setDelayMultiplier(float mDelayMultiplier)	{ root["delay_increment"] = mDelayMultiplier; }
	void LevelData::setRotationSpeed(float mRotationSpeed) 		{ root["rotation_speed"] = mRotationSpeed; }

	void LevelData::setValueFloat(const string& mValueName, float mValue)			{ root[mValueName] = mValue; }
	float LevelData::getValueFloat(const string& mValueName)						{ return root[mValueName].asFloat(); }
	void LevelData::setValueInt(const string& mValueName, int mValue)				{ root[mValueName] = mValue; }
	float LevelData::getValueInt(const string& mValueName)							{ return root[mValueName].asInt(); }
	void LevelData::setValueString(const string& mValueName, const string& mValue)	{ root[mValueName] = mValue; }
	string LevelData::getValueString(const string& mValueName)						{ return root[mValueName].asString(); }
	void LevelData::setValueBool(const string& mValueName, bool mValue)				{ root[mValueName] = mValue; }
	bool LevelData::getValueBool(const string& mValueName)							{ return root[mValueName].asBool(); }
}
