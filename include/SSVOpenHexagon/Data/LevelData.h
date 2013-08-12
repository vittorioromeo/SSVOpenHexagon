// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_LEVELDATA
#define HG_LEVELDATA

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Data/TrackedVariable.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	class LevelData
	{
		private:
			ssvuj::Obj root;

		public:
			Path packPath;

			std::string id						{packPath.getStr() + ssvuj::as<std::string>(root, "id", "nullId")};
			std::string name					{ssvuj::as<std::string>(root, "name", "nullName")};
			std::string description				{ssvuj::as<std::string>(root, "description", "")};
			std::string author					{ssvuj::as<std::string>(root, "author", "")};
			int menuPriority					{ssvuj::as<int>(root, "menuPriority", 0)};
			bool selectable						{ssvuj::as<bool>(root, "selectable", true)};
			std::string musicId					{ssvuj::as<std::string>(root, "musicId", "nullMusicId")};
			std::string styleId					{ssvuj::as<std::string>(root, "styleId", "nullStyleId")};
			Path luaScriptPath					{packPath + ssvuj::as<std::string>(root, "luaFile", "nullLuaPath")};
			std::vector<float> difficultyMults	{ssvuj::as<std::vector<float>>(root, "difficultyMults", {})};

			LevelData(const ssvuj::Obj& mRoot, const Path& mPackPath) : root{mRoot}, packPath{mPackPath} { difficultyMults.push_back(1.0f); ssvu::sort(difficultyMults); }

			std::string getRootString() const { return ssvuj::getWriteToString(root); }
	};

	struct LevelStatus
	{
		std::vector<TrackedVariable> trackedVariables;
		float speedMult{1.f}, speedInc{0.f};
		float rotationSpeed{0.f}, rotationSpeedInc{0.f}, rotationSpeedMax{0.f};
		float delayMult{1.f}, delayInc{0.f};
		float fastSpin{0.f};
		float incTime{15.f};
		float pulseMin{75.f}, pulseMax{80.f}, pulseSpeed{0.f}, pulseSpeedR{0.f};
		float pulseDelayMax{0.f}, pulseDelayHalfMax{0.f};
		float beatPulseMax{0.f}, beatPulseDelayMax{0.f};
		float radiusMin{72.f};
		float wallSkewLeft{0.f}, wallSkewRight{0.f};
		float wallAngleLeft{0.f}, wallAngleRight{0.f};
		float _3dEffectMultiplier{1.f};
		int sides{6}, sidesMax{6}, sidesMin{6};
		bool swapEnabled{false};
		bool tutorialMode{false};
		bool incEnabled{true};
		bool rndSideChangesEnabled{true};
	};
}

#endif
