// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Keyboard.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
class Font;
class RenderTarget;
} // namespace sf

namespace ssvs
{
class GameWindow;
}

namespace hg::Utils
{
class FastVertexVectorQuads;
} // namespace hg::Utils

namespace hg
{

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

    // Optional override for the actual draw target. When non-null, all
    // dialog draws go here instead of `window.getRenderWindow()` -- the
    // host uses this to route the dialog into the new UI's off-screen
    // composite texture so its magenta-sentinel frame goes through the
    // accent gradient shader at composite time. Set per-frame via
    // `setRenderTargetOverride`.
    sf::RenderTarget* renderTargetOverride{nullptr};

    DBoxDraw drawMode;

    sf::base::Vector<sf::base::String> dialogText;
    sf::Text                           txtDialog;

    float dialogWidth{0.f};
    float frameSize{0.f};
    float doubleFrameSize{0.f};
    float lineHeight{0.f};
    float totalHeight{0.f};

    float xPos{0.f};
    float yPos{0.f};

    KKey keyToClose{KKey::Unknown};

    bool             inputBox{false};
    bool             inputBoxPassword{false};
    sf::base::String input;

    void drawText(const sf::View& view, const sf::Color txtColor, const float xOffset, const float yOffset);
    void drawBox(Utils::FastVertexVectorQuads& quads,
                 const sf::Color               frameColor,
                 const float                   x1,
                 const float                   x2,
                 const float                   y1,
                 const float                   y2);
    void drawCenter(const sf::View& view, const sf::Color txtColor, const sf::Color frameColor, const sf::Color backdropColor);
    void drawCenterUpperHalf(const sf::View& view,
                             const sf::Color txtColor,
                             const sf::Color frameColor,
                             const sf::Color backdropColor);
    void drawTopLeft(const sf::View& view, const sf::Color txtColor, const sf::Color frameColor, const sf::Color backdropColor);

public:
    explicit HexagonDialogBox(sf::Font& font, ssvs::GameWindow& window);

    void create(const sf::base::String& output,
                const int               charSize,
                const float             mFrameSize,
                const DBoxDraw          mDrawMode,
                const float             xPos      = 0.f,
                const float             yPos      = 0.f,
                const bool              mInputBox = false);

    void create(const sf::base::String& output,
                const int               charSize,
                const float             mFrameSize,
                const DBoxDraw          mDrawMode,
                const KKey              mKeyToClose,
                const float             mXPos = 0.f,
                const float             mYPos = 0.f);

    void createInput(const sf::base::String& output, const int charSize, const float mFrameSize, const DBoxDraw mDrawMode);

    void draw(const sf::View& view, const sf::Color txtColor, const sf::Color frameColor, const sf::Color backdropColor);

    // Redirects subsequent draws to `target` instead of the window. Pass
    // `nullptr` to restore the default. Reset isn't automatic, so callers
    // are expected to set + clear bracketing each frame.
    void setRenderTargetOverride(sf::RenderTarget* target) noexcept;

    void clearDialogBox();

    [[nodiscard]] KKey                    getKeyToClose() const noexcept;
    [[nodiscard]] bool                    empty() const noexcept;
    [[nodiscard]] bool                    isInputBox() const noexcept;
    [[nodiscard]] sf::base::String&       getInput() noexcept;
    [[nodiscard]] const sf::base::String& getInput() const noexcept;
    void                                  setInputBoxPassword(const bool x) noexcept;
    [[nodiscard]] bool                    getInputBoxPassword() noexcept;
};

} // namespace hg
