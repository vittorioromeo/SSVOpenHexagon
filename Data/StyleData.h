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

#include <json/json.h>
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

namespace hg
{
	class StyleData
	{
		private:
			Json::Value root;

			float currentHue;
			float currentSwapTime;

			float pulseFactor{0};

		public:
			StyleData() = default;
			StyleData(Json::Value mRoot);

			void update(float mFrameTime);
			void drawBackground(RenderTarget& mRenderTarget, Vector2f mCenterPos, int mSides);

			string getId();
			float getHueMin();
			float getHueMax();
			bool getHuePingPong();
			float getHueIncrement();

			Color calculateColor(Json::Value mColorRoot);

			Color getMainColor();
			vector<Color> getColors();

			float getCurrentHue();
			float getCurrentSwapTime();

			void setValueFloat(string mValueName, float mValue);
			float getValueFloat(string mValueName);

			void setValueInt(string mValueName, int mValue);
			float getValueInt(string mValueName);

			void setValueString(string mValueName, string mValue);
			string getValueString(string mValueName);

			void setValueBool(string mValueName, bool mValue);
			bool getValueBool(string mValueName);
	};
}

#endif // STYLEDATA_H_
