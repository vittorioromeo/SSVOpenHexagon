// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Data/ColorData.hpp"
#include "SSVOpenHexagon/Data/CapColor.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

namespace hg
{

struct LevelStatus;

class StyleData
{
private:
    float currentHue, currentSwapTime{0}, pulseFactor{0};
    ssvufs::Path rootPath;
    sf::Color currentMainColor, current3DOverrideColor;
    std::vector<sf::Color> currentColors;

    sf::Color calculateColor(const ColorData& mColorData) const;

public:
    std::string id;
    float hueMin;
    float hueMax;
    float hueIncrement;
    float pulseMin;
    float pulseMax;
    float pulseIncrement;
    bool huePingPong;
    float maxSwapTime;
    float _3dDepth;
    float _3dSkew;
    float _3dSpacing;
    float _3dDarkenMult;
    float _3dAlphaMult;
    float _3dAlphaFalloff;
    float _3dPulseMax;
    float _3dPulseMin;
    float _3dPulseSpeed;
    float _3dPerspectiveMult;
    unsigned int BGColorOffset{0};
    float BGRotOff{0};
    sf::Color _3dOverrideColor;
    ColorData mainColorData;
    CapColor capColor;

    std::vector<ColorData> colorDatas;

    StyleData() = default;
    StyleData(const ssvuj::Obj& mRoot, const ssvufs::Path& mPath)
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
          mainColorData{ssvuj::getObj(mRoot, "main")}, //
          capColor{parseCapColor(ssvuj::getObj(mRoot, "cap_color"))}
    {
        currentHue = hueMin;

        const auto& objColors(ssvuj::getObj(mRoot, "colors"));
        const auto& colorCount(ssvuj::getObjSize(objColors));

        for(auto i(0u); i < colorCount; i++)
        {
            colorDatas.emplace_back(ssvuj::getObj(objColors, i));
        }
    }

    void update(ssvu::FT mFT, float mMult = 1.f);
    void computeColors(const LevelStatus& levelStatus);
    void drawBackground(sf::RenderTarget& mRenderTarget,
        const sf::Vector2f& mCenterPos, const LevelStatus& levelStatus) const;

    void setRootPath(const ssvufs::Path& mPath)
    {
        rootPath = mPath;
    }

    const ssvufs::Path& getRootPath() const noexcept
    {
        return rootPath;
    }

    const sf::Color& getMainColor() const noexcept
    {
        return currentMainColor;
    }

    const std::vector<sf::Color>& getColors() const noexcept
    {
        return currentColors;
    }

    const sf::Color& getColor(int mIdx) const noexcept
    {
        return currentColors[ssvu::getMod(mIdx, currentColors.size())];
    }

    float getCurrentHue() const noexcept
    {
        return currentHue;
    }

    float getCurrentSwapTime() const noexcept
    {
        return currentSwapTime;
    }

    const sf::Color& get3DOverrideColor() const noexcept
    {
        return current3DOverrideColor;
    }

    CapColor& getCapColor() noexcept
    {
        return capColor;
    }

    const CapColor& getCapColor() const noexcept
    {
        return capColor;
    }

    sf::Color getCapColorResult() const noexcept;
};

} // namespace hg
