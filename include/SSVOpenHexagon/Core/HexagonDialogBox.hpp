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

class HexagonDialogBox
{
private:
    HGAssets& assets;
    ssvs::GameWindow& window;
    StyleData& styleData;
    sf::Font& imagine;

    Utils::FastVertexVector<sf::PrimitiveType::Quads> dialogFrame;
    std::vector<std::string> dialogText;
    sf::Text txtDialog;

    float dialogHeight{0.f};
    float dialogWidth{0.f};
    float frameOffset{0.f};
    float lineHeight{0.f};

public:
    HexagonDialogBox(
        HGAssets& mAssets, ssvs::GameWindow& window, StyleData& styleData);

    void createDialogBox(const std::string& output, const int charSize);
    void drawDialogBox();
    void clearDialogBox();

    [[nodiscard]] bool empty() const noexcept
    {
        return dialogText.empty();
    }
};

} // namespace hg
