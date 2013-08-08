// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_STYLEDATA
#define HG_STYLEDATA

#include "SSVOpenHexagon/Core/HGDependencies.h"

namespace hg
{
	class StyleData
	{
		private:
			struct ColorData
			{
				ssvuj::Value root;
				ColorData(const ssvuj::Value& mRoot) : root{mRoot} { }

				bool main					{ssvuj::as<bool>(root, "main", false)};
				bool dynamic				{ssvuj::as<bool>(root, "dynamic", false)};
				bool dynamicOffset			{ssvuj::as<bool>(root, "dynamic_offset", false)};
				float dynamicDarkness		{ssvuj::as<float>(root, "dynamic_darkness", 1.f)};
				float hueShift				{ssvuj::as<float>(root, "hue_shift", 0.f)};
				float offset				{ssvuj::as<float>(root, "offset", 0.f)};
				sf::Color color				{ssvuj::as<sf::Color>(root, "value", sf::Color::White)};
				sf::Color pulse				{ssvuj::as<sf::Color>(root, "pulse", sf::Color::White)};
			};

			ssvuj::Value root;
			float currentHue, currentSwapTime{0}, pulseFactor{0};
			std::string rootPath;
			sf::Color currentMainColor, current3DOverrideColor;
			std::vector<sf::Color> currentColors;

			sf::Color calculateColor(const ColorData& mColorData) const;

		public:
			std::string id						{ssvuj::as<std::string>(root, "id", "nullId")};
			float hueMin						{ssvuj::as<float>(root, "hue_min", 0.f)};
			float hueMax						{ssvuj::as<float>(root, "hue_max", 360.f)};
			float hueIncrement					{ssvuj::as<float>(root, "hue_increment", 0.f)};
			float pulseMin						{ssvuj::as<float>(root, "pulse_min", 0.f)};
			float pulseMax						{ssvuj::as<float>(root, "pulse_max", 0.f)};
			float pulseIncrement				{ssvuj::as<float>(root, "pulse_increment", 0.f)};
			bool huePingPong					{ssvuj::as<bool>(root, "hue_ping_pong", false)};
			float maxSwapTime					{ssvuj::as<float>(root, "max_swap_time", 100.f)};
			float _3dDepth						{ssvuj::as<float>(root, "3D_depth", 15.f)};
			float _3dSkew						{ssvuj::as<float>(root, "3D_skew", 0.18f)};
			float _3dSpacing					{ssvuj::as<float>(root, "3D_spacing", 1.f)};
			float _3dDarkenMult					{ssvuj::as<float>(root, "3D_darken_multiplier", 1.5f)};
			float _3dAlphaMult					{ssvuj::as<float>(root, "3D_alpha_multiplier", 0.5f)};
			float _3dAlphaFalloff				{ssvuj::as<float>(root, "3D_alpha_falloff", 3.f)};
			float _3dPulseMax					{ssvuj::as<float>(root, "3D_pulse_max", 3.2f)};
			float _3dPulseMin					{ssvuj::as<float>(root, "3D_pulse_min", 0.f)};
			float _3dPulseSpeed					{ssvuj::as<float>(root, "3D_pulse_speed", 0.01f)};
			float _3dPerspectiveMult			{ssvuj::as<float>(root, "3D_perspective_multiplier", 1.f)};
			sf::Color _3dOverrideColor			{ssvuj::as<sf::Color>(root, "3D_override_color", sf::Color{0, 0, 0, 0})};
			ColorData mainColorData				{root["main"]};
			std::vector<ColorData> colorDatas;

			StyleData() = default;
			StyleData(const ssvuj::Value& mRoot) : root{mRoot}, currentHue{hueMin} { for(auto i(0u); i < ssvuj::size(root, "colors"); i++) colorDatas.emplace_back(root["colors"][i]); }

			void update(float mFrameTime, float mMult = 1.f);
			void computeColors();
			void drawBackground(sf::RenderTarget& mRenderTarget, ssvs::Vec2f mCenterPos, int mSides);

			void setRootPath(const std::string& mPath) { rootPath = mPath; }

			inline const std::string& getRootPath() const	{ return rootPath; }


			inline const sf::Color& getMainColor() const			{ return currentMainColor; }
			inline const std::vector<sf::Color>& getColors() const	{ return currentColors; }
			inline const sf::Color& getColor(int mIndex) const		{ return currentColors[mIndex]; }

			inline float getCurrentHue() const 			{ return currentHue; }
			inline float getCurrentSwapTime() const		{ return currentSwapTime; }

			inline const sf::Color& get3DOverrideColor() const	{ return current3DOverrideColor; }
	};
}

#endif
