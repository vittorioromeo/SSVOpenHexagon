// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Data/ColorData.hpp"
#include "SSVOpenHexagon/Data/CapColor.hpp"

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>
#include <string>

namespace Json {

class Value;

}

namespace ssvuj {

using Obj = Json::Value;

}

namespace hg::Utils {

class FastVertexVectorTris;
class FastVertexVectorTris;

} // namespace hg::Utils

namespace hg {

class StyleData
{
private:
    float currentHue{0};
    float currentSwapTime{0};
    float pulseFactor{0};
    sf::Color currentMainColor{sf::Color::Black};
    sf::Color currentPlayerColor{sf::Color::Black};
    sf::Color currentTextColor{sf::Color::Black};
    sf::Color currentWallColor{sf::Color::White};
    sf::Color current3DOverrideColor{sf::Color::Black};
    std::vector<sf::Color> currentColors;

    [[nodiscard]] static sf::Color calculateColor(const float mCurrentHue,
        const float mPulseFactor, const ColorData& mColorData);

    [[nodiscard]] static ColorData colorDataFromObjOrDefault(
        const ssvuj::Obj& mRoot, const std::string& mKey,
        const ColorData& mDefault);

    void drawBackgroundImpl(Utils::FastVertexVectorTris& vertices,
        const sf::Vector2f& mCenterPos, const unsigned int sides,
        const bool darkenUnevenBackgroundChunk, const bool blackAndWhite) const;

    void drawBackgroundMenuHexagonImpl(Utils::FastVertexVectorTris& vertices,
        const sf::Vector2f& mCenterPos, const unsigned int sides,
        const bool fourByThree, const bool blackAndWhite) const;

public:
    std::string id{};
    float hueMin{};
    float hueMax{};
    float hueIncrement{};
    bool huePingPong{};

    float pulseMin{};
    float pulseMax{};
    float pulseIncrement{};
    float maxSwapTime{};

    float _3dDepth{};
    float _3dSkew{};
    float _3dSpacing{};
    float _3dDarkenMult{};
    float _3dAlphaMult{};
    float _3dAlphaFalloff{};
    float _3dPulseMax{};
    float _3dPulseMin{};
    float _3dPulseSpeed{};
    float _3dPerspectiveMult{};

    float bgTileRadius{10000.f};
    unsigned int BGColorOffset{0};
    float BGRotOff{0}; // In degrees

private:
    sf::Color _3dOverrideColor;
    ColorData mainColorData;
    ColorData playerColor;
    ColorData textColor;
    ColorData wallColor;

    CapColor capColor;

    std::vector<ColorData> colorDatas;

public:
    explicit StyleData();
    explicit StyleData(const ssvuj::Obj& mRoot);

    void update(ssvu::FT mFT, float mMult = 1.f);

    void computeColors();

    void drawBackgroundMenu(Utils::FastVertexVectorTris& mTris,
        const sf::Vector2f& mCenterPos, const unsigned int sides,
        const bool darkenUnevenBackgroundChunk, const bool blackAndWhite,
        const bool fourByThree) const;

    void drawBackground(Utils::FastVertexVectorTris& mTris,
        const sf::Vector2f& mCenterPos, const unsigned int sides,
        const bool darkenUnevenBackgroundChunk, const bool blackAndWhite) const;

    void setCapColor(const CapColor& mCapColor);

    [[nodiscard]] const sf::Color& getMainColor() const noexcept;
    [[nodiscard]] const sf::Color& getPlayerColor() const noexcept;
    [[nodiscard]] const sf::Color& getTextColor() const noexcept;
    [[nodiscard]] const sf::Color& getWallColor() const noexcept;
    [[nodiscard]] const std::vector<sf::Color>& getColors() const noexcept;
    [[nodiscard]] const sf::Color& getColor(
        const std::size_t mIdx) const noexcept;
    [[nodiscard]] float getCurrentHue() const noexcept;
    [[nodiscard]] float getCurrentSwapTime() const noexcept;
    [[nodiscard]] const sf::Color& get3DOverrideColor() const noexcept;
    [[nodiscard]] sf::Color getCapColorResult() const noexcept;
};

} // namespace hg
