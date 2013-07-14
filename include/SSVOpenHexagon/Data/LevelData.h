// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_LEVELDATA
#define HG_LEVELDATA

#include <vector>
#include <string>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Data/TrackedVariable.h"

namespace hg
{
	class LevelData
	{
		private:
			ssvuj::Value root;

		public:
			std::vector<TrackedVariable> trackedVariables;
			std::string packPath, levelRootPath, styleRootPath, luaScriptPath;
			std::vector<float> difficultyMultipliers{ssvuj::as<std::vector<float>>(root, "difficulty_multipliers", {})};
			float speedMultiplier				{ssvuj::as<float>(root, "speed_multiplier", 1.f)};
			float speedIncrement				{ssvuj::as<float>(root, "speed_increment", 0.f)};
			float rotationSpeed					{ssvuj::as<float>(root, "rotation_speed", 0.f)};
			float rotationSpeedIncrement		{ssvuj::as<float>(root, "rotation_increment", 0.f)};
			float delayMultiplier				{ssvuj::as<float>(root, "delay_multiplier", 1.f)};
			float delayIncrement				{ssvuj::as<float>(root, "delay_increment", 0.f)};
			float fastSpin						{ssvuj::as<float>(root, "fast_spin", 0.f)};
			float incrementTime					{ssvuj::as<float>(root, "increment_time", 15.f)};
			float pulseMin						{ssvuj::as<float>(root, "pulse_min", 75.f)};
			float pulseMax						{ssvuj::as<float>(root, "pulse_max", 80.f)};
			float pulseSpeed					{ssvuj::as<float>(root, "pulse_speed", 0.f)};
			float pulseSpeedR					{ssvuj::as<float>(root, "pulse_speed_r", 0.f)};
			float pulseDelayMax					{ssvuj::as<float>(root, "pulse_delay_max", 0.f)};
			float pulseDelayHalfMax				{ssvuj::as<float>(root, "pulse_delay_half_max", 0.f)};
			float beatPulseMax					{ssvuj::as<float>(root, "beatpulse_max", 0.f)};
			float beatPulseDelayMax				{ssvuj::as<float>(root, "beatpulse_delay_max", 0.f)};
			float radiusMin						{ssvuj::as<float>(root, "radius_min", 72.f)};
			float wallSkewLeft					{ssvuj::as<float>(root, "wall_skew_left", 0.f)};
			float wallSkewRight					{ssvuj::as<float>(root, "wall_skew_right", 0.f)};
			float wallAngleLeft					{ssvuj::as<float>(root, "wall_angle_left", 0.f)};
			float wallAngleRight				{ssvuj::as<float>(root, "wall_angle_right", 0.f)};
			float _3dEffectMultiplier			{ssvuj::as<float>(root, "3d_effect_multiplier", 1.f)};
			float rotationSpeedMax				{ssvuj::as<float>(root, "rotation_speed_max", 0.f)};
			int sides							{ssvuj::as<int>(root, "sides", 6)};
			int sidesMax						{ssvuj::as<int>(root, "sides_max", 6)};
			int sidesMin						{ssvuj::as<int>(root, "sides_min", 6)};
			bool swapEnabled					{ssvuj::as<bool>(root, "swap_enabled", false)};
			int menuPriority					{ssvuj::as<int>(root, "menu_priority", 0)};
			bool selectable						{ssvuj::as<bool>(root, "selectable", true)};
			std::string id						{packPath + ssvuj::as<std::string>(root, "id", "nullId")};
			std::string name					{ssvuj::as<std::string>(root, "name", "nullName")};
			std::string description				{ssvuj::as<std::string>(root, "description", "")};
			std::string author					{ssvuj::as<std::string>(root, "author", "")};
			std::string musicId					{ssvuj::as<std::string>(root, "music_id", "nullMusicId")};
			std::string styleId					{ssvuj::as<std::string>(root, "style_id", "nullStyleId")};

			LevelData() = default;
			LevelData(const ssvuj::Value& mRoot) : root{mRoot} { difficultyMultipliers.push_back(1.0f); ssvu::sort(difficultyMultipliers); }
	};
}

#endif
