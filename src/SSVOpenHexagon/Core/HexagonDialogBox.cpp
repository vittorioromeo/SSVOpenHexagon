#include "SSVOpenHexagon/Core/HexagonDialogBox.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/FontHeight.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SSVStart/Utils/SFML.hpp>
#include <SSVStart/GameSystem/GameWindow.hpp>

#include <SFML/Graphics/Font.hpp>

#include <algorithm>
#include <string>
#include <tuple>

namespace hg {

[[nodiscard]] static Utils::FastVertexVectorTris& getDialogFrame()
{
    thread_local Utils::FastVertexVectorTris result;
    return result;
}

HexagonDialogBox::HexagonDialogBox(sf::Font& mFont, ssvs::GameWindow& mWindow)
    : window{mWindow}, txtDialog{"", mFont, 0}
{}

void HexagonDialogBox::create(const std::string& output, const int charSize,
    const float mFrameSize, const DBoxDraw mDrawMode, const float mXPos,
    const float mYPos, const bool mInputBox)
{
    lineHeight = Utils::getFontHeight(txtDialog, charSize);
    txtDialog.setString(output);
    dialogWidth = ssvs::getGlobalWidth(txtDialog);
    frameSize = mFrameSize;
    doubleFrameSize = 2.f * frameSize;
    drawMode = mDrawMode;
    xPos = mXPos;
    yPos = mYPos;
    inputBox = mInputBox;
    inputBoxPassword = false;
    input.clear();

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

    if(inputBox)
    {
        totalHeight += lineHeight * 4;
    }
}

void HexagonDialogBox::create(const std::string& output, const int charSize,
    const float mFrameSize, const DBoxDraw mDrawMode, const KKey mKeyToClose,
    const float mXPos, const float mYPos)
{
    create(output, charSize, mFrameSize, mDrawMode, mXPos, mYPos);
    keyToClose = mKeyToClose;
}

void HexagonDialogBox::createInput(const std::string& output,
    const int charSize, const float mFrameSize, const DBoxDraw mDrawMode)
{
    create(output, charSize, mFrameSize, mDrawMode, 0.f, 0.f, true);
    keyToClose = KKey::Enter;
}

void HexagonDialogBox::draw(
    const sf::Color& txtColor, const sf::Color& backdropColor)
{
    switch(drawMode)
    {
        case DBoxDraw::topLeft:
        {
            drawTopLeft(txtColor, backdropColor);
            break;
        }

        case DBoxDraw::center:
        {
            drawCenter(txtColor, backdropColor);
            break;
        }

        default:
        {
            SSVOH_ASSERT(drawMode == DBoxDraw::centerUpperHalf);
            drawCenterUpperHalf(txtColor, backdropColor);
            break;
        }
    }
}

void HexagonDialogBox::drawBox(Utils::FastVertexVectorTris& quads,
    const sf::Color& frameColor, const float x1, const float x2, const float y1,
    const float y2)
{
    const sf::Vector2f nw{x1, y1};
    const sf::Vector2f sw{x1, y2};
    const sf::Vector2f se{x2, y2};
    const sf::Vector2f ne{x2, y1};

    quads.batch_unsafe_emplace_back_quad(frameColor, nw, sw, se, ne);
}

void HexagonDialogBox::drawText(
    const sf::Color& txtColor, const float xOffset, const float yOffset)
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
                    yOffset + heightOffset + 5.f});
            window.draw(txtDialog);
        }

        heightOffset += interline;
    }

    if(inputBox)
    {
        heightOffset += interline;

        if(inputBoxPassword)
        {
            txtDialog.setString(std::string(input.size(), '*'));
        }
        else
        {
            txtDialog.setString(input);
        }

        txtDialog.setPosition({xOffset - ssvs::getGlobalWidth(txtDialog) / 2.f,
            yOffset + heightOffset + 5.f});
        window.draw(txtDialog);
    }
}

inline constexpr float fontHeightDifferential = 0.9f;

void HexagonDialogBox::drawTopLeft(
    const sf::Color& txtColor, const sf::Color& backdropColor)
{
    Utils::FastVertexVectorTris& dialogFrame = getDialogFrame();
    dialogFrame.clear();
    dialogFrame.reserve_quad(2);

    // outer frame
    drawBox(dialogFrame, txtColor, xPos,
        2.f * doubleFrameSize + dialogWidth + xPos, yPos, totalHeight + yPos);

    // text backdrop
    drawBox(dialogFrame, backdropColor, frameSize + xPos,
        doubleFrameSize + frameSize + dialogWidth + xPos, frameSize + yPos,
        totalHeight - frameSize + yPos);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, xPos + doubleFrameSize + dialogWidth / 2.f,
        yPos - lineHeight * fontHeightDifferential + doubleFrameSize);
}

[[nodiscard]] static float calculateFMax(
    const float configWidth, const float configHeight)
{
    return std::max(1024.f / configWidth, 768.f / configHeight);
}

[[nodiscard]] static std::tuple<float, float, float> calculateFMaxAndWAndH(
    const float configWidth, const float configHeight, const float yPos)
{
    const float fmax = calculateFMax(configWidth, configHeight);
    const float w = configWidth * fmax;
    const float h = (configHeight * fmax) / 2.f + yPos;

    return {fmax, w, h};
}

void HexagonDialogBox::drawCenter(
    const sf::Color& txtColor, const sf::Color& backdropColor)
{
    const auto [fmax, w, h] =
        calculateFMaxAndWAndH(Config::getWidth(), Config::getHeight(), yPos);

    const float leftBorder = (w - dialogWidth) / 2.f + xPos,
                rightBorder = (w + dialogWidth) / 2.f + xPos,
                halfHeight = totalHeight / 2.f;

    Utils::FastVertexVectorTris& dialogFrame = getDialogFrame();
    dialogFrame.clear();
    dialogFrame.reserve_quad(2);

    // outer frame
    drawBox(dialogFrame, txtColor, leftBorder - doubleFrameSize,
        rightBorder + doubleFrameSize, h - halfHeight, h + halfHeight);

    // text backdrop
    drawBox(dialogFrame, backdropColor, leftBorder - frameSize,
        rightBorder + frameSize, h - halfHeight + frameSize,
        h + halfHeight - frameSize);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, w / 2.f,
        h - halfHeight - lineHeight * fontHeightDifferential + doubleFrameSize);
}

void HexagonDialogBox::drawCenterUpperHalf(
    const sf::Color& txtColor, const sf::Color& backdropColor)
{
    const auto [fmax, w, h] =
        calculateFMaxAndWAndH(Config::getWidth(), Config::getHeight(), yPos);

    const float leftBorder = (w - dialogWidth) / 2.f + xPos,
                rightBorder = (w + dialogWidth) / 2.f + xPos;

    Utils::FastVertexVectorTris& dialogFrame = getDialogFrame();
    dialogFrame.clear();
    dialogFrame.reserve_quad(2);

    // outer frame
    drawBox(dialogFrame, txtColor, leftBorder - doubleFrameSize,
        rightBorder + doubleFrameSize, h - totalHeight, h);

    // text backdrop
    drawBox(dialogFrame, backdropColor, leftBorder - frameSize,
        rightBorder + frameSize, h - totalHeight + frameSize, h - frameSize);

    window.draw(dialogFrame);

    // Text
    drawText(txtColor, w / 2.f,
        h - totalHeight - lineHeight * fontHeightDifferential +
            doubleFrameSize);
}

void HexagonDialogBox::clearDialogBox()
{
    getDialogFrame().clear();
    dialogText.clear();
    input.clear();
    inputBox = false;
    inputBoxPassword = false;
    keyToClose = KKey::Unknown;
}

[[nodiscard]] ssvs::KKey HexagonDialogBox::getKeyToClose() const noexcept
{
    return keyToClose;
}

[[nodiscard]] bool HexagonDialogBox::empty() const noexcept
{
    return dialogText.empty();
}

[[nodiscard]] bool HexagonDialogBox::isInputBox() const noexcept
{
    return inputBox;
}

[[nodiscard]] std::string& HexagonDialogBox::getInput() noexcept
{
    return input;
}

[[nodiscard]] const std::string& HexagonDialogBox::getInput() const noexcept
{
    return input;
}

void HexagonDialogBox::setInputBoxPassword(const bool x) noexcept
{
    inputBoxPassword = x;
}

[[nodiscard]] bool HexagonDialogBox::getInputBoxPassword() noexcept
{
    return inputBoxPassword;
}

} // namespace hg
