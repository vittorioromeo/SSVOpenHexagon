// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef STYLEDATA_H_
#define STYLEDATA_H_

#include <json/json.h>
#include <SFML/Graphics.hpp>

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
			void drawBackground(sf::RenderTarget& mRenderTarget, sf::Vector2f mCenterPos, int mSides);

			std::string getId();
			float getHueMin();
			float getHueMax();
			bool getHuePingPong();
			float getHueIncrement();

			sf::Color calculateColor(Json::Value mColorRoot);

			sf::Color getMainColor();
			std::vector<sf::Color> getColors();

			float getCurrentHue();
			float getCurrentSwapTime();

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

#endif // STYLEDATA_H_
