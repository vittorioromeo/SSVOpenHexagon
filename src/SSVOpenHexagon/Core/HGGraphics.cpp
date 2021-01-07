// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include <SSVStart/Utils/Vector2.hpp>

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace hg::Utils;
using namespace ssvu;

namespace hg
{

[[nodiscard]] static std::string formatTime(const double x)
{
    return ssvu::toStr(std::floor(x * 1000) / 1000.f);
}

void HexagonGame::drawSetup()
{
    // Setup.
    styleData.computeColors(levelStatus);
    window.clear(Color::Black);    

    // Death camera shake.
    if(status.hasDied)
    {
        return;
    }

    if(levelStatus.cameraShake > 0.f)
    {
        const sf::Vector2f shake(
            getRndR(-levelStatus.cameraShake, levelStatus.cameraShake),
            getRndR(-levelStatus.cameraShake, levelStatus.cameraShake));

        backgroundCamera.setCenter(shake);
        overlayCamera.setCenter(
            shake + sf::Vector2f{Config::getWidth() / 2.f,
            Config::getHeight() / 2.f});
    }
    else
    {
        backgroundCamera.setCenter(ssvs::zeroVec2f);
        overlayCamera.setCenter(sf::Vector2f{
            Config::getWidth() / 2.f, Config::getHeight() / 2.f});
    }
}

void HexagonGame::drawWrapup()
{
    // Top camera and text.
    overlayCamera.apply();
    drawText();

    // HUD icons.
    if(Config::getShowKeyIcons() || mustShowReplayUI())
    {
        drawKeyIcons();
    }

    // Misc.
    if(Config::getFlash())
    {
        render(flashPolygon);
    }

    if(mustTakeScreenshot)
    {
        window.saveScreenshot("screenshot.png");
        mustTakeScreenshot = false;
    }
}

void HexagonGame::draw2D()
{
    wallQuads.clear();
    playerTris.clear();
    capTris.clear();
    capQuads.clear();
    
    if(!Config::getNoBackground())
    {
        backgroundCamera.apply();
        styleData.drawBackground(window, centerPos, levelStatus);
    }

    // Draw the walls and player.
    for(CWall& w : walls)
    {
        w.draw(*this);
    }

    cwManager.draw(*this);

    if(status.started)
    {
        player.draw(*this, styleData.getCapColorResult());
    }

    render(wallQuads);
    render(playerTris);
    render(capTris);
    render(capQuads);
}

void HexagonGame::drawProjections()
{
    wallQuads3D.clear();
    playerTris3D.clear();
    wallQuads.clear();
    playerTris.clear();
    capQuads.clear();
    capTris.clear();

    if(!Config::getNoBackground())
    {
        backgroundCamera.apply();
        styleData.drawBackground(window, centerPos, levelStatus);
    }

    // Draw the walls and player.
    for(CWall& w : walls)
    {
        w.draw(*this);
    }

    cwManager.draw(*this);

    if(status.started)
    {
        player.draw(*this, styleData.getCapColorResult());
    }

    // Draw the projections.
    const float effect{
        styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};
    const sf::Vector2f skew{1.f, 1.f + effect};
    backgroundCamera.setSkew(skew);

    const float depth(styleData._3dDepth);
    const std::size_t numWallQuads(wallQuads.size() + capQuads.size());
    const std::size_t numPlayerTris(playerTris.size());

    wallQuads3D.reserve(numWallQuads * depth);
    playerTris3D.reserve(numPlayerTris * depth);

    const float radRot(
        ssvu::toRad(backgroundCamera.getRotation()) + (ssvu::pi / 2.f));
    const float sinRot(std::sin(radRot));
    const float cosRot(std::cos(radRot));

    sf::Color wallColor{getColorDarkened(styleData.get3DOverrideColor(), styleData._3dDarkenMult)},
        playerColor{styleData.get3DOverrideColor() == styleData.getMainColor() ?
            getColorDarkened(styleData.getPlayerColor(), styleData._3dDarkenMult) :
            wallColor};
    wallColor.a /= styleData._3dAlphaMult;
    playerColor.a /= styleData._3dAlphaMult;

    std::size_t j;
    sf::Vector2f newPos;
    float depthIndex, offset;
    const float effectMult{(effect * 3.6f) * 1.4f};
    for(unsigned int i = 0; i < depth; ++i)
    {
        wallQuads3D.unsafe_emplace_other(wallQuads);
        wallQuads3D.unsafe_emplace_other(capQuads);
        playerTris3D.unsafe_emplace_other(playerTris);

        depthIndex = depth - i - 1;
        offset = styleData._3dSpacing *
                ((depthIndex + 1.f) * styleData._3dPerspectiveMult) * effectMult;
        newPos = {offset * cosRot, offset * sinRot};
        wallColor.a -= styleData._3dAlphaFalloff;
        playerColor.a -= styleData._3dAlphaFalloff;

        for(j = i * numWallQuads; j < (i + 1) * numWallQuads; ++j)
        {
            wallQuads3D[j].position += newPos;
            wallQuads3D[j].color = wallColor;
        }
        for(j = i * numPlayerTris; j < (i + 1) * numPlayerTris; ++j)
        {
            playerTris3D[j].position += newPos;
            playerTris3D[j].color = playerColor;
        }
    }

    render(wallQuads3D);
    render(playerTris3D);
    render(wallQuads);
    render(playerTris);
    render(capTris);
    render(capQuads);
}

void HexagonGame::draw3D()
{
    wallQuads3D.clear();
    wallQuads.clear();
    playerTris.clear();
    capTris.clear();
    capQuads.clear();

    // Calculate the 3D effect shift.
    const float effect{
        styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};
    backgroundCamera.setSkew({1.f, 1.f + effect});
    const float offset(styleData._3dSpacing *
                ((styleData._3dDepth + 1.f) * styleData._3dPerspectiveMult) *
                (effect * 3.6f) * 1.4f);
    const float radRot(ssvu::toRad(backgroundCamera.getRotation()) + ssvu::piHalf);
    offset3D = {offset * std::cos(radRot), offset * std::sin(radRot)};

    // Draw the background.
    if(!Config::getNoBackground())
    {
        backgroundCamera.apply();
        styleData.drawBackground(window,
            {ssvs::zeroVec2f.x + offset3D.x, ssvs::zeroVec2f.y + offset3D.y},
            levelStatus);
    }

    const sf::Color wallColor{getColorDarkened(
        styleData.get3DOverrideColor(), styleData._3dDarkenMult)};

    // Draw the custom walls.
    cwManager.draw3D(*this, wallColor);

    // Draw the regular walls.
    for(CWall& w : walls)
    {
        w.draw3D(*this, player, centerPos, wallColor);
    }

    // Handle player and pivot.
    if(status.started)
    {
        player.draw3D(*this,
            styleData.get3DOverrideColor() == styleData.getMainColor() ?
                getColorDarkened(styleData.getPlayerColor(), styleData._3dDarkenMult) :
                wallColor, styleData.getCapColorResult());
    }

    // Render.
    render(wallQuads3D);
    render(wallQuads);
    render(playerTris);
    render(capTris);
    render(capQuads);
}

void HexagonGame::draw()
{
    drawSetup();
    drawFunc();
    drawWrapup();
}

void HexagonGame::initFlashEffect()
{
    flashPolygon.clear();
    flashPolygon.emplace_back(
        sf::Vector2f{-100.f, -100.f}, Color{255, 255, 255, 0});
    flashPolygon.emplace_back(sf::Vector2f{Config::getWidth() + 100.f, -100.f},
        Color{255, 255, 255, 0});
    flashPolygon.emplace_back(
        sf::Vector2f{Config::getWidth() + 100.f, Config::getHeight() + 100.f},
        Color{255, 255, 255, 0});
    flashPolygon.emplace_back(sf::Vector2f{-100.f, Config::getHeight() + 100.f},
        Color{255, 255, 255, 0});
}

void HexagonGame::drawKeyIcons()
{
    constexpr sf::Uint8 offOpacity = 90;
    constexpr sf::Uint8 onOpacity = 255;

    const sf::Color colorText = getColorText();

    const sf::Color offColor{colorText.r, colorText.g, colorText.b, offOpacity};
    const sf::Color onColor{colorText.r, colorText.g, colorText.b, onOpacity};

    keyIconLeft.setColor((getInputMovement() == -1) ? onColor : offColor);
    keyIconRight.setColor((getInputMovement() == 1) ? onColor : offColor);
    keyIconFocus.setColor(getInputFocused() ? onColor : offColor);
    keyIconSwap.setColor(getInputSwap() ? onColor : offColor);

    render(keyIconLeft);
    render(keyIconRight);
    render(keyIconFocus);
    render(keyIconSwap);

    // ------------------------------------------------------------------------

    if(mustShowReplayUI())
    {
        replayIcon.setColor(onColor);
        render(replayIcon);
    }
}

void HexagonGame::updateText()
{
    os.str("");

    if(levelStatus.tutorialMode)
    {
        os << "TUTORIAL MODE\n";
    }
    else if(Config::getOfficial())
    {
        os << "OFFICIAL MODE\n";
    }

    if(Config::getDebug())
    {
        os << "DEBUG MODE\n";
        os << "CUSTOM WALLS: " << cwManager.count() << "\n";
    }

    if(status.started)
    {
        if(levelStatus.swapEnabled)
        {
            os << "SWAP ENABLED\n";
        }

        if(Config::getInvincible())
        {
            os << "INVINCIBILITY ON\n";
        }

        if(Config::getTimescale() != 1.f)
        {
            os << "TIMESCALE " << Config::getTimescale() << "\n";
        }

        if(status.scoreInvalid)
        {
            os << "SCORE INVALIDATED (" << status.invalidReason << ")\n";
        }

        if(status.hasDied)
        {
            os << status.restartInput;
            os << status.replayInput;
        }

        if(calledDeprecatedFunctions.size() > 1)
        {
            os << calledDeprecatedFunctions.size()
               << " WARNINGS RAISED (CHECK CONSOLE)\n";
        }
        else if(calledDeprecatedFunctions.size() > 0)
        {
            os << "1 WARNING RAISED (CHECK CONSOLE)\n";
        }

        const auto& trackedVariables(levelStatus.trackedVariables);
        if(Config::getShowTrackedVariables() && !trackedVariables.empty())
        {
            os << "\n";
            for(const auto& t : trackedVariables)
            {
                if(!lua.doesVariableExist(t.variableName))
                {
                    continue;
                }
                string name{t.displayName};
                string var{lua.readVariable<string>(t.variableName)};
                Utils::uppercasify(name);
                Utils::uppercasify(var);
                os << name << ": " << var << "\n";
            }
        }
    }
    else
    {
        os << "ROTATE TO START\n";
        messageText.setString("ROTATE TO START");
    }

    os.flush();

    // Set in game timer text
    if(!levelStatus.scoreOverridden)
    {
        // By default, use the timer for scoring
        if(status.started)
        {
            timeText.setString(formatTime(status.getTimeSeconds()));
        }
        else
        {
            timeText.setString("0");
        }
    }
    else
    {
        // Alternative scoring
        timeText.setString(
            lua.readVariable<std::string>(levelStatus.scoreOverride));
    }

    const auto getScaledCharacterSize = [&](const float size) {
        return ssvu::toNum<unsigned int>(
            size / Config::getZoomFactor() * Config::getTextScaling());
    };

    timeText.setCharacterSize(getScaledCharacterSize(70.f));
    timeText.setOrigin(0, 0);

    // Set information text
    text.setString(os.str());
    text.setCharacterSize(getScaledCharacterSize(25.f));
    text.setOrigin(0, 0);

    // Set FPS Text, if option is enabled.
    if(Config::getShowFPS())
    {
        fpsText.setString(toStr(window.getFPS()));
        fpsText.setCharacterSize(getScaledCharacterSize(25.f));
        // fpsText.setOrigin(0, 0);
    }

    messageText.setCharacterSize(getScaledCharacterSize(38.f));
    messageText.setOrigin(getGlobalWidth(messageText) / 2.f, 0);

    // ------------------------------------------------------------------------
    if(mustShowReplayUI())
    {
        const replay_file& rf = activeReplay->replayFile;

        os.str("");

        if(!levelStatus.scoreOverridden)
        {
            os << formatTime(rf._played_score / 60.0) << "s";
        }
        else
        {
            os << formatTime(rf._played_score);
        }

        os << " BY " << rf._player_name << '\n'
           << activeReplay->replayPackName << " - "
           << activeReplay->replayLevelName << " (" << rf._difficulty_mult
           << "x)";

        os.flush();

        replayText.setCharacterSize(getScaledCharacterSize(20.f));
        replayText.setString(os.str());
    }
    else
    {
        replayText.setString("");
    }
}

void HexagonGame::drawText_TimeAndStatus(const sf::Color& offsetColor)
{
    if(Config::getDrawTextOutlines())
    {
        timeText.setOutlineColor(offsetColor);
        text.setOutlineColor(offsetColor);
        fpsText.setOutlineColor(offsetColor);
        replayText.setOutlineColor(offsetColor);

        timeText.setOutlineThickness(2.f);
        text.setOutlineThickness(1.f);
        fpsText.setOutlineThickness(1.f);
        replayText.setOutlineThickness(1.f);
    }
    else
    {
        timeText.setOutlineThickness(0.f);
        text.setOutlineThickness(0.f);
        fpsText.setOutlineThickness(0.f);
        replayText.setOutlineThickness(0.f);
    }

    const float padding = Config::getTextPadding() * Config::getTextScaling();
    const float offsetRatio = Config::getHeight() / 720.f;

    const sf::Color colorText = getColorText();

    timeText.setFillColor(colorText);
    timeText.setPosition(
        sf::Vector2f{padding, -22.f * offsetRatio * Config::getTextScaling()});
    render(timeText);

    text.setFillColor(colorText);
    text.setPosition(sf::Vector2f{padding, ssvs::getGlobalBottom(timeText)});
    render(text);

    if(Config::getShowFPS())
    {
        fpsText.setFillColor(colorText);
        fpsText.setOrigin(0, ssvs::getGlobalHeight(fpsText));
        fpsText.setPosition(sf::Vector2f{
            padding, Config::getHeight() - ((8.f * (2.f * offsetRatio))) *
                                               Config::getTextScaling()});
        render(fpsText);
    }

    if(mustShowReplayUI())
    {
        const float scaling =
            Config::getKeyIconsScale() / Config::getZoomFactor();
        const float replayPadding = 8.f * scaling;

        replayText.setFillColor(colorText);
        replayText.setOrigin(ssvs::getLocalCenterE(replayText));
        replayText.setPosition(ssvs::getGlobalCenterW(replayIcon) -
                               sf::Vector2f{replayPadding, 0});
        render(replayText);
    }
}

void HexagonGame::drawText_Message(const sf::Color& offsetColor)
{
    if(messageText.getString() == "")
    {
        return;
    }

    if(Config::getDrawTextOutlines())
    {
        messageText.setOutlineColor(offsetColor);
        messageText.setOutlineThickness(1.f);
    }
    else
    {
        messageText.setOutlineThickness(0.f);
    }

    messageText.setPosition(
        sf::Vector2f{Config::getWidth() / 2.f, Config::getHeight() / 6.f});
    messageText.setFillColor(getColorText());
    render(messageText);
}


void HexagonGame::drawText()
{
    const sf::Color offsetColor{
        Config::getBlackAndWhite() ? Color::Black : getColor(0)};

    drawText_TimeAndStatus(offsetColor);
    drawText_Message(offsetColor);
}

} // namespace hg
