#include "SSVOpenHexagon/Core/HexagonDialogBox.hpp"

#include "SSVOpenHexagon/Utils/Utils.hpp"
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

void HexagonDialogBox::create(const std::string& output, const int charSize,
    const float mFrameSize, const DBoxDraw mDrawMode, const float mXPos,
    const float mYPos)
{
    lineHeight = Utils::getFontHeight(txtDialog, charSize);
    txtDialog.setString(output);
    dialogWidth = ssvs::getGlobalWidth(txtDialog);
    frameSize = mFrameSize;
    doubleFrameSize = 2.f * frameSize;
    drawFunc = drawModeToDrawFunc(mDrawMode);
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

    const int size = dialogText.size();
    const float dialogHeight =
        lineHeight * size + (lineHeight / 2.f) * (size - 1);
    totalHeight = dialogHeight + 2.f * doubleFrameSize;
}

void HexagonDialogBox::create(const std::string& output, const int charSize,
    const float mFrameSize, const DBoxDraw mDrawMode, const KKey mKeyToClose,
    const float mXPos, const float mYPos)
{
    create(output, charSize, mFrameSize, mDrawMode, mXPos, mYPos);
    keyToClose = mKeyToClose;
}

HexagonDialogBox::DrawFunc HexagonDialogBox::drawModeToDrawFunc(
    DBoxDraw drawMode)
{
    switch(drawMode)
    {
        case DBoxDraw::topLeft:
        {
            return [this](const Color& txtColor, const Color& backdropColor) {
                drawTopLeft(txtColor, backdropColor);
            };
        }

        case DBoxDraw::center:
        {
            return [this](const Color& txtColor, const Color& backdropColor) {
                drawCenter(txtColor, backdropColor);
            };
        }

        default:
        {
            assert(drawMode == DBoxDraw::centerUpperHalf);
            return [this](const Color& txtColor, const Color& backdropColor) {
                drawCenterUpperHalf(txtColor, backdropColor);
            };
        }
    }
}

void HexagonDialogBox::draw(const Color& txtColor, const Color& backdropColor)
{
    drawFunc(txtColor, backdropColor);
}

void HexagonDialogBox::drawBox(const Color& frameColor, const float x1,
    const float x2, const float y1, const float y2)
{
    sf::Vector2f topLeft{x1, y1};
    sf::Vector2f topRight{x2, y1};
    sf::Vector2f bottomRight{x2, y2};
    sf::Vector2f bottomLeft{x1, y2};
    dialogFrame.batch_unsafe_emplace_back(
        frameColor, topLeft, topRight, bottomRight, bottomLeft);
}

void HexagonDialogBox::drawText(
    const Color& txtColor, const float xOffset, const float yOffset)
{
    float heightOffset = 0.f;
    const float interline = lineHeight * 1.5f;
    txtDialog.setFillColor(txtColor);

    for(auto& str : dialogText)
    {
        if(!str.empty())
        {
            txtDialog.setString(str);
            txtDialog.setPosition(
                {xOffset - ssvs::getGlobalWidth(txtDialog) / 2.f,
                    yOffset + heightOffset});
            window.draw(txtDialog);
        }
        heightOffset += interline;
    }
}

inline constexpr float fontHeightDifferential = 0.9f;

void HexagonDialogBox::drawTopLeft(
    const Color& txtColor, const Color& backdropColor)
{
    dialogFrame.clear();
    dialogFrame.reserve(8);

    // outer frame
    drawBox(txtColor, xPos, 2.f * doubleFrameSize + dialogWidth + xPos, yPos,
        totalHeight + yPos);

    // text backdrop
    drawBox(backdropColor, frameSize + xPos,
        doubleFrameSize + frameSize + dialogWidth + xPos, frameSize + yPos,
        totalHeight - frameSize + yPos);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, xPos + doubleFrameSize + dialogWidth / 2.f,
        yPos - lineHeight * fontHeightDifferential + doubleFrameSize);
}

void HexagonDialogBox::drawCenter(
    const Color& txtColor, const Color& backdropColor)
{
    const float fmax = std::max(
                    1024.f / Config::getWidth(), 768.f / Config::getHeight()),
                w = Config::getWidth() * fmax,
                h = (Config::getHeight() * fmax) / 2.f + yPos;

    const float leftBorder = (w - dialogWidth) / 2.f + xPos,
                rightBorder = (w + dialogWidth) / 2.f + xPos,
                halfHeight = totalHeight / 2.f;

    dialogFrame.clear();
    dialogFrame.reserve(8);

    // outer frame
    drawBox(txtColor, leftBorder - doubleFrameSize,
        rightBorder + doubleFrameSize, h - halfHeight, h + halfHeight);

    // text backdrop
    drawBox(backdropColor, leftBorder - frameSize, rightBorder + frameSize,
        h - halfHeight + frameSize, h + halfHeight - frameSize);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, w / 2.f,
        h - halfHeight - lineHeight * fontHeightDifferential + doubleFrameSize);
}

void HexagonDialogBox::drawCenterUpperHalf(
    const Color& txtColor, const Color& backdropColor)
{
    const float fmax = std::max(
                    1024.f / Config::getWidth(), 768.f / Config::getHeight()),
                w = Config::getWidth() * fmax,
                h = (Config::getHeight() * fmax) / 2.f + yPos;

    const float leftBorder = (w - dialogWidth) / 2.f + xPos,
                rightBorder = (w + dialogWidth) / 2.f + xPos;

    dialogFrame.clear();
    dialogFrame.reserve(8);

    // outer frame
    drawBox(txtColor, leftBorder - doubleFrameSize,
        rightBorder + doubleFrameSize, h - totalHeight, h);

    // text backdrop
    drawBox(backdropColor, leftBorder - frameSize, rightBorder + frameSize,
        h - totalHeight + frameSize, h - frameSize);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, w / 2.f,
        h - totalHeight - lineHeight * fontHeightDifferential +
            doubleFrameSize);
}

void HexagonDialogBox::clearDialogBox()
{
    assets.playSound("select.ogg");
    dialogFrame.clear();
    dialogText.clear();
    keyToClose = KKey::Unknown;
}

} // namespace hg
