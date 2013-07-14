// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_STYLEDATA
#define HG_STYLEDATA

#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SSVStart/SSVStart.h>

namespace hg
{
	class StyleData
	{
		private:
			ssvuj::Value root;
			float currentHue, currentSwapTime{0}, pulseFactor{0};
			std::string rootPath;
			sf::Color currentMainColor, current3DOverrideColor;
			std::vector<sf::Color> currentColors;

		public:
			StyleData() = default;
			StyleData(const ssvuj::Value& mRoot);

			void update(float mFrameTime);
			void computeColors();
			void drawBackground(sf::RenderTarget& mRenderTarget, ssvs::Vec2f mCenterPos, int mSides);

			void setRootPath(const std::string& mPath) { rootPath = mPath; }
			inline void setPulseIncrement(float mValue)		{ ssvuj::set(root, "pulse_increment", mValue); }
			inline void setHueIncrement(float mValue)		{ ssvuj::set(root, "hue_increment", mValue); }

			inline const std::string& getRootPath() const	{ return rootPath; }
			inline std::string getId() const				{ return ssvuj::as<std::string>(root, "id"); }
			inline float getHueMin() const					{ return ssvuj::as<float>(root, "hue_min"); }
			inline float getHueMax() const					{ return ssvuj::as<float>(root, "hue_max"); }
			inline bool getHuePingPong() const				{ return ssvuj::as<bool>(root, "hue_ping_pong"); }
			inline float getHueIncrement() const			{ return ssvuj::as<float>(root, "hue_increment"); }
			inline float getMaxSwapTime() const				{ return ssvuj::as<float>(root, "max_swap_time", 100.f); }

			sf::Color calculateColor(const ssvuj::Value& mColorRoot) const;

			inline const sf::Color& getMainColor() const			{ return currentMainColor; }
			inline const std::vector<sf::Color>& getColors() const	{ return currentColors; }
			inline const sf::Color& getColor(int mIndex) const		{ return currentColors[mIndex]; }

			inline float getCurrentHue() const 			{ return currentHue; }
			inline float getCurrentSwapTime() const		{ return currentSwapTime; }

			inline unsigned int get3DDepth() const				{ return ssvuj::as<float>(root, "3D_depth", 15); }
			inline float get3DSkew() const						{ return ssvuj::as<float>(root, "3D_skew", 0.18f); }
			inline float get3DSpacing() const					{ return ssvuj::as<float>(root, "3D_spacing", 1.0f); }
			inline float get3DDarkenMultiplier() const			{ return ssvuj::as<float>(root, "3D_darken_multiplier", 1.5f); }
			inline float get3DAlphaMultiplier() const			{ return ssvuj::as<float>(root, "3D_alpha_multiplier", 0.5f); }
			inline float get3DAlphaFalloff() const				{ return ssvuj::as<float>(root, "3D_alpha_falloff", 3.0f); }
			inline const sf::Color& get3DOverrideColor() const	{ return current3DOverrideColor; }
			inline float get3DPulseMax() const					{ return ssvuj::as<float>(root, "3D_pulse_max", 3.2f); }
			inline float get3DPulseMin() const					{ return ssvuj::as<float>(root, "3D_pulse_min", -0.0f); }
			inline float get3DPulseSpeed() const				{ return ssvuj::as<float>(root, "3D_pulse_speed", 0.01f); }
			inline float get3DPerspectiveMultiplier() const		{ return ssvuj::as<float>(root, "3D_perspective_multiplier", 1.0f); }
	};
}

#endif
