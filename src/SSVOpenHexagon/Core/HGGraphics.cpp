// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

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

void HexagonGame::draw()
{
    styleData.computeColors(levelStatus);

    window.clear(Color::Black);

    if(!status.hasDied)
    {
        if(levelStatus.cameraShake > 0)
        {
            const sf::Vector2f shake(
                getRndI(-levelStatus.cameraShake, levelStatus.cameraShake),
                getRndI(-levelStatus.cameraShake, levelStatus.cameraShake));

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

    if(!Config::getNoBackground())
    {
        backgroundCamera.apply();
        styleData.drawBackground(window, ssvs::zeroVec2f, levelStatus);
    }

    backgroundCamera.apply();

    wallQuads3D.clear();
    playerTris3D.clear();
    wallQuads.clear();
    playerTris.clear();
    capTris.clear();

    for(CWall& w : walls)
    {
        w.draw(*this);
    }

    cwManager.draw(*this);

    if(status.started)
    {
        player.draw(*this, styleData.getCapColorResult());
    }

    if(Config::get3D())
    {
        const auto depth(styleData._3dDepth);
        const auto numWallQuads(wallQuads.size());
        const auto numPlayerTris(playerTris.size());

        wallQuads3D.reserve(numWallQuads * depth);
        playerTris3D.reserve(numPlayerTris * depth);

        const float effect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};

        const sf::Vector2f skew{1.f, 1.f + effect};
        backgroundCamera.setSkew(skew);

        const auto radRot(
            ssvu::toRad(backgroundCamera.getRotation()) + (ssvu::pi / 2.f));
        const auto sinRot(std::sin(radRot));
        const auto cosRot(std::cos(radRot));

        for(std::size_t i = 0; i < depth; ++i)
        {
            wallQuads3D.unsafe_emplace_other(wallQuads);
        }

        for(std::size_t i = 0; i < depth; ++i)
        {
            playerTris3D.unsafe_emplace_other(playerTris);
        }

        for(auto j(0); j < depth; ++j)
        {
            const float i(depth - j - 1);

            const float offset(styleData._3dSpacing *
                               (float(i + 1.f) * styleData._3dPerspectiveMult) *
                               (effect * 3.6f) * 1.4f);

            sf::Vector2f newPos(offset * cosRot, offset * sinRot);

            status.overrideColor = getColorDarkened(
                styleData.get3DOverrideColor(), styleData._3dDarkenMult);
            status.overrideColor.a /= styleData._3dAlphaMult;
            status.overrideColor.a -= i * styleData._3dAlphaFalloff;

            for(std::size_t k = j * numWallQuads; k < (j + 1) * numWallQuads;
                ++k)
            {
                wallQuads3D[k].position += newPos;
                wallQuads3D[k].color = status.overrideColor;
            }

            for(std::size_t k = j * numPlayerTris; k < (j + 1) * numPlayerTris;
                ++k)
            {
                playerTris3D[k].position += newPos;
                playerTris3D[k].color = status.overrideColor;
            }
        }
    }

    render(wallQuads3D);
    render(playerTris3D);
    render(wallQuads);
    render(playerTris);
    render(capTris);

    overlayCamera.apply();
    drawText();

    if(Config::getShowKeyIcons())
    {
        drawKeyIcons();
    }

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

    const sf::Color c = getColorMain();

    const sf::Color offColor{c.r, c.g, c.b, offOpacity};
    const sf::Color onColor{c.r, c.g, c.b, onOpacity};

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
            os << "PRESS R TO RESTART\n";
            os << "PRESS Y TO VIEW REPLAY\n";
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
    if(status.started)
    {
        timeText.setString(formatTime(status.getTimeSeconds()));
    }
    else
    {
        timeText.setString("0");
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

        os << formatTime(rf._played_frametime / 60.0) << "s BY "
           << rf._player_name << '\n';

        os << activeReplay->replayPackName << " - "
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

    timeText.setFillColor(getColorMain());
    timeText.setPosition(
        sf::Vector2f{padding, -22.f * offsetRatio * Config::getTextScaling()});
    render(timeText);

    text.setFillColor(getColorMain());
    text.setPosition(sf::Vector2f{padding, ssvs::getGlobalBottom(timeText)});
    render(text);

    if(Config::getShowFPS())
    {
        fpsText.setFillColor(getColorMain());
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
        const float padding = 8.f * scaling;

        replayText.setFillColor(getColorMain());
        replayText.setOrigin(ssvs::getLocalCenterE(replayText));
        replayText.setPosition(
            ssvs::getGlobalCenterW(replayIcon) - sf::Vector2f{padding, 0});
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
    messageText.setFillColor(getColorMain());
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
