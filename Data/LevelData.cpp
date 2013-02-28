// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Data/LevelData.h"
#include "Utils/Utils.h"

using namespace std;

namespace hg
{
	LevelData::LevelData(Json::Value mRoot) : root{mRoot} { }

	void LevelData::addEvent(Json::Value mEventRoot)
	{
		events.push_back(mEventRoot);
	}

	void LevelData::setPackPath(string mPackPath) { packPath = mPackPath; }
	string LevelData::getPackPath() { return packPath; }

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
	float LevelData::getPulseMin()
	{		
		if(root.isMember("pulse_min")) return root["pulse_min"].asFloat();
		return 75.f;
	}
	float LevelData::getPulseMax()
	{
		if(root.isMember("pulse_max")) return root["pulse_max"].asFloat();
		return 80.f;
	}
	float LevelData::getPulseSpeed()
	{
		if(root.isMember("pulse_speed")) return root["pulse_speed"].asFloat();
		return 0.f;
	}
	float LevelData::getPulseSpeedR()
	{
		if(root.isMember("pulse_speed_r")) return root["pulse_speed_r"].asFloat();
		return 0.f;
	}
	float LevelData::getPulseDelayMax()
	{
		if(root.isMember("pulse_delay_max")) return root["pulse_delay_max"].asFloat();
		return 0.f;
	}
	float LevelData::getPulseDelayHalfMax()
	{
		if(root.isMember("pulse_delay_half_max")) return root["pulse_delay_half_max"].asFloat();
		return 0.f;
	}
	float LevelData::getBeatPulseMax()
	{
		if(root.isMember("beatpulse_max")) return root["beatpulse_max"].asFloat();
		return 0.f;
	}
	float LevelData::getBeatPulseDelayMax()
	{
		if(root.isMember("beatpulse_delay_max")) return root["beatpulse_delay_max"].asFloat();
		return 0.f;
	}
	float LevelData::getRadiusMin()
	{
		if(root.isMember("radius_min")) return root["radius_min"].asFloat();
		return 72.f;
	}
	vector<float> LevelData::getDifficultyMultipliers()
	{
		vector<float> result{1.0f};
		if(root.isMember("difficulty_multipliers")) for(Json::Value f : root["difficulty_multipliers"]) result.push_back(f.asFloat());
		sort(begin(result), end(result));
		return result;
	}
	float LevelData::get3DMultiplier()
	{
		if(root.isMember("3d_multiplier")) return root["3d_multiplier"].asFloat();
		return 1.f;
	}
	float LevelData::get3DIncrement()
	{
		if(root.isMember("3d_increment")) return root["3d_increment"].asFloat();
		return 0.001f;
	}
	float LevelData::get3DMax()
	{
		if(root.isMember("3d_max")) return root["3d_max"].asFloat();
		return 0.34f;
	}
	float LevelData::get3DMin()
	{
		if(root.isMember("3d_min")) return root["3d_min"].asFloat();
		return 0.18f;
	}

	vector<Json::Value>& LevelData::getEvents()		{ return events; }

	void LevelData::setSpeedMultiplier(float mSpeedMultiplier)  { root["speed_multiplier"] = mSpeedMultiplier; }
	void LevelData::setDelayMultiplier(float mDelayMultiplier)	{ root["delay_increment"] = mDelayMultiplier; }
	void LevelData::setRotationSpeed(float mRotationSpeed) 		{ root["rotation_speed"] = mRotationSpeed; }

	void LevelData::setValueFloat(string mValueName, float mValue)	{ root[mValueName] = mValue; }
	float LevelData::getValueFloat(string mValueName)				{ return root[mValueName].asFloat(); }

	void LevelData::setValueInt(string mValueName, int mValue)		{ root[mValueName] = mValue; }
	float LevelData::getValueInt(string mValueName)					{ return root[mValueName].asInt(); }

	void LevelData::setValueString(string mValueName, string mValue){ root[mValueName] = mValue; }
	string LevelData::getValueString(string mValueName)				{ return root[mValueName].asString(); }

	void LevelData::setValueBool(string mValueName, bool mValue)	{ root[mValueName] = mValue; }
	bool LevelData::getValueBool(string mValueName)					{ return root[mValueName].asBool(); }
}
