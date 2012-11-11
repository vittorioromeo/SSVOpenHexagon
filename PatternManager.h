#ifndef PATTERNMANAGER_H_
#define PATTERNMANAGER_H_

#include "CPlayer.h"
#include "CWall.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include "SSVStart.h"
#include "SSVEntitySystem.h"

namespace hg
{
	constexpr float baseSpeed{5};
	constexpr float baseThickness{40};

	class HexagonGame;

	class PatternManager
	{
		friend class HexagonGame;

		private:
			HexagonGame* hgPtr;
			Timeline& timeline;
			int& sides;
			Vector2f& centerPos;

			float thickness {baseThickness};
			float speed {baseSpeed};

			void wallBase(Vector2f, int, float, float);

		public:
			PatternManager(HexagonGame*);			

			int getRandomSide();
			int getRandomDirection();
			float getPerfectThickness	(float mBaseThickness);
			float getPerfectDelay		(float mThickness, float mSpeed);

			void wall					(int mSide, float mThickness);
			void rWall					(int mSide, float mThickness);
			void wallExtra				(int mSide, float mThickness, int mExtra = 0);
			void rWallExtra				(int mSide, float mThickness, int mExtra = 0);
			void barrage				(int mSide, float mThickness, int mNeighbors = 0);
			void barrageOnlyNeighbors	(int mSide, float mThickness, int mNeighbors = 1);
			void altBarrage				(int mSide, float mThickness, int mStep);

			void alternateBarrageDiv	(int mDiv = 2);
			void barrageSpin			(int mTimes);
			void mirrorSpin				(int mTimes);
			void evilRSpin				(int mTimes = 1, int mSteps = 2);
			void inverseBarrage			(int mTimes = 1);
	};
}
#endif /* PATTERNMANAGER_H_ */
