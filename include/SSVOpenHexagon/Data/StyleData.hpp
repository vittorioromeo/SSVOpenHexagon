// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_STYLEDATA
#define HG_STYLEDATA

#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{
	class StyleData
	{
		private:
			struct ColorData
			{
				ssvuj::Obj root;
				ColorData(const ssvuj::Obj& mRoot) : root{mRoot} { }

				bool main					{ssvuj::getExtr<bool>(root, "main", false)};
				bool dynamic				{ssvuj::getExtr<bool>(root, "dynamic", false)};
				bool dynamicOffset			{ssvuj::getExtr<bool>(root, "dynamic_offset", false)};
				float dynamicDarkness		{ssvuj::getExtr<float>(root, "dynamic_darkness", 1.f)};
				float hueShift				{ssvuj::getExtr<float>(root, "hue_shift", 0.f)};
				float offset				{ssvuj::getExtr<float>(root, "offset", 0.f)};
				sf::Color color				{ssvuj::getExtr<sf::Color>(root, "value", sf::Color::White)};
				sf::Color pulse				{ssvuj::getExtr<sf::Color>(root, "pulse", sf::Color::White)};
			};

			ssvuj::Obj root;
			float currentHue, currentSwapTime{0}, pulseFactor{0};
			Path rootPath;
			sf::Color currentMainColor, current3DOverrideColor;
			std::vector<sf::Color> currentColors;

			sf::Color calculateColor(const ColorData& mColorData) const;

		public:
			std::string id						{ssvuj::getExtr<std::string>(root, "id", "nullId")};
			float hueMin						{ssvuj::getExtr<float>(root, "hue_min", 0.f)};
			float hueMax						{ssvuj::getExtr<float>(root, "hue_max", 360.f)};
			float hueIncrement					{ssvuj::getExtr<float>(root, "hue_increment", 0.f)};
			float pulseMin						{ssvuj::getExtr<float>(root, "pulse_min", 0.f)};
			float pulseMax						{ssvuj::getExtr<float>(root, "pulse_max", 0.f)};
			float pulseIncrement				{ssvuj::getExtr<float>(root, "pulse_increment", 0.f)};
			bool huePingPong					{ssvuj::getExtr<bool>(root, "hue_ping_pong", false)};
			float maxSwapTime					{ssvuj::getExtr<float>(root, "max_swap_time", 100.f)};
			float _3dDepth						{ssvuj::getExtr<float>(root, "3D_depth", 15.f)};
			float _3dSkew						{ssvuj::getExtr<float>(root, "3D_skew", 0.18f)};
			float _3dSpacing					{ssvuj::getExtr<float>(root, "3D_spacing", 1.f)};
			float _3dDarkenMult					{ssvuj::getExtr<float>(root, "3D_darken_multiplier", 1.5f)};
			float _3dAlphaMult					{ssvuj::getExtr<float>(root, "3D_alpha_multiplier", 0.5f)};
			float _3dAlphaFalloff				{ssvuj::getExtr<float>(root, "3D_alpha_falloff", 3.f)};
			float _3dPulseMax					{ssvuj::getExtr<float>(root, "3D_pulse_max", 3.2f)};
			float _3dPulseMin					{ssvuj::getExtr<float>(root, "3D_pulse_min", 0.f)};
			float _3dPulseSpeed					{ssvuj::getExtr<float>(root, "3D_pulse_speed", 0.01f)};
			float _3dPerspectiveMult			{ssvuj::getExtr<float>(root, "3D_perspective_multiplier", 1.f)};
			sf::Color _3dOverrideColor			{ssvuj::getExtr<sf::Color>(root, "3D_override_color", sf::Color::Transparent)};
			ColorData mainColorData				{root["main"]};
			std::vector<ColorData> colorDatas;

			StyleData() = default;
			StyleData(const ssvuj::Obj& mRoot, const Path& mPath) : root{mRoot}, rootPath{mPath}
			{
				currentHue = hueMin;
				for(auto i(0u); i < ssvuj::getObjSize(root, "colors"); i++) colorDatas.emplace_back(root["colors"][i]);
			}

			void update(FT mFT, float mMult = 1.f);
			void computeColors();
			void drawBackground(sf::RenderTarget& mRenderTarget, const Vec2f& mCenterPos, int mSides);

			void setRootPath(const Path& mPath) { rootPath = mPath; }

			inline const Path& getRootPath() const { return rootPath; }

			inline const sf::Color& getMainColor() const			{ return currentMainColor; }
			inline const std::vector<sf::Color>& getColors() const	{ return currentColors; }
			inline const sf::Color& getColor(int mIdx) const		{ return currentColors[ssvu::getWrapIdx(mIdx, currentColors.size())]; }

			inline float getCurrentHue() const 			{ return currentHue; }
			inline float getCurrentSwapTime() const		{ return currentSwapTime; }

			inline const sf::Color& get3DOverrideColor() const	{ return current3DOverrideColor; }
	};
}

#endif
