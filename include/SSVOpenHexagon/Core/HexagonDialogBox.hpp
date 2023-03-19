// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <string>
#include <vector>

namespace sf {
class Font;
}

namespace ssvs {
class GameWindow;
}

namespace hg::Utils {
class FastVertexVectorTris;
} // namespace hg::Utils

namespace hg {

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

    ssvs::GameWindow& window;

    DBoxDraw drawMode;

    std::vector<std::string> dialogText;
    sf::Text txtDialog;

    float dialogWidth{0.f};
    float frameSize{0.f};
    float doubleFrameSize{0.f};
    float lineHeight{0.f};
    float totalHeight{0.f};

    float xPos{0.f};
    float yPos{0.f};

    KKey keyToClose{KKey::Unknown};

    bool inputBox{false};
    bool inputBoxPassword{false};
    std::string input;

    void drawText(
        const sf::Color& txtColor, const float xOffset, const float yOffset);
    void drawBox(Utils::FastVertexVectorTris& quads,
        const sf::Color& frameColor, const float x1, const float x2,
        const float y1, const float y2);
    void drawCenter(const sf::Color& txtColor, const sf::Color& backdropColor);
    void drawCenterUpperHalf(
        const sf::Color& txtColor, const sf::Color& backdropColor);
    void drawTopLeft(const sf::Color& txtColor, const sf::Color& backdropColor);

public:
    explicit HexagonDialogBox(sf::Font& font, ssvs::GameWindow& window);

    void create(const std::string& output, const int charSize,
        const float mFrameSize, const DBoxDraw mDrawMode,
        const float xPos = 0.f, const float yPos = 0.f,
        const bool mInputBox = false);

    void create(const std::string& output, const int charSize,
        const float mFrameSize, const DBoxDraw mDrawMode,
        const KKey mKeyToClose, const float mXPos = 0.f,
        const float mYPos = 0.f);

    void createInput(const std::string& output, const int charSize,
        const float mFrameSize, const DBoxDraw mDrawMode);

    void draw(const sf::Color& txtColor, const sf::Color& backdropColor);

    void clearDialogBox();

    [[nodiscard]] KKey getKeyToClose() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool isInputBox() const noexcept;
    [[nodiscard]] std::string& getInput() noexcept;
    [[nodiscard]] const std::string& getInput() const noexcept;
    void setInputBoxPassword(const bool x) noexcept;
    [[nodiscard]] bool getInputBoxPassword() noexcept;
};

} // namespace hg
