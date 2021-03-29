// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SSVStart/GameSystem/GameSystem.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <string>

namespace hg
{

class HGAssets;

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
    using DrawFunc = std::function<void(const sf::Color&, const sf::Color&)>;

    HGAssets& assets;
    ssvs::GameWindow& window;
    DrawFunc drawFunc;

    Utils::FastVertexVectorQuads dialogFrame;
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
        const sf::Color& txtColor, const float xOffset, const float yOffset);
    void drawBox(const sf::Color& frameColor, const float x1, const float x2,
        const float y1, const float y2);
    void drawCenter(const sf::Color& txtColor, const sf::Color& backdropColor);
    void drawCenterUpperHalf(
        const sf::Color& txtColor, const sf::Color& backdropColor);
    void drawTopLeft(const sf::Color& txtColor, const sf::Color& backdropColor);

public:
    explicit HexagonDialogBox(HGAssets& mAssets, ssvs::GameWindow& window);

    void create(const std::string& output, const int charSize,
        const float mFrameSize, const DBoxDraw mDrawMode,
        const float xPos = 0.f, const float yPos = 0.f);
    void create(const std::string& output, const int charSize,
        const float mFrameSize, const DBoxDraw mDrawMode,
        const KKey mKeyToClose, const float mXPos = 0.f,
        const float mYPos = 0.f);

    void draw(const sf::Color& txtColor, const sf::Color& backdropColor);
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
