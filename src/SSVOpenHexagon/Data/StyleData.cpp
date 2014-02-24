// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{
	Color StyleData::calculateColor(const ColorData& mColorData) const
	{
		Color color{mColorData.color};

		if(mColorData.dynamic)
		{
			const auto& dynamicColor(getColorFromHue((currentHue + mColorData.hueShift) / 360.f));

			if(!mColorData.main)
			{
				if(mColorData.dynamicOffset)
				{
					SSVU_ASSERT(mColorData.offset != 0);

					color.r += dynamicColor.r / mColorData.offset;
					color.g += dynamicColor.g / mColorData.offset;
					color.b += dynamicColor.b / mColorData.offset;
					color.a += dynamicColor.a;
				}
				else color = getColorDarkened(dynamicColor, mColorData.dynamicDarkness);
			}
			else color = dynamicColor;
		}

		const auto& pulse(mColorData.pulse);
		return Color(getClamped(color.r + pulse.r * pulseFactor, 0.f, 255.f),
					 getClamped(color.g + pulse.g * pulseFactor, 0.f, 255.f),
					 getClamped(color.b + pulse.b * pulseFactor, 0.f, 255.f),
					 getClamped(color.a + pulse.a * pulseFactor, 0.f, 255.f));
	}

	void StyleData::update(FT mFT, float mMult)
	{
		currentSwapTime += mFT * mMult;
		if(currentSwapTime > maxSwapTime) currentSwapTime = 0;

		currentHue += hueIncrement * mFT * mMult;

		if(currentHue < hueMin)
		{
			if(huePingPong) { currentHue = hueMin; hueIncrement *= -1.f; }
			else currentHue = hueMax;
		}
		if(currentHue > hueMax)
		{
			if(huePingPong) { currentHue = hueMax; hueIncrement *= -1.f; }
			else currentHue = hueMin;
		}

		pulseFactor += pulseIncrement * mFT;

		if(pulseFactor < pulseMin) { pulseIncrement *= -1.f; pulseFactor = pulseMin; }
		if(pulseFactor > pulseMax) { pulseIncrement *= -1.f; pulseFactor = pulseMax; }
	}

	void StyleData::computeColors()
	{
		currentMainColor = calculateColor(mainColorData);
		current3DOverrideColor = _3dOverrideColor.a != 0 ? _3dOverrideColor : getMainColor();
		currentColors.clear();
		for(const auto& cd : colorDatas) currentColors.emplace_back(calculateColor(cd));

		if(currentColors.size() > 1) ssvu::rotate(currentColors, begin(currentColors) + currentSwapTime / (maxSwapTime / 2.f));
	}

	void StyleData::drawBackground(RenderTarget& mRenderTarget, const Vec2f& mCenterPos, int mSides)
	{
		float div{ssvu::tau / mSides * 1.0001f}, distance{4500};

		ssvs::VertexVector<sf::PrimitiveType::Triangles> vertices;
		const auto& colors(getColors());

		for(int i{0}; i < mSides; ++i)
		{
			float angle{div * i};
			Color currentColor{ssvu::getByWrapIdx(colors, i)};

			if(Config::getBlackAndWhite()) currentColor = Color::Black;
			else if(i % 2 == 0 && i == mSides - 1) currentColor = getColorDarkened(currentColor, 1.4f);

			vertices.emplace_back(mCenterPos, currentColor);
			vertices.emplace_back(getOrbitRad(mCenterPos, angle + div * 0.5f, distance), currentColor);
			vertices.emplace_back(getOrbitRad(mCenterPos, angle - div * 0.5f, distance), currentColor);
		}

		mRenderTarget.draw(vertices);
	}
}

