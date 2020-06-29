// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{

sf::Color StyleData::calculateColor(const ColorData& mColorData) const
{
    sf::Color color{mColorData.color};

    if(mColorData.dynamic)
    {
        const auto hue = (currentHue + mColorData.hueShift) / 360.f;

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

void StyleData::update(FT mFT, HexagonGameStatus& status, float mMult)
{
    skew = {1.f, 1.f + _3dSkew * Config::get3DMultiplier() * status.pulse3D};

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

void StyleData::computeColors(LevelStatus& levelStatus)
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
        ssvu::rotate(currentColors, begin(currentColors) + currentSwapTime /
                    (maxSwapTime / 2.f) + colorPosOffset % levelStatus.sides);
    }
}

void StyleData::drawBackground(RenderTarget& mRenderTarget,const sf::Vector2f& mCenterPos,
    LevelStatus& levelStatus, const StyleData& styleData) const
{
    const auto sides = levelStatus.sides;

    float div{ssvu::tau / sides * 1.0001f};
    float distance{styleData.BGTileRadius};

    static Utils::FastVertexVector<sf::PrimitiveType::Triangles> vertices;

    vertices.clear();
    vertices.reserve(sides * 3);

    const auto& colors(getColors());

    auto fieldAngle = ssvu::toRad(styleData.BGRotOff+levelStatus.rotation);

    for(auto i(0u); i < sides; ++i)
    {
        const float angle{fieldAngle + div * i};
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

        const sf::Vector2 pos2{Utils::getSkewedOrbitRad(mCenterPos, angle + div * 0.5f, distance, styleData.skew)};
        const sf::Vector2 pos3{Utils::getSkewedOrbitRad(mCenterPos, angle - div * 0.5f, distance, styleData.skew)};
        vertices.batch_unsafe_emplace_back(
            currentColor,
            mCenterPos,
            pos2,
            pos3);
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
