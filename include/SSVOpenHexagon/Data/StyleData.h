// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_STYLEDATA
#define HG_STYLEDATA

#include "json/json.h"
#include <SFML/Graphics.hpp>

namespace hg
{
	class StyleData
	{
		private:
			Json::Value root;
			float currentHue, currentSwapTime{0}, pulseFactor{0};
			std::string rootPath;
			sf::Color currentMainColor;
			std::vector<sf::Color> currentColors;

		public:
			StyleData() = default;
			StyleData(Json::Value mRoot);

			void update(float mFrameTime);
			void computeColors();
			void drawBackground(sf::RenderTarget& mRenderTarget, sf::Vector2f mCenterPos, int mSides);

			void setRootPath(const std::string& mPath);

			std::string getRootPath();
			std::string getId();
			float getHueMin();
			float getHueMax();
			bool getHuePingPong();
			float getHueIncrement();
			float getMaxSwapTime();

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

			unsigned int get3DDepth();
			float get3DSkew();
			float get3DSpacing();
			float get3DDarkenMultiplier();
			float get3DAlphaMultiplier();
			float get3DAlphaFalloff();
			sf::Color get3DOverrideColor();
			float get3DPulseMax();
			float get3DPulseMin();
			float get3DPulseSpeed();
			float get3DPerspectiveMultiplier();
	};
}

#endif
