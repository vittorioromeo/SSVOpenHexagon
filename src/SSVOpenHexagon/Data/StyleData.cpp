// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Data/StyleData.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Global/Config.h"

using namespace std;
using namespace sf;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{
	Color StyleData::calculateColor(const Json::Value& mColorRoot) const
	{
		Color color{getColorFromJsonArray(mColorRoot["value"])};

		if(as<bool>(mColorRoot, "dynamic"))
		{
			Color dynamicColor{getColorFromHue((currentHue + as<float>(mColorRoot, "hue_shift")) / 360.0f)};

			if(as<bool>(mColorRoot, "main")) color = dynamicColor;
			else
			{
				if(!as<bool>(mColorRoot, "dynamic_offset")) color = getColorDarkened(dynamicColor, as<float>(mColorRoot, "dynamic_darkness"));
				else
				{
					color.r += dynamicColor.r / as<float>(mColorRoot, "offset");
					color.g += dynamicColor.g / as<float>(mColorRoot, "offset");
					color.b += dynamicColor.b / as<float>(mColorRoot, "offset");
					color.a += dynamicColor.a;
				}
			}
		}

		Color pulse{getColorFromJsonArray(mColorRoot["pulse"])};
		return Color(getClamped(color.r + pulse.r * pulseFactor, 0.f, 255.f),
					 getClamped(color.g + pulse.g * pulseFactor, 0.f, 255.f),
					 getClamped(color.b + pulse.b * pulseFactor, 0.f, 255.f),
					 getClamped(color.a + pulse.a * pulseFactor, 0.f, 255.f));
	}

	StyleData::StyleData(const Json::Value& mRoot) : root{mRoot}, currentHue{getHueMin()} { }

	void StyleData::update(float mFrameTime)
	{
		currentSwapTime += mFrameTime;
		if(currentSwapTime > getMaxSwapTime()) currentSwapTime = 0;

		currentHue += getHueIncrement() * mFrameTime;

		if(currentHue < getHueMin())
		{
			if(getHuePingPong()) { currentHue = getHueMin(); root["hue_increment"] = getHueIncrement() * -1; }
			else currentHue = getHueMax();
		}
		if(currentHue > getHueMax())
		{
			if(getHuePingPong()) { currentHue = getHueMax(); root["hue_increment"] = getHueIncrement() * -1; }
			else currentHue = getHueMin();
		}

		pulseFactor += as<float>(root, "pulse_increment") * mFrameTime;

		if(pulseFactor < as<float>(root, "pulse_min")) { root["pulse_increment"] = as<float>(root, "pulse_increment") * -1; pulseFactor = as<float>(root, "pulse_min"); }
		if(pulseFactor > as<float>(root, "pulse_max")) { root["pulse_increment"] = as<float>(root, "pulse_increment") * -1; pulseFactor = as<float>(root, "pulse_max"); }
	}

	void StyleData::computeColors()
	{
		currentMainColor = calculateColor(root["main"]);
		current3DOverrideColor = has(root, "3D_override_color") ? getColorFromJsonArray(root["3D_override_color"]) : getMainColor();
		currentColors.clear();
		for(unsigned int i{0}; i < size(root, "colors"); i++) currentColors.push_back(calculateColor(root["colors"][i]));
		rotate(currentColors.begin(), currentColors.begin() + currentSwapTime / (getMaxSwapTime() / 2), currentColors.end());
	}

	void StyleData::setRootPath(const std::string& mPath) { rootPath = mPath; }
	string StyleData::getRootPath() const { return rootPath; }

	string StyleData::getId() const					{ return as<string>(root, "id"); }
	float StyleData::getHueMin() const				{ return as<float>(root, "hue_min"); }
	float StyleData::getHueMax() const				{ return as<float>(root, "hue_max"); }
	bool StyleData::getHuePingPong() const			{ return as<bool>(root, "hue_ping_pong"); }
	float StyleData::getHueIncrement() const		{ return as<float>(root, "hue_increment"); }
	float StyleData::getMaxSwapTime() const			{ return as<float>(root, "max_swap_time", 100.f); }

	float StyleData::getCurrentHue() const 			{ return currentHue; }
	float StyleData::getCurrentSwapTime() const		{ return currentSwapTime; }
	Color StyleData::getMainColor() const			{ return currentMainColor; }
	vector<Color> StyleData::getColors() const		{ return currentColors; }

	void StyleData::setValueFloat(const string& mValueName, float mValue)			{ ssvuj::set(root, mValueName, mValue); }
	float StyleData::getValueFloat(const string& mValueName) const					{ return as<float>(root, mValueName); }
	void StyleData::setValueInt(const string& mValueName, int mValue)				{ ssvuj::set(root, mValueName, mValue); }
	int StyleData::getValueInt(const string& mValueName) const						{ return as<int>(root, mValueName); }
	void StyleData::setValueString(const string& mValueName, const string& mValue)	{ ssvuj::set(root, mValueName, mValue); }
	string StyleData::getValueString(const string& mValueName) const				{ return as<string>(root, mValueName); }
	void StyleData::setValueBool(const string& mValueName, bool mValue)				{ ssvuj::set(root, mValueName, mValue); }
	bool StyleData::getValueBool(const string& mValueName) const					{ return as<bool>(root, mValueName); }

	void StyleData::drawBackground(RenderTarget& mRenderTarget, Vector2f mCenterPos, int mSides)
	{
		float div{360.f / mSides * 1.0001f}, distance{4500};

		VertexArray vertices{PrimitiveType::Triangles, 3};
		vector<Color> colors{getColors()};

		for(int i{0}; i < mSides; ++i)
		{
			float angle{div * i};
			Color currentColor{colors[i % colors.size()]};
			if(getBlackAndWhite()) currentColor = Color::Black;

			if(i % 2 == 0) if(i == mSides - 1) currentColor = getColorDarkened(currentColor, 1.4f);

			vertices.append(Vertex{mCenterPos, currentColor});
			vertices.append(Vertex{getOrbitFromDegrees(mCenterPos, angle + div * 0.5f, distance), currentColor});
			vertices.append(Vertex{getOrbitFromDegrees(mCenterPos, angle - div * 0.5f, distance), currentColor});
		}

		mRenderTarget.draw(vertices);
	}

	unsigned int StyleData::get3DDepth() const			{ return as<float>(root, "3D_depth", 15); }
	float StyleData::get3DSkew() const					{ return as<float>(root, "3D_skew", 0.18f); }
	float StyleData::get3DSpacing() const				{ return as<float>(root, "3D_spacing", 1.0f); }
	float StyleData::get3DDarkenMultiplier() const		{ return as<float>(root, "3D_darken_multiplier", 1.5f); }
	float StyleData::get3DAlphaMultiplier() const		{ return as<float>(root, "3D_alpha_multiplier", 0.5f); }
	float StyleData::get3DAlphaFalloff() const			{ return as<float>(root, "3D_alpha_falloff", 3.0f); }
	Color StyleData::get3DOverrideColor() const			{ return current3DOverrideColor; }
	float StyleData::get3DPulseMax() const				{ return as<float>(root, "3D_pulse_max", 3.2f); }
	float StyleData::get3DPulseMin() const				{ return as<float>(root, "3D_pulse_min", -0.0f); }
	float StyleData::get3DPulseSpeed() const			{ return as<float>(root, "3D_pulse_speed", 0.01f); }
	float StyleData::get3DPerspectiveMultiplier() const	{ return as<float>(root, "3D_perspective_multiplier", 1.0f); }
}

