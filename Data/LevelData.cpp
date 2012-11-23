/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Data/LevelData.h"
#include "Utils/Utils.h"

namespace hg
{
	LevelData::LevelData(Json::Value mRoot) : root{mRoot} { }

	function<void(PatternManager* pm)> LevelData::getRandomPattern()
	{
		currentPattern++;

		if(currentPattern == (int)pfuncs.size()) currentPattern = 0;
		if(currentPattern == 0) random_shuffle(pfuncs.begin(), pfuncs.end());

		return pfuncs[currentPattern];
	}

	void LevelData::addPattern(function<void(PatternManager* pm)> mPatternFunc, int mChance)
	{
		for(int i{0}; i < mChance; i++) pfuncs.push_back(mPatternFunc);
	}
	void LevelData::addEvent(Json::Value mEventRoot)
	{
		events.push_back(mEventRoot);
	}

	Json::Value& LevelData::getRoot()				{ return root; }
	string LevelData::getId() 						{ return root["id"].asString(); }
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
