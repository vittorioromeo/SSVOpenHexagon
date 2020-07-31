// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

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
        const auto hue = std::fmod(currentHue + mColorData.hueShift, 360.f) / 360.f;

        const auto& dynamicColor(
            ssvs::getColorFromHSV(ssvu::getClamped(hue, 0.f, 1.f), 1.f, 1.f));

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

    const auto& pulse(mColorData.pulse);
    return sf::Color(ssvu::toNum<sf::Uint8>(ssvu::getClamped(
                         color.r + pulse.r * pulseFactor, 0.f, 255.f)),
        ssvu::toNum<sf::Uint8>(
            ssvu::getClamped(color.g + pulse.g * pulseFactor, 0.f, 255.f)),
        ssvu::toNum<sf::Uint8>(
            ssvu::getClamped(color.b + pulse.b * pulseFactor, 0.f, 255.f)),
        ssvu::toNum<sf::Uint8>(
            ssvu::getClamped(color.a + pulse.a * pulseFactor, 0.f, 255.f)));
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

    if(currentHue > hueMax)
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
    if(pulseFactor > pulseMax)
    {
        pulseIncrement *= -1.f;
        pulseFactor = pulseMax;
    }
}

void StyleData::computeColors(const LevelStatus& levelStatus)
{
    currentMainColor = calculateColor(mainColorData);

    current3DOverrideColor =
        _3dOverrideColor.a != 0 ? _3dOverrideColor : getMainColor();

    currentColors.clear();

    for(const auto& cd : colorDatas)
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

void StyleData::drawBackground(sf::RenderTarget& mRenderTarget,
    const sf::Vector2f& mCenterPos, const LevelStatus& levelStatus) const
{
    const auto sides = levelStatus.sides;

    float div{ssvu::tau / sides * 1.0001f};
    float distance{bgTileRadius};

    static Utils::FastVertexVector<sf::PrimitiveType::Triangles> vertices;

    vertices.clear();
    vertices.reserve(sides * 3);

    const auto& colors(getColors());

    for(auto i(0u); i < sides; ++i)
    {
        const float angle{ssvu::toRad(BGRotOff) + div * i};
        sf::Color currentColor{ssvu::getByModIdx(colors, i)};

        const bool darkenUnevenBackgroundChunk =
            (i % 2 == 0 && i == sides - 1) &&
            Config::getDarkenUnevenBackgroundChunk() &&
            levelStatus.darkenUnevenBackgroundChunk;

        if(Config::getBlackAndWhite())
        {
            currentColor = sf::Color::Black;
        }
        else if(darkenUnevenBackgroundChunk)
        {
            currentColor = Utils::getColorDarkened(currentColor, 1.4f);
        }

        vertices.batch_unsafe_emplace_back(currentColor, mCenterPos,
            ssvs::getOrbitRad(mCenterPos, angle + div * 0.5f, distance),
            ssvs::getOrbitRad(mCenterPos, angle - div * 0.5f, distance));
    }

    mRenderTarget.draw(vertices);
}

sf::Color StyleData::getCapColorResult() const noexcept
{
    return Utils::match(
        capColor,                                              //
        [this](CapColorMode::Main) { return getMainColor(); }, //
        [this](CapColorMode::MainDarkened) {
            return Utils::getColorDarkened(getMainColor(), 1.4f);
        },                                                             //
        [this](CapColorMode::ByIndex x) { return getColor(x.index); }, //
        [this](ColorData data) { return calculateColor(data); });
}

} // namespace hg
