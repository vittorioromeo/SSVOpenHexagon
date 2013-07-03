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

			void addEvent(const ssvuj::Value& mEventRoot);

			void loadTrackedVariables(const ssvuj::Value& mRoot);

			ssvuj::Value& getRoot();

			void setPackPath(const std::string& mPath);
			void setLevelRootPath(const std::string& mPath);
			void setStyleRootPath(const std::string& mPath);
			void setLuaScriptPath(const std::string& mPath);
			std::string getPackPath() const;
			std::string getLevelRootPath() const;
			std::string getStyleRootPath() const;
			std::string getLuaScriptPath() const;

			std::string getId() const;
			std::string getName() const;
			std::string getDescription() const;
			std::string getAuthor() const;
			int getMenuPriority() const;
			bool getSelectable() const;
			std::string getStyleId() const;
			std::string getMusicId() const;
			float getSpeedMultiplier() const;
			float getSpeedIncrement() const;
			float getRotationSpeed() const;
			float getRotationSpeedIncrement() const;
			float getDelayMultiplier() const;
			float getDelayIncrement() const;
			float getFastSpin() const;
			int getSides() const;
			int getSidesMax() const;
			int getSidesMin() const;
			float getIncrementTime() const;
			float getPulseMin() const;
			float getPulseMax() const;
			float getPulseSpeed() const;
			float getPulseSpeedR() const;
			float getPulseDelayMax() const;
			float getPulseDelayHalfMax() const;
			float getBeatPulseMax() const;
			float getBeatPulseDelayMax() const;
			float getRadiusMin() const;
			std::vector<float> getDifficultyMultipliers() const;
			std::vector<ssvuj::Value>& getEvents();
			const std::vector<TrackedVariable>& getTrackedVariables() const;

			void setSpeedMultiplier(float mSpeedMultiplier);
			void setDelayMultiplier(float mDelayMultiplier);
			void setRotationSpeed(float mRotationSpeed);
			void setValueFloat(const std::string& mValueName, float mValue);
			float getValueFloat(const std::string& mValueName) const;
			void setValueInt(const std::string& mValueName, int mValue);
			float getValueInt(const std::string& mValueName) const;
			void setValueString(const std::string& mValueName, const std::string& mValue);
			std::string getValueString(const std::string& mValueName) const;
			void setValueBool(const std::string& mValueName, bool mValue);
			bool getValueBool(const std::string& mValueName) const;
	};
}

#endif
