#include "SSVOpenHexagon/Core/HexagonDialogBox.hpp"

#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

#include <SSVStart/Utils/SFML.hpp>

namespace hg
{

HexagonDialogBox::HexagonDialogBox(
    HGAssets& mAssets, ssvs::GameWindow& mWindow, StyleData& mStyleData)
    : assets{mAssets}, window{mWindow}, styleData{mStyleData},
      imagine{assets.get<sf::Font>("forcedsquare.ttf")}, txtDialog{
                                                             "", imagine, 0}
{
}

void HexagonDialogBox::createDialogBox(
    const std::string& output, const int charSize)
{
    txtDialog.setCharacterSize(charSize);
    txtDialog.setString(output);
    dialogWidth = ssvs::getGlobalWidth(txtDialog);
    dialogHeight = ssvs::getGlobalHeight(txtDialog);

    // in order to properly adjust the text after the dialogText vector has been
    // built we need to know the height of a single line. dialogHeight is the
    // height of the entire box. So we assign an arbitrary text string and
    // record the outputted value
    txtDialog.setString("A");
    lineHeight = ssvs::getGlobalHeight(txtDialog);

    frameOffset = 10.f;

    std::string temp;
    for(char c : output)
    {
        if(c == '\n')
        {
            dialogText.emplace_back(temp);
            temp.clear();
        }
        else
        {
            temp += c;
        }
    }
}

void HexagonDialogBox::drawDialogBox()
{
    const sf::Color color = styleData.getTextColor();

    const float fmax =
        std::max(1024.f / Config::getWidth(), 768.f / Config::getHeight());

    const float w = Config::getWidth() * fmax;
    const float h = (Config::getHeight() * fmax) / 2.f;

    // Alright what's this: if I apply the clean txtDialog height value to these
    // quads there is a small extra margin on the top and bottom (right and left
    // are perfect). Luckily the height of those margins are 0.85f of the height
    // of one line for the top, and around 0.6f of the same line height for the
    // bottom. Is there a better way to do it? Maybe, but I printed some
    // txtDialog.getXXX() values and cannot see an obvious answer.
    const float heightDif = lineHeight * 0.85f,
                heightDifBottom = lineHeight * 0.6f;

    // outer frame (text color)
    const float leftBorder = (w - dialogWidth) / 2.f,
                rightBorder = (w + dialogWidth) / 2.f,
                doubleOffset = frameOffset * 2.f;

    dialogFrame.clear();
    dialogFrame.reserve_more(8);

    sf::Vector2f p1{leftBorder - doubleOffset,
        h - dialogHeight - doubleOffset + heightDif}; // top left
    sf::Vector2f p2{rightBorder + doubleOffset,
        h - dialogHeight - doubleOffset + heightDif}; // top right
    sf::Vector2f p3{rightBorder + doubleOffset,
        h + doubleOffset - heightDifBottom}; // bottom right
    sf::Vector2f p4{leftBorder - doubleOffset,
        h + doubleOffset - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(color, p1, p2, p3, p4);

    // text backdrop (spinning background color)
    p1 = {leftBorder - frameOffset,
        h - dialogHeight - frameOffset + heightDif}; // top left
    p2 = {rightBorder + frameOffset,
        h - dialogHeight - frameOffset + heightDif}; // top right
    p3 = {rightBorder + frameOffset,
        h + frameOffset - heightDifBottom}; // bottom right
    p4 = {leftBorder - frameOffset,
        h + frameOffset - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(
        styleData.getColor(0), p1, p2, p3, p4);

    window.draw(dialogFrame);

    // text
    float heightOffset = 0.f;
    const float interlineSpace = lineHeight * 1.5f;
    txtDialog.setFillColor(color);
    for(auto& str : dialogText)
    {
        if(!str.empty())
        {
            txtDialog.setString(str);
            txtDialog.setPosition({(w - ssvs::getGlobalWidth(txtDialog)) / 2.f,
                h - dialogHeight + heightOffset});
            window.draw(txtDialog);
        }
        heightOffset += interlineSpace;
    }
}

void HexagonDialogBox::clearDialogBox()
{
    assets.playSound("beep.ogg");
    dialogFrame.clear();
    dialogText.clear();
}

} // namespace hg
