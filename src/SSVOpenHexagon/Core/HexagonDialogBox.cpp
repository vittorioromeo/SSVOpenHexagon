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
      imagine{assets.get<sf::Font>("forcedsquare.ttf")},
      txtDialog{"", imagine, 0}
{
}

void HexagonDialogBox::createDialogBox(const std::string& output, const int charSize,
    const float mFrameSize, const int mDrawMode, const float mXPos, const float mYPos)
{
    txtDialog.setCharacterSize(charSize);
    txtDialog.setString(output);
    dialogWidth = ssvs::getGlobalWidth(txtDialog);
    dialogHeight = ssvs::getGlobalHeight(txtDialog);

    txtDialog.setString("A");
    lineHeight = ssvs::getGlobalHeight(txtDialog);

    frameSize = mFrameSize;
    doubleFrameSize = 2.f * frameSize;

    switch(mDrawMode)
    {
    case DBoxDraw::topLeft:
        drawFunc = [this](const Color& txtColor, const Color& backdropColor) {
            drawTopLeft(txtColor, backdropColor);
        };
        break;

    case DBoxDraw::centered:
        drawFunc = [this](const Color& txtColor, const Color& backdropColor) {
            drawCentered(txtColor, backdropColor);
        };
        break;

    case DBoxDraw::centeredUpperHalf:
        drawFunc = [this](const Color& txtColor, const Color& backdropColor) {
          drawCenteredUpperHalf(txtColor, backdropColor);
        };
        break;
    }

    xPos = mXPos;
    yPos = mYPos;

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

void HexagonDialogBox::createDialogBox(const std::string& output, const int charSize,
    const float mFrameSize, const int mDrawMode, const KKey mKeyToClose,
    const float mXPos, const float mYPos)
{
    createDialogBox(output, charSize, mFrameSize, mDrawMode, mXPos, mYPos);
    keyToClose = mKeyToClose;
}

void HexagonDialogBox::draw(const Color& txtColor, const Color& backdropColor)
{
    drawFunc(txtColor, backdropColor);
}

void HexagonDialogBox::drawText(const Color& txtColor, const float xOffset, const float yOffset)
{
    // text
    float heightOffset = 0.f;
    const float interlineSpace = lineHeight * 1.5f;
    txtDialog.setFillColor(txtColor);
    for(auto& str : dialogText)
    {
        if(!str.empty())
        {
            txtDialog.setString(str);
            txtDialog.setPosition({xOffset - ssvs::getGlobalWidth(txtDialog) / 2.f,
                yOffset + heightOffset});
            window.draw(txtDialog);
        }
        heightOffset += interlineSpace;
    }
}

void HexagonDialogBox::drawTopLeft(const Color& txtColor, const Color& backdropColor)
{
    // Alright what's this: if I apply the clean txtDialog height value to these
    // quads there is a small extra margin on the top and bottom (right and left
    // are perfect). Luckily the height of those margins are 0.85f of the height
    // of one line for the top, and around 0.6f of the same line height for the
    // bottom. Is there a better way to do it? Maybe, but I printed some
    // txtDialog.getXXX() values and cannot see an obvious answer.
    const float heightDif = lineHeight * 0.85f,
                heightDifBottom = lineHeight * 0.6f;

    // outer frame (text color)
    dialogFrame.clear();
    dialogFrame.reserve_more(8);

    sf::Vector2f p1{xPos - doubleFrameSize,                 yPos - doubleFrameSize + heightDif}; // top left
    sf::Vector2f p2{xPos + dialogWidth + doubleFrameSize,   yPos - doubleFrameSize + heightDif}; // top right
    sf::Vector2f p3{xPos + dialogWidth + doubleFrameSize,   yPos + dialogHeight + doubleFrameSize - heightDifBottom}; // bottom right
    sf::Vector2f p4{xPos - doubleFrameSize,                 yPos + dialogHeight + doubleFrameSize - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(txtColor, p1, p2, p3, p4);

    // text backdrop (spinning background color)
    p1 = {xPos - frameSize,                 yPos - frameSize + heightDif}; // top left
    p2 = {xPos + dialogWidth + frameSize,   yPos - frameSize + heightDif}; // top right
    p3 = {xPos + dialogWidth + frameSize,   yPos + dialogHeight + frameSize - heightDifBottom}; // bottom right
    p4 = {xPos - frameSize,                 yPos + dialogHeight + frameSize - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(backdropColor, p1, p2, p3, p4);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, xPos + dialogWidth / 2.f, yPos);
}

void HexagonDialogBox::drawCentered(const Color& txtColor, const Color& backdropColor)
{
    const float fmax =
        std::max(1024.f / Config::getWidth(), 768.f / Config::getHeight()),
        w = Config::getWidth() * fmax,
        h = (Config::getHeight() * fmax) / 2.f + yPos;

    const float heightDifTop = lineHeight * 0.85f,
        heightDifBottom = lineHeight * 0.6f;

    // outer frame (text color)
    const float leftBorder = (w - dialogWidth) / 2.f + xPos,
        rightBorder = (w + dialogWidth) / 2.f;

    const float heightOffsetTop = dialogHeight / 2.f,
        heightOffsetBottom = heightOffsetTop;

    dialogFrame.clear();
    dialogFrame.reserve_more(8);

    sf::Vector2f p1{leftBorder - doubleFrameSize,   h - heightOffsetTop - doubleFrameSize + heightDifTop}; // top left
    sf::Vector2f p2{rightBorder + doubleFrameSize,  h - heightOffsetTop - doubleFrameSize + heightDifTop}; // top right
    sf::Vector2f p3{rightBorder + doubleFrameSize,  h + heightOffsetBottom + doubleFrameSize - heightDifBottom}; // bottom right
    sf::Vector2f p4{leftBorder - doubleFrameSize,   h + heightOffsetBottom + doubleFrameSize - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(txtColor, p1, p2, p3, p4);

    // text backdrop (spinning background color)
    p1 = {leftBorder - frameSize,   h - heightOffsetTop - frameSize + heightDifTop}; // top left
    p2 = {rightBorder + frameSize,  h - heightOffsetTop - frameSize + heightDifTop}; // top right
    p3 = {rightBorder + frameSize,  h + heightOffsetBottom + frameSize - heightDifBottom}; // bottom right
    p4 = {leftBorder - frameSize,   h + heightOffsetBottom + frameSize - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(backdropColor, p1, p2, p3, p4);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, w / 2.f, h - heightOffsetTop);
}

void HexagonDialogBox::drawCenteredUpperHalf(const Color& txtColor, const Color& backdropColor)
{
    const float fmax =
        std::max(1024.f / Config::getWidth(), 768.f / Config::getHeight()),
                w = Config::getWidth() * fmax,
                h = (Config::getHeight() * fmax) / 2.f + yPos;

    const float heightDifTop = lineHeight * 0.85f,
                heightDifBottom = lineHeight * 0.6f;

    // outer frame (text color)
    const float leftBorder = (w - dialogWidth) / 2.f + xPos,
                rightBorder = (w + dialogWidth) / 2.f;

    dialogFrame.clear();
    dialogFrame.reserve_more(8);

    sf::Vector2f p1{leftBorder - doubleFrameSize,   h - dialogHeight - doubleFrameSize + heightDifTop}; // top left
    sf::Vector2f p2{rightBorder + doubleFrameSize,  h - dialogHeight - doubleFrameSize + heightDifTop}; // top right
    sf::Vector2f p3{rightBorder + doubleFrameSize,  h + doubleFrameSize - heightDifBottom}; // bottom right
    sf::Vector2f p4{leftBorder - doubleFrameSize,   h + doubleFrameSize - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(txtColor, p1, p2, p3, p4);

    // text backdrop (spinning background color)
    p1 = {leftBorder - frameSize,   h - dialogHeight - frameSize + heightDifTop}; // top left
    p2 = {rightBorder + frameSize,  h - dialogHeight - frameSize + heightDifTop}; // top right
    p3 = {rightBorder + frameSize,  h + frameSize - heightDifBottom}; // bottom right
    p4 = {leftBorder - frameSize,   h + frameSize - heightDifBottom}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(backdropColor, p1, p2, p3, p4);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, w / 2.f, h - dialogHeight);
}

void HexagonDialogBox::clearDialogBox()
{
    assets.playSound("select.ogg");
    dialogFrame.clear();
    dialogText.clear();
    keyToClose = KKey::Unknown;
}

} // namespace hg
