// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_STYLEDATA
#define HG_STYLEDATA

#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SFML/Graphics.hpp>

namespace hg
{
	class StyleData
	{
		private:
			Json::Value root;
			float currentHue, currentSwapTime{0}, pulseFactor{0};
			std::string rootPath;
			sf::Color currentMainColor, current3DOverrideColor;
			std::vector<sf::Color> currentColors;

		public:
			StyleData() = default;
			StyleData(const Json::Value& mRoot);

			void update(float mFrameTime);
			void computeColors();
			void drawBackground(sf::RenderTarget& mRenderTarget, sf::Vector2f mCenterPos, int mSides);

			void setRootPath(const std::string& mPath);

			std::string getRootPath() const;
			std::string getId() const;
			float getHueMin() const;
			float getHueMax() const;
			bool getHuePingPong() const;
			float getHueIncrement() const;
			float getMaxSwapTime() const;

			sf::Color calculateColor(const Json::Value& mColorRoot) const;

			sf::Color getMainColor() const;
			std::vector<sf::Color> getColors() const;

			float getCurrentHue() const;
			float getCurrentSwapTime() const;

			void setValueFloat(const std::string& mValueName, float mValue);
			float getValueFloat(const std::string& mValueName) const;
			void setValueInt(const std::string& mValueName, int mValue);
			int getValueInt(const std::string& mValueName) const;
			void setValueString(const std::string& mValueName, const std::string& mValue);
			std::string getValueString(const std::string& mValueName) const;
			void setValueBool(const std::string& mValueName, bool mValue);
			bool getValueBool(const std::string& mValueName) const;

			unsigned int get3DDepth() const;
			float get3DSkew() const;
			float get3DSpacing() const;
			float get3DDarkenMultiplier() const;
			float get3DAlphaMultiplier() const;
			float get3DAlphaFalloff() const;
			sf::Color get3DOverrideColor() const;
			float get3DPulseMax() const;
			float get3DPulseMin() const;
			float get3DPulseSpeed() const;
			float get3DPerspectiveMultiplier() const;
	};
}

#endif
