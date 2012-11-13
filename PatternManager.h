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
			Vector2f& centerPos;
			int& sides;

			float thickness	{baseThickness};
			float speed 	{baseSpeed};

			void initalizeTimeline();
			void finalizeTimeline();

		public:
			PatternManager(HexagonGame*);			

			int getRandomSide();
			int getRandomDirection();
			float getPerfectThickness	(float mBaseThickness);
			float getPerfectDelay		(float mThickness, float mSpeed);

			void wall					(int mSide, float mThickness);						// simple wall
			void rWall					(int mSide, float mThickness);						// wall + its opposite
			void wallExtra				(int mSide, float mThickness, int mExtra = 0);		// wall + neighbors from 1 side (1=neighbor on the right)(-1=neighbor on the left)
			void rWallExtra				(int mSide, float mThickness, int mExtra = 0);		// wallExtra + its opposite
			void barrage				(int mSide, float mThickness, int mNeighbors = 0);	// all walls except one side (neighbors both from left and right)
			void barrageOnlyNeighbors	(int mSide, float mThickness, int mNeighbors = 1);	// all walls except neighbors
			void altBarrage				(int mSide, float mThickness, int mStep);			// pick a wall every X walls

			void alternateWallBarrage	(int mTimes = 5, int mDiv = 2);						// zig-zag
			void barrageSpiral			(int mTimes, float mDelayMultiplier = 1); 			// spiral of barrages with only 1 free side
			void mirrorSpiral			(int mTimes);										// spiral with touching sides, only 1 closed side
			void extraWallVortex		(int mTimes = 1, int mSteps = 2);					// left-left right-right
			void inverseBarrage			(int mTimes = 1);									// barrage 0° and barrage 180°
			void mirrorWallStrip		(int mTimes = 2);									// several rWalls one after another on the same side
			void tunnelBarrage			(int mTimes = 1);									// tunnel of barrages where you have to zig zag
	};
}
#endif /* PATTERNMANAGER_H_ */
