// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/StyleData.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"

#include <SSVUtils/Core/Utils/Math.hpp>

#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/Utils/SFML.hpp>

namespace hg {

[[nodiscard]] ColorData StyleData::colorDataFromObjOrDefault(
    const ssvuj::Obj& mRoot, const std::string& mKey, const ColorData& mDefault)
{
    if(ssvuj::hasObj(mRoot, mKey))
    {
        return ColorData{ssvuj::getObj(mRoot, mKey)};
    }

    return mDefault;
}

StyleData::StyleData() = default;

StyleData::StyleData(const ssvuj::Obj& mRoot)
    : id{ssvuj::getExtr<std::string>(mRoot, "id", "nullId")},
      hueMin{ssvuj::getExtr<float>(mRoot, "hue_min", 0.f)},
      hueMax{ssvuj::getExtr<float>(mRoot, "hue_max", 360.f)},
      hueIncrement{ssvuj::getExtr<float>(mRoot, "hue_increment", 0.f)},
      huePingPong{ssvuj::getExtr<bool>(mRoot, "hue_ping_pong", false)},

      pulseMin{ssvuj::getExtr<float>(mRoot, "pulse_min", 0.f)},
      pulseMax{ssvuj::getExtr<float>(mRoot, "pulse_max", 0.f)},
      pulseIncrement{ssvuj::getExtr<float>(mRoot, "pulse_increment", 0.f)},
      maxSwapTime{ssvuj::getExtr<float>(mRoot, "max_swap_time", 100.f)},

      _3dDepth{ssvuj::getExtr<float>(mRoot, "3D_depth", 15.f)},
      _3dSkew{ssvuj::getExtr<float>(mRoot, "3D_skew", 0.18f)},
      _3dSpacing{ssvuj::getExtr<float>(mRoot, "3D_spacing", 1.f)},
      _3dDarkenMult{ssvuj::getExtr<float>(mRoot, "3D_darken_multiplier", 1.5f)},
      _3dAlphaMult{ssvuj::getExtr<float>(mRoot, "3D_alpha_multiplier", 0.5f)},
      _3dAlphaFalloff{ssvuj::getExtr<float>(mRoot, "3D_alpha_falloff", 3.f)},
      _3dPulseMax{ssvuj::getExtr<float>(mRoot, "3D_pulse_max", 3.2f)},
      _3dPulseMin{ssvuj::getExtr<float>(mRoot, "3D_pulse_min", 0.f)},
      _3dPulseSpeed{ssvuj::getExtr<float>(mRoot, "3D_pulse_speed", 0.01f)},
      _3dPerspectiveMult{
          ssvuj::getExtr<float>(mRoot, "3D_perspective_multiplier", 1.f)},
      _3dOverrideColor{ssvuj::getExtr<sf::Color>(
          mRoot, "3D_override_color", sf::Color::Transparent)},
      mainColorData{ssvuj::getObj(mRoot, "main")},                          //
      playerColor{
          colorDataFromObjOrDefault(mRoot, "player_color", mainColorData)}, //
      textColor{
          colorDataFromObjOrDefault(mRoot, "text_color", mainColorData)},   //
      wallColor{colorDataFromObjOrDefault(mRoot, "wall_color", mainColorData)},
      capColor{parseCapColor(ssvuj::getObj(mRoot, "cap_color"))}
{
    currentHue = hueMin;

    const auto& objColors(ssvuj::getObj(mRoot, "colors"));
    const auto& colorCount(ssvuj::getObjSize(objColors));

    colorDatas.reserve(colorCount);
    for(auto i(0u); i < colorCount; i++)
    {
        colorDatas.emplace_back(ssvuj::getObj(objColors, i));
    }
}

sf::Color StyleData::calculateColor(const float mCurrentHue,
    const float mPulseFactor, const ColorData& mColorData)
{
    sf::Color color{mColorData.color};

    if(mColorData.dynamic)
    {
        const float hue =
            std::fmod(mCurrentHue + mColorData.hueShift, 360.f) / 360.f;

        const sf::Color dynamicColor = Utils::getColorFromHue(hue);

        if(!mColorData.main)
        {
            if(mColorData.dynamicOffset)
            {
                if(mColorData.offset != 0)
                {
                    color.r += dynamicColor.r / mColorData.offset;
                    color.g += dynamicColor.g / mColorData.offset;
                    color.b += dynamicColor.b / mColorData.offset;
                }

                color.a += dynamicColor.a;
            }
            else
            {
                color = Utils::getColorDarkened(
                    dynamicColor, mColorData.dynamicDarkness);
            }
        }
        else
        {
            color = dynamicColor;
        }
    }

    return sf::Color( //
        Utils::componentClamp(color.r + mColorData.pulse.r * mPulseFactor),
        Utils::componentClamp(color.g + mColorData.pulse.g * mPulseFactor),
        Utils::componentClamp(color.b + mColorData.pulse.b * mPulseFactor),
        Utils::componentClamp(color.a + mColorData.pulse.a * mPulseFactor));
}

void StyleData::update(ssvu::FT mFT, float mMult)
{
    currentSwapTime += mFT * mMult;
    if(currentSwapTime > maxSwapTime)
    {
        currentSwapTime = 0;
    }

    currentHue += hueIncrement * mFT * mMult;

    if(currentHue < hueMin)
    {
        if(huePingPong)
        {
            currentHue = hueMin;
            hueIncrement *= -1.f;
        }
        else
        {
            currentHue = hueMax;
        }
    }
    else if(currentHue > hueMax)
    {
        if(huePingPong)
        {
            currentHue = hueMax;
            hueIncrement *= -1.f;
        }
        else
        {
            currentHue = hueMin;
        }
    }

    pulseFactor += pulseIncrement * mFT;

    if(pulseFactor < pulseMin)
    {
        pulseIncrement *= -1.f;
        pulseFactor = pulseMin;
    }
    else if(pulseFactor > pulseMax)
    {
        pulseIncrement *= -1.f;
        pulseFactor = pulseMax;
    }
}

void StyleData::computeColors()
{
    currentMainColor = calculateColor(currentHue, pulseFactor, mainColorData);
    currentPlayerColor = calculateColor(currentHue, pulseFactor, playerColor);
    currentTextColor = calculateColor(currentHue, pulseFactor, textColor);
    currentWallColor = calculateColor(currentHue, pulseFactor, wallColor);

    current3DOverrideColor =
        _3dOverrideColor.a != 0 ? _3dOverrideColor : getMainColor();

    currentColors.clear();

    for(const ColorData& cd : colorDatas)
    {
        currentColors.emplace_back(calculateColor(currentHue, pulseFactor, cd));
    }

    if(currentColors.size() > 1)
    {
        const unsigned int rotation = currentSwapTime / (maxSwapTime / 2.f);

        ssvu::rotate(currentColors,
            std::begin(currentColors) +
                ssvu::getMod(rotation + BGColorOffset, currentColors.size()));
    }
}

void StyleData::drawBackgroundImpl(Utils::FastVertexVectorTris& vertices,
    const sf::Vector2f& mCenterPos, const unsigned int sides,
    const bool darkenUnevenBackgroundChunk, const bool blackAndWhite) const
{
    const float div{ssvu::tau / sides * 1.0001f};
    const float halfDiv{div / 2.f};
    const float distance{bgTileRadius};

    const std::vector<sf::Color>& colors(getColors());
    if(colors.empty())
    {
        return;
    }

    for(auto i(0u); i < sides; ++i)
    {
        const float angle{ssvu::toRad(BGRotOff) + div * i};
        sf::Color currentColor{ssvu::getByModIdx(colors, i)};

        const bool mustDarkenUnevenBackgroundChunk =
            (i % 2 == 0 && i == sides - 1) && darkenUnevenBackgroundChunk;

        if(blackAndWhite)
        {
            currentColor = sf::Color::Black;
        }
        else if(mustDarkenUnevenBackgroundChunk)
        {
            currentColor = Utils::getColorDarkened(currentColor, 1.4f);
        }

        vertices.batch_unsafe_emplace_back(currentColor, mCenterPos,
            ssvs::getOrbitRad(mCenterPos, angle + halfDiv, distance),
            ssvs::getOrbitRad(mCenterPos, angle - halfDiv, distance));
    }
}

void StyleData::drawBackgroundMenuHexagonImpl(
    Utils::FastVertexVectorTris& vertices, const sf::Vector2f& mCenterPos,
    const unsigned int sides, const bool fourByThree,
    const bool blackAndWhite) const
{
    const float div{ssvu::tau / sides * 1.0001f};
    const float halfDiv{div / 2.f};
    const float hexagonRadius{fourByThree ? 75.f : 100.f};

    const sf::Color& colorMain{
        blackAndWhite ? sf::Color::White : getMainColor()};
    const sf::Color colorCap{
        blackAndWhite ? sf::Color::Black : getCapColorResult()};

    for(auto i(0u); i < sides; ++i)
    {
        const float angle{ssvu::toRad(BGRotOff) + div * i};

        vertices.batch_unsafe_emplace_back(colorMain, mCenterPos,
            ssvs::getOrbitRad(
                mCenterPos, angle + halfDiv, hexagonRadius + 10.f),
            ssvs::getOrbitRad(
                mCenterPos, angle - halfDiv, hexagonRadius + 10.f));

        vertices.batch_unsafe_emplace_back(colorCap, mCenterPos,
            ssvs::getOrbitRad(mCenterPos, angle + halfDiv, hexagonRadius),
            ssvs::getOrbitRad(mCenterPos, angle - halfDiv, hexagonRadius));
    }
}

void StyleData::drawBackground(Utils::FastVertexVectorTris& mTris,
    const sf::Vector2f& mCenterPos, const unsigned int sides,
    const bool darkenUnevenBackgroundChunk, const bool blackAndWhite) const
{
    mTris.reserve_more(sides * 3);

    drawBackgroundImpl(
        mTris, mCenterPos, sides, darkenUnevenBackgroundChunk, blackAndWhite);
}

void StyleData::drawBackgroundMenu(Utils::FastVertexVectorTris& mTris,
    const sf::Vector2f& mCenterPos, const unsigned int sides,
    const bool darkenUnevenBackgroundChunk, const bool blackAndWhite,
    const bool fourByThree) const
{
    mTris.reserve_more(sides * 3 + sides * 6);

    drawBackgroundImpl(
        mTris, mCenterPos, sides, darkenUnevenBackgroundChunk, blackAndWhite);
    drawBackgroundMenuHexagonImpl(
        mTris, mCenterPos, sides, fourByThree, blackAndWhite);
}

void StyleData::setCapColor(const CapColor& mCapColor)
{
    capColor = mCapColor;
}

[[nodiscard]] const sf::Color& StyleData::getMainColor() const noexcept
{
    return currentMainColor;
}

[[nodiscard]] const sf::Color& StyleData::getPlayerColor() const noexcept
{
    return currentPlayerColor;
}

[[nodiscard]] const sf::Color& StyleData::getTextColor() const noexcept
{
    return currentTextColor;
}

[[nodiscard]] const sf::Color& StyleData::getWallColor() const noexcept
{
    return currentWallColor;
}

[[nodiscard]] const std::vector<sf::Color>&
StyleData::getColors() const noexcept
{
    return currentColors;
}

[[nodiscard]] const sf::Color& StyleData::getColor(
    const std::size_t mIdx) const noexcept
{
    SSVOH_ASSERT(!currentColors.empty());
    return ssvu::getByModIdx(currentColors, mIdx);
}

[[nodiscard]] float StyleData::getCurrentHue() const noexcept
{
    return currentHue;
}

[[nodiscard]] float StyleData::getCurrentSwapTime() const noexcept
{
    return currentSwapTime;
}

[[nodiscard]] const sf::Color& StyleData::get3DOverrideColor() const noexcept
{
    return current3DOverrideColor;
}

sf::Color StyleData::getCapColorResult() const noexcept
{
    return capColor.linear_match(                                       //
        [this](CapColorMode::Main) { return getMainColor(); },          //
        [this](CapColorMode::MainDarkened)
        { return Utils::getColorDarkened(getMainColor(), 1.4f); },      //
        [this](CapColorMode::ByIndex x) { return getColor(x._index); }, //
        [this](const ColorData& data)
        { return calculateColor(currentHue, pulseFactor, data); });
}

} // namespace hg
