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

#ifndef PATTERNMANAGER_H_
#define PATTERNMANAGER_H_

#include <iostream>
#include <sstream>
#include <string>
#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Utils/Utils.h"

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

			float thickness	{baseThickness};
			float speed 	{baseSpeed};

			float currentSpeedMultiplier;		
			float currentDelayMultiplier;

			float adjDelay{1.0f};
			float adjSpeed{1.0f};
			float adjThickness{1.0f};

			void initalizeTimeline();
			void finalizeTimeline();
			
		public:
			PatternManager(HexagonGame*);			

			void setAdj(float mAdjDelay, float mAdjSpeed, float mAdjThickness);
			void resetAdj();

			void wall					(int mSide, float mThickness);
	};
}
#endif /* PATTERNMANAGER_H_ */
