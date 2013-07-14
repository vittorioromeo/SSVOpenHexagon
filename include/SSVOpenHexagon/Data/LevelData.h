// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_LEVELDATA
#define HG_LEVELDATA

#include <vector>
#include <string>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Data/TrackedVariable.h"

namespace hg
{
	class LevelData
	{
		private:
			ssvuj::Value root;
			std::vector<ssvuj::Value> events;
			std::string packPath, levelRootPath, styleRootPath, luaScriptPath;
			std::vector<TrackedVariable> trackedVariables;

		public:
			LevelData() = default;
			LevelData(const ssvuj::Value& mRoot);

			inline void addEvent(const ssvuj::Value& mEventRoot) { events.push_back(mEventRoot); }

			void loadTrackedVariables(const ssvuj::Value& mRoot);

			inline void setPackPath(const std::string& mPath) 		{ packPath = mPath; }
			inline void setLevelRootPath(const std::string& mPath) 	{ levelRootPath = mPath; }
			inline void setStyleRootPath(const std::string& mPath) 	{ styleRootPath = mPath; }
			inline void setLuaScriptPath(const std::string& mPath)	{ luaScriptPath = mPath; }
			inline void setSpeedMultiplier(float mSpeedMultiplier)  { ssvuj::set(root, "speed_multiplier", mSpeedMultiplier); }
			inline void setDelayMultiplier(float mDelayMultiplier)	{ ssvuj::set(root, "delay_multiplier", mDelayMultiplier); }
			inline void setRotationSpeed(float mRotationSpeed) 		{ ssvuj::set(root, "rotation_speed", mRotationSpeed); }
			inline void setValueFloat(const std::string& mValueName, float mValue)					{ ssvuj::set(root, mValueName, mValue); }
			inline void setValueInt(const std::string& mValueName, int mValue)						{ ssvuj::set(root, mValueName, mValue); }
			inline void setValueString(const std::string& mValueName, const std::string& mValue)	{ ssvuj::set(root, mValueName, mValue); }
			inline void setValueBool(const std::string& mValueName, bool mValue)					{ ssvuj::set(root, mValueName, mValue); }

			inline ssvuj::Value& getRoot() { return root; }
			inline const std::string& getPackPath() const		{ return packPath; }
			inline const std::string& getLevelRootPath() const	{ return levelRootPath; }
			inline const std::string& getStyleRootPath() const	{ return styleRootPath; }
			inline const std::string& getLuaScriptPath() const	{ return luaScriptPath; }
			inline std::string getId() const				{ return getPackPath() + ssvuj::as<std::string>(root, "id"); }
			inline std::string getName() const				{ return ssvuj::as<std::string>(root, "name"); }
			inline std::string getDescription() const		{ return ssvuj::as<std::string>(root, "description"); }
			inline std::string getAuthor() const			{ return ssvuj::as<std::string>(root, "author"); }
			inline int getMenuPriority() const				{ return ssvuj::as<int>(root, "menu_priority"); }
			inline bool getSelectable() const				{ return ssvuj::as<bool>(root, "selectable"); }
			inline std::string getMusicId() const			{ return ssvuj::as<std::string>(root, "music_id"); }
			inline std::string getStyleId() const			{ return ssvuj::as<std::string>(root, "style_id"); }
			inline float getSpeedMultiplier() const			{ return ssvuj::as<float>(root, "speed_multiplier"); }
			inline float getSpeedIncrement() const			{ return ssvuj::as<float>(root, "speed_increment"); }
			inline float getRotationSpeed() const			{ return ssvuj::as<float>(root, "rotation_speed"); }
			inline float getRotationSpeedIncrement() const	{ return ssvuj::as<float>(root, "rotation_increment"); }
			inline float getDelayMultiplier() const			{ return ssvuj::as<float>(root, "delay_multiplier"); }
			inline float getDelayIncrement() const			{ return ssvuj::as<float>(root, "delay_increment"); }
			inline float getFastSpin() const				{ return ssvuj::as<float>(root, "fast_spin"); }
			inline int getSides() const						{ return ssvuj::as<int>(root, "sides"); }
			inline int getSidesMax() const					{ return ssvuj::as<int>(root, "sides_max"); }
			inline int getSidesMin() const					{ return ssvuj::as<int>(root, "sides_min"); }
			inline float getIncrementTime() const			{ return ssvuj::as<float>(root, "increment_time"); }
			inline float getPulseMin() const				{ return ssvuj::as<float>(root, "pulse_min", 75.f); }
			inline float getPulseMax() const				{ return ssvuj::as<float>(root, "pulse_max", 80.f); }
			inline float getPulseSpeed() const				{ return ssvuj::as<float>(root, "pulse_speed", 0.f); }
			inline float getPulseSpeedR() const				{ return ssvuj::as<float>(root, "pulse_speed_r", 0.f); }
			inline float getPulseDelayMax() const			{ return ssvuj::as<float>(root, "pulse_delay_max", 0.f); }
			inline float getPulseDelayHalfMax() const		{ return ssvuj::as<float>(root, "pulse_delay_half_max", 0.f); }
			inline float getBeatPulseMax() const			{ return ssvuj::as<float>(root, "beatpulse_max", 0.f); }
			inline float getBeatPulseDelayMax() const		{ return ssvuj::as<float>(root, "beatpulse_delay_max", 0.f); }
			inline float getRadiusMin() const				{ return ssvuj::as<float>(root, "radius_min", 72.f); }
			std::vector<float> getDifficultyMultipliers() const;
			inline std::vector<ssvuj::Value>& getEvents() { return events; }
			inline const std::vector<TrackedVariable>& getTrackedVariables() const { return trackedVariables; }
			inline float getValueFloat(const std::string& mValueName) const							{ return ssvuj::as<float>(root, mValueName); }
			inline float getValueInt(const std::string& mValueName) const							{ return ssvuj::as<int>(root, mValueName); }
			inline std::string getValueString(const std::string& mValueName) const					{ return ssvuj::as<std::string>(root, mValueName); }
			inline bool getValueBool(const std::string& mValueName) const							{ return ssvuj::as<bool>(root, mValueName); }
			inline bool getSwapEnabled() const { return ssvuj::as<bool>(root, "swap_enabled", false); }
	};
}

#endif
