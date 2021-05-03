// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/Utils/SFML.hpp>

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Utils/Rnd.hpp>

namespace hg {

[[nodiscard]] static std::string formatTime(const double x)
{
    return ssvu::toStr(std::floor(x * 1000) / 1000.f);
}

void HexagonGame::render(
    sf::Drawable& mDrawable, const sf::RenderStates& mStates)
{
    if(window == nullptr)
    {
        ssvu::lo("hg::HexagonGame::render")
            << "Attempted to render without a game window\n";

        return;
    }

    window->draw(mDrawable, mStates);
}

void HexagonGame::draw()
{
    if(window == nullptr)
    {
        return;
    }

    SSVOH_ASSERT(backgroundCamera.has_value());
    SSVOH_ASSERT(overlayCamera.has_value());

    window->clear(sf::Color::Black);

    if(!status.hasDied)
    {
        if(levelStatus.cameraShake > 0.f)
        {
            const sf::Vector2f shake(ssvu::getRndR(-levelStatus.cameraShake,
                                         levelStatus.cameraShake),
                ssvu::getRndR(
                    -levelStatus.cameraShake, levelStatus.cameraShake));

            backgroundCamera->setCenter(shake);
            overlayCamera->setCenter(
                shake + sf::Vector2f{Config::getWidth() / 2.f,
                            Config::getHeight() / 2.f});
        }
        else
        {
            backgroundCamera->setCenter(ssvs::zeroVec2f);
            overlayCamera->setCenter(sf::Vector2f{
                Config::getWidth() / 2.f, Config::getHeight() / 2.f});
        }
    }

    if(!Config::getNoBackground())
    {
        backgroundCamera->apply();

        backgroundTris.clear();

        styleData.drawBackground(backgroundTris, ssvs::zeroVec2f,
            levelStatus.sides,
            Config::getDarkenUnevenBackgroundChunk() &&
                levelStatus.darkenUnevenBackgroundChunk,
            Config::getBlackAndWhite());

        render(backgroundTris);
    }

    backgroundCamera->apply();

    wallQuads3D.clear();
    playerTris3D.clear();
    wallQuads.clear();
    playerTris.clear();
    capTris.clear();

    // Reserve right amount of memory for all walls and custom walls
    wallQuads.reserve_more(4 * walls.size() + 4 * cwManager.count());

    for(CWall& w : walls)
    {
        w.draw(getColorMain(), wallQuads);
    }

    cwManager.draw(wallQuads);

    if(status.started)
    {
        player.draw(getSides(), getColorMain(), getColorPlayer(), wallQuads,
            capTris, playerTris, styleData.getCapColorResult());
    }

    if(Config::get3D())
    {
        const float depth(styleData._3dDepth);
        const std::size_t numWallQuads(wallQuads.size());
        const std::size_t numPlayerTris(playerTris.size());

        wallQuads3D.reserve(numWallQuads * depth);
        playerTris3D.reserve(numPlayerTris * depth);

        const float effect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};

        const sf::Vector2f skew{1.f, 1.f + effect};
        backgroundCamera->setSkew(skew);

        const float radRot(
            ssvu::toRad(backgroundCamera->getRotation()) + (ssvu::pi / 2.f));
        const float sinRot(std::sin(radRot));
        const float cosRot(std::cos(radRot));

        for(std::size_t i = 0; i < depth; ++i)
        {
            wallQuads3D.unsafe_emplace_other(wallQuads);
        }

        for(std::size_t i = 0; i < depth; ++i)
        {
            playerTris3D.unsafe_emplace_other(playerTris);
        }

        const auto adjustAlpha = [&](sf::Color& c, const float i)
        {
            SSVOH_ASSERT(styleData._3dAlphaMult != 0.f);

            const float newAlpha =
                (static_cast<float>(c.a) / styleData._3dAlphaMult) -
                i * styleData._3dAlphaFalloff;

            c.a = Utils::componentClamp(newAlpha);
        };

        for(int j(0); j < static_cast<int>(depth); ++j)
        {
            const float i(depth - j - 1);

            const float offset(styleData._3dSpacing *
                               (float(i + 1.f) * styleData._3dPerspectiveMult) *
                               (effect * 3.6f) * 1.4f);

            const sf::Vector2f newPos(offset * cosRot, offset * sinRot);

            sf::Color overrideColor{Utils::getColorDarkened(
                styleData.get3DOverrideColor(), styleData._3dDarkenMult)};

            adjustAlpha(overrideColor, i);

            for(std::size_t k = j * numWallQuads; k < (j + 1) * numWallQuads;
                ++k)
            {
                wallQuads3D[k].position += newPos;
                wallQuads3D[k].color = overrideColor;
            }

            // Apply player color if no 3D override is present.
            if(styleData.get3DOverrideColor() == styleData.getMainColor())
            {
                overrideColor = Utils::getColorDarkened(
                    styleData.getPlayerColor(), styleData._3dDarkenMult);

                adjustAlpha(overrideColor, i);
            }

            for(std::size_t k = j * numPlayerTris; k < (j + 1) * numPlayerTris;
                ++k)
            {
                playerTris3D[k].position += newPos;
                playerTris3D[k].color = overrideColor;
            }
        }
    }

    render(wallQuads3D);
    render(playerTris3D);
    render(wallQuads);
    render(playerTris);
    render(capTris);

    overlayCamera->apply();

    drawParticles();
    drawText();

    // ------------------------------------------------------------------------
    // Draw key icons.
    if(Config::getShowKeyIcons() || mustShowReplayUI())
    {
        drawKeyIcons();
    }

    // ------------------------------------------------------------------------
    // Draw level info.
    if(Config::getShowLevelInfo() || mustShowReplayUI())
    {
        drawLevelInfo();
    }

    // ------------------------------------------------------------------------
    if(Config::getFlash())
    {
        render(flashPolygon);
    }

    if(mustTakeScreenshot)
    {
        if(window != nullptr)
        {
            window->saveScreenshot("screenshot.png");
        }

        mustTakeScreenshot = false;
    }

    drawImguiLuaConsole();
}

void HexagonGame::drawImguiLuaConsole()
{
    if(window == nullptr)
    {
        return;
    }

    if(!ilcShowConsole)
    {
        return;
    }

    SSVOH_ASSERT(overlayCamera.has_value());
    overlayCamera->unapply();

    ImGui::SFML::Render(*window);
}

void HexagonGame::initFlashEffect()
{
    flashPolygon.clear();

    flashPolygon.emplace_back(
        sf::Vector2f{-100.f, -100.f}, sf::Color{255, 255, 255, 0});

    flashPolygon.emplace_back(sf::Vector2f{Config::getWidth() + 100.f, -100.f},
        sf::Color{255, 255, 255, 0});

    flashPolygon.emplace_back(
        sf::Vector2f{Config::getWidth() + 100.f, Config::getHeight() + 100.f},
        sf::Color{255, 255, 255, 0});

    flashPolygon.emplace_back(sf::Vector2f{-100.f, Config::getHeight() + 100.f},
        sf::Color{255, 255, 255, 0});
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

void HexagonGame::drawLevelInfo()
{
    render(levelInfoRectangle);
    render(levelInfoTextLevel);
    render(levelInfoTextPack);
    render(levelInfoTextAuthor);
    render(levelInfoTextBy);
    render(levelInfoTextDM);
}

void HexagonGame::drawParticles()
{
    for(Particle& p : particles)
    {
        render(p.sprite);
    }
}

void HexagonGame::updateText(ssvu::FT mFT)
{
    if(window == nullptr)
    {
        return;
    }

    // ------------------------------------------------------------------------
    // Update "personal best" text animation.
    pbTextGrowth += 0.08f * mFT;
    if(pbTextGrowth > ssvu::pi * 2.f)
    {
        pbTextGrowth = 0;
    }

    // ------------------------------------------------------------------------
    os.str("");

    if(debugPause)
    {
        os << "(!) PAUSED (!)\n";
    }

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

        os << "CUSTOM WALLS: " << cwManager.count() << " / "
           << cwManager.maxHandles() << '\n';
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

        if(const float timescale = Config::getTimescale(); timescale != 1.f)
        {
            os << "TIMESCALE " << timescale << '\n';
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
            os << '\n';
            for(const auto& [variableName, display] : trackedVariables)
            {
                if(!lua.doesVariableExist(variableName))
                {
                    continue;
                }

                const std::string value{
                    lua.readVariable<std::string>(variableName)};

                os << Utils::toUppercase(display) << ": "
                   << Utils::toUppercase(value) << '\n';
            }
        }
    }
    else if(Config::getRotateToStart())
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

    const auto getScaledCharacterSize = [&](const float size)
    {
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
        fpsText.setString(ssvu::toStr(window->getFPS()));
        fpsText.setCharacterSize(getScaledCharacterSize(25.f));
    }

    messageText.setCharacterSize(getScaledCharacterSize(38.f));
    messageText.setOrigin(ssvs::getGlobalWidth(messageText) / 2.f, 0);

    const float growth = std::sin(pbTextGrowth);
    pbText.setCharacterSize(getScaledCharacterSize(64.f) + growth * 10.f);
    pbText.setOrigin(ssvs::getGlobalWidth(pbText) / 2.f, 0);

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

        os << " BY " << rf._player_name;

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

    if(Config::getShowTimer())
    {
        timeText.setFillColor(colorText);
        timeText.setPosition(sf::Vector2f{
            padding, -22.f * offsetRatio * Config::getTextScaling()});
        render(timeText);
    }

    if(Config::getShowStatusText())
    {
        text.setFillColor(colorText);
        text.setPosition(
            sf::Vector2f{padding, ssvs::getGlobalBottom(timeText)});
        render(text);
    }

    if(Config::getShowFPS())
    {
        fpsText.setFillColor(colorText);
        fpsText.setOrigin(0, ssvs::getGlobalHeight(fpsText));

        if(Config::getShowLevelInfo() || mustShowReplayUI())
        {
            fpsText.setPosition(sf::Vector2f{padding,
                ssvs::getGlobalTop(levelInfoRectangle) -
                    ((8.f * (2.f * offsetRatio))) * Config::getTextScaling()});
        }
        else
        {
            fpsText.setPosition(sf::Vector2f{
                padding, Config::getHeight() - ((8.f * (2.f * offsetRatio))) *
                                                   Config::getTextScaling()});
        }

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

template <typename FRender>
static void drawTextMessagePBImpl(sf::Text& text, const sf::Color& offsetColor,
    const sf::Vector2f& pos, const sf::Color& color, float outlineThickness,
    FRender&& fRender)
{
    if(text.getString().isEmpty())
    {
        return;
    }

    if(Config::getDrawTextOutlines())
    {
        text.setOutlineColor(offsetColor);
        text.setOutlineThickness(outlineThickness);
    }
    else
    {
        text.setOutlineThickness(0.f);
    }

    text.setPosition(pos);
    text.setFillColor(color);

    fRender(text);
}

void HexagonGame::drawText_Message(const sf::Color& offsetColor)
{
    drawTextMessagePBImpl(messageText, offsetColor,
        {Config::getWidth() / 2.f, Config::getHeight() / 6.f}, getColorText(),
        1.f /* outlineThickness */, [this](sf::Text& t) { render(t); });
}

void HexagonGame::drawText_PersonalBest(const sf::Color& offsetColor)
{
    drawTextMessagePBImpl(pbText, offsetColor,
        {Config::getWidth() / 2.f,
            Config::getHeight() - Config::getHeight() / 4.f},
        getColorText(), 4.f /* outlineThickness */,
        [this](sf::Text& t) { render(t); });
}

void HexagonGame::drawText()
{
    const sf::Color offsetColor{
        Config::getBlackAndWhite() || styleData.getColors().empty()
            ? sf::Color::Black
            : getColor(0)};

    drawText_TimeAndStatus(offsetColor);
    drawText_Message(offsetColor);
    drawText_PersonalBest(offsetColor);
}

} // namespace hg
