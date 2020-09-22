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

enum DBoxDraw
{
    topLeft = 0,
    centered,
    levelReload
};

class HexagonDialogBox
{
private:
    using KKey = sf::Keyboard::Key;

    HGAssets& assets;
    ssvs::GameWindow& window;
    StyleData& styleData;
    sf::Font& imagine;

    Utils::FastVertexVector<sf::PrimitiveType::Quads> dialogFrame;
    std::vector<std::string> dialogText;
    sf::Text txtDialog;

    float dialogHeight{0.f};
    float dialogWidth{0.f};
    float frameSize{0.f};
    float doubleFrameSize{0.f};
    float lineHeight{0.f};

    int drawMode{0};
    float xPos{0.f};
    float yPos{0.f};

    KKey keyToClose{KKey::Unknown};

public:
    HexagonDialogBox(
        HGAssets& mAssets, ssvs::GameWindow& window, StyleData& styleData);

    void createDialogBox(const std::string& output, const int charSize,
        const float mFrameSize, const int mDrawMode,
        const float xPos = 0.f, const float yPos = 0.f);
    void createDialogBox(const std::string& output, const int charSize,
        const float mFrameSize, const int mDrawMode,
        const KKey mKeyToClose, const float mXPos = 0.f, const float mYPos = 0.f);
    void drawDialogBox();
    void drawDialogBoxCentered();
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
