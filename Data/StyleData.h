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

#ifndef STYLEDATA_H_
#define STYLEDATA_H_

#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

namespace hg
{
	class StyleData
	{
		private:
			string id;
			float hueMin;
			float hueMax;
			bool huePingPong;
			float hueIncrement;
			bool huePulse;
			bool mainDynamic;
			float mainDynamicDarkness;
			Color mainStatic;
			bool aDynamic;
			float aDynamicDarkness;
			Color aStatic;
			bool bDynamic;
			bool bDynamicOffset;
			float bDynamicDarkness;
			Color bStatic;

			float currentHue;
			float currentSwapTime;
			Color currentMain;
			Color currentA;
			Color currentB;

			float pulseFactor{1.00f};
			float pulseFactorMin{0.77f};
			float pulseFactorMax{1.10f};
			float pulseFactorIncrement{0.018f};

		public:
			StyleData() = default;
			StyleData(string mId, float mHueMin, float mHueMax, bool mHuePingPong, float mHueIncrement, bool mHuePulse, bool mMainDynamic,
						float mMainDynamicDarkness, Color mMainStatic, bool mADynamic, float mADynamicDarkness, Color mAStatic,
						bool mBDynamic, bool mBDynamicOffset, float mBDynamicDarkness, Color mBStatic);

			void update(float mFrameTime);

			string getId();
			float getHueMin();
			float getHueMax();
			bool getHuePingPong();
			float getHueIncrement();
			bool getHuePulse();
			bool getMainDynamic();
			float getMainDynamicDarkness();
			Color getMainStatic();
			bool getADynamic();
			float getADynamicDarkness();
			Color getAStatic();
			bool getBDynamic();
			bool getBDynamicOffset();
			float getBDynamicDarkness();
			Color getBStatic();

			float getCurrentHue();
			float getCurrentSwapTime();
			Color getCurrentMain();
			Color getCurrentA();
			Color getCurrentB();
	};
}

#endif // STYLEDATA_H_
