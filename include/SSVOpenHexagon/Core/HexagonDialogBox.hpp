// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SSVStart/GameSystem/GameSystem.hpp>

#include <string>

namespace hg
{

class HGAssets;
class StyleData;

enum class DBoxDraw
{
    topLeft = 0,
    center,
    centerUpperHalf
};

class HexagonDialogBox
{
private:
    using KKey = sf::Keyboard::Key;
    using Color = sf::Color;
    using DrawFunc = std::function<void(const Color&, const Color&)>;

    HGAssets& assets;
    ssvs::GameWindow& window;
    StyleData& styleData;

    DrawFunc drawFunc;

    Utils::FastVertexVector<sf::PrimitiveType::Quads> dialogFrame;
    std::vector<std::string> dialogText;
    sf::Font& imagine;
    sf::Text txtDialog;

    float dialogWidth{0.f};
    float frameSize{0.f};
    float doubleFrameSize{0.f};
    float lineHeight{0.f};
    float totalHeight{0.f};

    float xPos{0.f};
    float yPos{0.f};

    KKey keyToClose{KKey::Unknown};

    [[nodiscard]] DrawFunc drawModeToDrawFunc(DBoxDraw drawMode);

    void drawText(
        const Color& txtColor, const float xOffset, const float yOffset);
    void drawBox(const Color& frameColor, const float x1, const float x2,
        const float y1, const float y2);
    void drawCenter(const Color& txtColor, const Color& backdropColor);
    void drawCenterUpperHalf(const Color& txtColor, const Color& backdropColor);
    void drawTopLeft(const Color& txtColor, const Color& backdropColor);

public:
    HexagonDialogBox(
        HGAssets& mAssets, ssvs::GameWindow& window, StyleData& styleData);

    void create(const std::string& output, const int charSize,
        const float mFrameSize, const DBoxDraw mDrawMode,
        const float xPos = 0.f, const float yPos = 0.f);
    void create(const std::string& output, const int charSize,
        const float mFrameSize, const DBoxDraw mDrawMode,
        const KKey mKeyToClose, const float mXPos = 0.f,
        const float mYPos = 0.f);

    void draw(const Color& txtColor, const Color& backdropColor);
    void clearDialogBox();

    [[nodiscard]] KKey getKeyToClose() const noexcept
    {
        return keyToClose;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return dialogText.empty();
    }
};

} // namespace hg
