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
	class HexagonGame;

	class PatternManager
	{
		friend class HexagonGame;

		public:
			PatternManager(HexagonGame*);

			HexagonGame* hgPtr;

			Timeline& timeline;
			int& sides;
			Vector2f& centerPos;

			void wall(Vector2f, int, float, float);
			void barrage(Vector2f, int, float, float, int);
			void barrageDiv(Vector2f, int, float, float, int);
			void mirrorWall(Vector2f, int, float, float, int);

			void alternateBarrageDiv(int);
			void zigZag(int);
			void spin(int);
			void mirrorSpin(int);
	};
}
#endif /* PATTERNMANAGER_H_ */
