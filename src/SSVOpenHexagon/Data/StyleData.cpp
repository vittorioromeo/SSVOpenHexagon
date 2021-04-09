// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/StyleData.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

#include <SSVUtils/Core/Utils/Math.hpp>

#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/Utils/SFML.hpp>

namespace hg
{

sf::Color StyleData::calculateColor(const ColorData& mColorData) const
{
    sf::Color color{mColorData.color};

    if(mColorData.dynamic)
    {
        const float hue =
            std::fmod(currentHue + mColorData.hueShift, 360.f) / 360.f;

        const sf::Color dynamicColor = ssvs::getColorFromHSV(hue, 1.f, 1.f);

        if(!mColorData.main)
        {
            if(mColorData.dynamicOffset)
            {
                SSVOH_ASSERT(mColorData.offset != 0);

                color.r += dynamicColor.r / mColorData.offset;
                color.g += dynamicColor.g / mColorData.offset;
                color.b += dynamicColor.b / mColorData.offset;
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

    const auto componentClamp = [](const float value) -> sf::Uint8 {
        if(value > 255.f)
        {
            return sf::Uint8(255);
        }

        if(value < 0)
        {
            return sf::Uint8(0);
        }

        return static_cast<sf::Uint8>(value);
    };

    return sf::Color( //
        componentClamp(color.r + mColorData.pulse.r * pulseFactor),
        componentClamp(color.g + mColorData.pulse.g * pulseFactor),
        componentClamp(color.b + mColorData.pulse.b * pulseFactor),
        componentClamp(color.a + mColorData.pulse.a * pulseFactor));
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
    currentMainColor = calculateColor(mainColorData);
    currentPlayerColor = calculateColor(playerColor);
    currentTextColor = calculateColor(textColor);

    current3DOverrideColor =
        _3dOverrideColor.a != 0 ? _3dOverrideColor : getMainColor();

    currentColors.clear();

    for(const ColorData& cd : colorDatas)
    {
        currentColors.emplace_back(calculateColor(cd));
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
    const bool darkenUnevenBackgroundChunk) const
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

        // TODO (P2): remove dependency on config
        const bool mustDarkenUnevenBackgroundChunk =
            (i % 2 == 0 && i == sides - 1) &&
            Config::getDarkenUnevenBackgroundChunk() &&
            darkenUnevenBackgroundChunk;

        // TODO (P2): remove dependency on config
        if(Config::getBlackAndWhite())
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
    const unsigned int sides, const bool fourByThree) const
{
    const float div{ssvu::tau / sides * 1.0001f};
    const float halfDiv{div / 2.f};
    const float hexagonRadius{fourByThree ? 75.f : 100.f};

    const sf::Color& colorMain{getMainColor()};
    const sf::Color colorCap{getCapColorResult()};

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
    const bool darkenUnevenBackgroundChunk) const
{
    mTris.reserve_more(sides * 3);

    drawBackgroundImpl(mTris, mCenterPos, sides, darkenUnevenBackgroundChunk);
}

void StyleData::drawBackgroundMenu(Utils::FastVertexVectorTris& mTris,
    const sf::Vector2f& mCenterPos, const unsigned int sides,
    const bool darkenUnevenBackgroundChunk, const bool fourByThree) const
{
    mTris.reserve_more(sides * 3 + sides * 6);

    drawBackgroundImpl(mTris, mCenterPos, sides, darkenUnevenBackgroundChunk);
    drawBackgroundMenuHexagonImpl(mTris, mCenterPos, sides, fourByThree);
}

sf::Color StyleData::getCapColorResult() const noexcept
{
    return Utils::match(
        capColor,                                              //
        [this](CapColorMode::Main) { return getMainColor(); }, //
        [this](CapColorMode::MainDarkened) {
            return Utils::getColorDarkened(getMainColor(), 1.4f);
        },                                                              //
        [this](CapColorMode::ByIndex x) { return getColor(x._index); }, //
        [this](ColorData data) { return calculateColor(data); });
}

} // namespace hg
