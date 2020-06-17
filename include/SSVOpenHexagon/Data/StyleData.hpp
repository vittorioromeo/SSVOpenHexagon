// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"

namespace hg
{
class LevelStatus;
class StyleData
{
private:
    struct ColorData
    {
        bool main, dynamic, dynamicOffset;
        float dynamicDarkness, hueShift, offset;
        sf::Color color, pulse;

        ColorData() = default;
        ColorData(const ssvuj::Obj& mRoot)
            : main{ssvuj::getExtr<bool>(mRoot, "main", false)},
              dynamic{ssvuj::getExtr<bool>(mRoot, "dynamic", false)},
              dynamicOffset{
                  ssvuj::getExtr<bool>(mRoot, "dynamic_offset", false)},
              dynamicDarkness{
                  ssvuj::getExtr<float>(mRoot, "dynamic_darkness", 1.f)},
              hueShift{ssvuj::getExtr<float>(mRoot, "hue_shift", 0.f)},
              offset{ssvuj::getExtr<float>(mRoot, "offset", 0.f)},
              color{
                  ssvuj::getExtr<sf::Color>(mRoot, "value", sf::Color::White)},
              pulse{ssvuj::getExtr<sf::Color>(mRoot, "pulse", sf::Color::White)}
        {
        }
    };

    float currentHue, currentSwapTime{0}, pulseFactor{0};
    Path rootPath;
    sf::Color currentMainColor, current3DOverrideColor;
    std::vector<sf::Color> currentColors;

    sf::Color calculateColor(const ColorData& mColorData) const;

public:
    std::string id;
    float hueMin, hueMax, hueIncrement, pulseMin, pulseMax, pulseIncrement;
    bool huePingPong;
    float maxSwapTime, _3dDepth, _3dSkew, _3dSpacing, _3dDarkenMult,
        _3dAlphaMult, _3dAlphaFalloff, _3dPulseMax, _3dPulseMin, _3dPulseSpeed,
        _3dPerspectiveMult;
    sf::Color _3dOverrideColor;
    ColorData mainColorData;
    std::vector<ColorData> colorDatas;

    StyleData() = default;
    StyleData(const ssvuj::Obj& mRoot, const Path& mPath)
        : rootPath{mPath}, id{ssvuj::getExtr<std::string>(
                               mRoot, "id", "nullId")},
          hueMin{ssvuj::getExtr<float>(mRoot, "hue_min", 0.f)},
          hueMax{ssvuj::getExtr<float>(mRoot, "hue_max", 360.f)},
          hueIncrement{ssvuj::getExtr<float>(mRoot, "hue_increment", 0.f)},
          pulseMin{ssvuj::getExtr<float>(mRoot, "pulse_min", 0.f)},
          pulseMax{ssvuj::getExtr<float>(mRoot, "pulse_max", 0.f)},
          pulseIncrement{ssvuj::getExtr<float>(mRoot, "pulse_increment", 0.f)},
          huePingPong{ssvuj::getExtr<bool>(mRoot, "hue_ping_pong", false)},
          maxSwapTime{ssvuj::getExtr<float>(mRoot, "max_swap_time", 100.f)},
          _3dDepth{ssvuj::getExtr<float>(mRoot, "3D_depth", 15.f)},
          _3dSkew{ssvuj::getExtr<float>(mRoot, "3D_skew", 0.18f)},
          _3dSpacing{ssvuj::getExtr<float>(mRoot, "3D_spacing", 1.f)},
          _3dDarkenMult{
              ssvuj::getExtr<float>(mRoot, "3D_darken_multiplier", 1.5f)},
          _3dAlphaMult{
              ssvuj::getExtr<float>(mRoot, "3D_alpha_multiplier", 0.5f)},
          _3dAlphaFalloff{
              ssvuj::getExtr<float>(mRoot, "3D_alpha_falloff", 3.f)},
          _3dPulseMax{ssvuj::getExtr<float>(mRoot, "3D_pulse_max", 3.2f)},
          _3dPulseMin{ssvuj::getExtr<float>(mRoot, "3D_pulse_min", 0.f)},
          _3dPulseSpeed{ssvuj::getExtr<float>(mRoot, "3D_pulse_speed", 0.01f)},
          _3dPerspectiveMult{
              ssvuj::getExtr<float>(mRoot, "3D_perspective_multiplier", 1.f)},
          _3dOverrideColor{ssvuj::getExtr<sf::Color>(
              mRoot, "3D_override_color", sf::Color::Transparent)},
          mainColorData{ssvuj::getObj(mRoot, "main")}
    {
        currentHue = hueMin;

        const auto& objColors(ssvuj::getObj(mRoot, "colors"));
        const auto& colorCount(ssvuj::getObjSize(objColors));

        for(auto i(0u); i < colorCount; i++)
        {
            colorDatas.emplace_back(ssvuj::getObj(objColors, i));
        }
    }

    void update(FT mFT, float mMult = 1.f);
    void computeColors();
    void drawBackground(sf::RenderTarget& mRenderTarget,
        const Vec2f& mCenterPos, const LevelStatus& levelStatus) const;

    void setRootPath(const Path& mPath)
    {
        rootPath = mPath;
    }

    const Path& getRootPath() const
    {
        return rootPath;
    }

    const sf::Color& getMainColor() const
    {
        return currentMainColor;
    }

    const std::vector<sf::Color>& getColors() const
    {
        return currentColors;
    }

    const sf::Color& getColor(int mIdx) const
    {
        return currentColors[ssvu::getMod(mIdx, currentColors.size())];
    }

    float getCurrentHue() const
    {
        return currentHue;
    }
    float getCurrentSwapTime() const
    {
        return currentSwapTime;
    }

    const sf::Color& get3DOverrideColor() const
    {
        return current3DOverrideColor;
    }
};

} // namespace hg
