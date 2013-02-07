// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

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
