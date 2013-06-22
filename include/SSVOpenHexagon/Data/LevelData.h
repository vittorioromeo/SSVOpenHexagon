// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_LEVELDATA
#define HG_LEVELDATA

#include <vector>
#include <string>
#include <SSVJsonCpp/SSVJsonCpp.h>

namespace hg
{
	class LevelData
	{
		private:
			Json::Value root;
			std::vector<Json::Value> events;
			std::string packPath, levelRootPath, styleRootPath, luaScriptPath;

		public:
			LevelData() = default;
			LevelData(const Json::Value& mRoot);

			void addEvent(const Json::Value& mEventRoot);

			Json::Value& getRoot();

			void setPackPath(const std::string& mPath);
			void setLevelRootPath(const std::string& mPath);
			void setStyleRootPath(const std::string& mPath);
			void setLuaScriptPath(const std::string& mPath);
			std::string getPackPath();
			std::string getLevelRootPath();
			std::string getStyleRootPath();
			std::string getLuaScriptPath();

			std::string getId();
			std::string getName();
			std::string getDescription();
			std::string getAuthor();
			int getMenuPriority();
			bool getSelectable();
			std::string getStyleId();
			std::string getMusicId();
			float getSpeedMultiplier();
			float getSpeedIncrement();
			float getRotationSpeed();
			float getRotationSpeedIncrement();
			float getDelayMultiplier();
			float getDelayIncrement();
			float getFastSpin();
			int getSides();
			int getSidesMax();
			int getSidesMin();
			float getIncrementTime();
			float getPulseMin();
			float getPulseMax();
			float getPulseSpeed();
			float getPulseSpeedR();
			float getPulseDelayMax();
			float getPulseDelayHalfMax();
			float getBeatPulseMax();
			float getBeatPulseDelayMax();
			float getRadiusMin();
			std::vector<float> getDifficultyMultipliers();
			std::vector<Json::Value>& getEvents();

			void setSpeedMultiplier(float mSpeedMultiplier);
			void setDelayMultiplier(float mDelayMultiplier);
			void setRotationSpeed(float mRotationSpeed);
			void setValueFloat(const std::string& mValueName, float mValue);
			float getValueFloat(const std::string& mValueName);
			void setValueInt(const std::string& mValueName, int mValue);
			float getValueInt(const std::string& mValueName);
			void setValueString(const std::string& mValueName, const std::string& mValue);
			std::string getValueString(const std::string& mValueName);
			void setValueBool(const std::string& mValueName, bool mValue);
			bool getValueBool(const std::string& mValueName);
	};
}

#endif
