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

void HexagonDialogBox::create(const std::string& output, const int charSize,
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

    case DBoxDraw::center:
        drawFunc = [this](const Color& txtColor, const Color& backdropColor) {
            drawCenter(txtColor, backdropColor);
        };
        break;

    case DBoxDraw::centerUpperHalf:
        drawFunc = [this](const Color& txtColor, const Color& backdropColor) {
          drawCenterUpperHalf(txtColor, backdropColor);
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

void HexagonDialogBox::create(const std::string& output, const int charSize,
    const float mFrameSize, const int mDrawMode, const KKey mKeyToClose,
    const float mXPos, const float mYPos)
{
    create(output, charSize, mFrameSize, mDrawMode, mXPos, mYPos);
    keyToClose = mKeyToClose;
}

void HexagonDialogBox::draw(const Color& txtColor, const Color& backdropColor)
{
    drawFunc(txtColor, backdropColor);
}

void HexagonDialogBox::drawBox(const Color& frameColor, const float x1,
    const float x2, const float y1, const float y2)
{
    sf::Vector2f p1{x1, y1}; // top left
    sf::Vector2f p2{x2, y1}; // top right
    sf::Vector2f p3{x2, y2}; // bottom right
    sf::Vector2f p4{x1, y2}; // bottom left
    dialogFrame.batch_unsafe_emplace_back(frameColor, p1, p2, p3, p4);
}

void HexagonDialogBox::drawText(const Color& txtColor, const float xOffset, const float yOffset)
{
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

    dialogFrame.clear();
    dialogFrame.reserve_more(8);

    // outer frame
    drawBox(txtColor, xPos, xPos + 2.f * doubleFrameSize + dialogWidth,
        yPos, yPos + 2.f * doubleFrameSize + dialogHeight - heightDifBottom - heightDif);

    // text backdrop
    drawBox(backdropColor, xPos + frameSize,
        xPos + doubleFrameSize + frameSize + dialogWidth, yPos + frameSize,
        yPos + doubleFrameSize + frameSize + dialogHeight - heightDifBottom - heightDif);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, xPos + doubleFrameSize + dialogWidth / 2.f,
        yPos + doubleFrameSize - heightDif);
}

void HexagonDialogBox::drawCenter(const Color& txtColor, const Color& backdropColor)
{
    const float fmax =
        std::max(1024.f / Config::getWidth(), 768.f / Config::getHeight()),
        w = Config::getWidth() * fmax,
        h = (Config::getHeight() * fmax) / 2.f + yPos;

    const float heightDifTop = lineHeight * 0.85f,
                heightDifBottom = lineHeight * 0.6f,
                leftBorder = (w - dialogWidth) / 2.f + xPos,
                rightBorder = (w + dialogWidth) / 2.f,
                heightOffsetTop = dialogHeight / 2.f,
                heightOffsetBottom = heightOffsetTop;

    dialogFrame.clear();
    dialogFrame.reserve_more(8);

    // outer frame
    drawBox(txtColor, leftBorder - doubleFrameSize, rightBorder + doubleFrameSize,
            h - heightOffsetTop - doubleFrameSize + heightDifTop,
            h + heightOffsetBottom + doubleFrameSize - heightDifBottom);

    // text backdrop
    drawBox(backdropColor, leftBorder - frameSize, rightBorder + frameSize,
            h - heightOffsetTop - frameSize + heightDifTop,
            h + heightOffsetBottom + frameSize - heightDifBottom);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, w / 2.f, h - heightOffsetTop);
}

void HexagonDialogBox::drawCenterUpperHalf(const Color& txtColor, const Color& backdropColor)
{
    const float fmax =
        std::max(1024.f / Config::getWidth(), 768.f / Config::getHeight()),
                w = Config::getWidth() * fmax,
                h = (Config::getHeight() * fmax) / 2.f + yPos;

    const float heightDifTop = lineHeight * 0.85f,
                heightDifBottom = lineHeight * 0.6f,
                leftBorder = (w - dialogWidth) / 2.f + xPos,
                rightBorder = (w + dialogWidth) / 2.f;

    dialogFrame.clear();
    dialogFrame.reserve_more(8);

    // outer frame (text color)
    drawBox(txtColor, leftBorder - doubleFrameSize, rightBorder + doubleFrameSize,
            h - dialogHeight - doubleFrameSize + heightDifTop,
            h + doubleFrameSize - heightDifBottom);

    // text backdrop (spinning background color)
    drawBox(backdropColor, leftBorder - frameSize, rightBorder + frameSize,
            h - dialogHeight - frameSize + heightDifTop,
            h + frameSize - heightDifBottom);

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
