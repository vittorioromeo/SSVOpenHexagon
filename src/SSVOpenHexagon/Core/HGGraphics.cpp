// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include "SSVOpenHexagon/Components/CWall.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Imgui.hpp"

#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"

#include "SSVStart/Utils/SFML.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/Utils/SFML.hpp>

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Utils/Rnd.hpp>

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <cstdint>

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
    if(window == nullptr || Config::getDisableGameRendering())
    {
        return;
    }

    const auto getRenderStates = [this](
                                     const RenderStage rs) -> sf::RenderStates
    {
        if(!Config::getShaders())
        {
            return sf::RenderStates::Default;
        }

        const std::optional<std::size_t> fragmentShaderId =
            status.fragmentShaderIds[static_cast<std::size_t>(rs)];

        if(!fragmentShaderId.has_value())
        {
            return sf::RenderStates::Default;
        }

        runLuaFunctionIfExists<int, float>(
            "onRenderStage", static_cast<int>(rs), 60.f / window->getFPS());
        return sf::RenderStates{assets.getShaderByShaderId(*fragmentShaderId)};
    };

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
        window->setView(backgroundCamera->apply());

        backgroundTris.clear();

        styleData.drawBackground(backgroundTris, ssvs::zeroVec2f,
            levelStatus.sides,
            Config::getDarkenUnevenBackgroundChunk() &&
                levelStatus.darkenUnevenBackgroundChunk,
            Config::getBlackAndWhite());

        render(backgroundTris, getRenderStates(RenderStage::BackgroundTris));
    }

    window->setView(backgroundCamera->apply());

    wallQuads3D.clear();
    pivotQuads3D.clear();
    playerTris3D.clear();
    wallQuads.clear();
    pivotQuads.clear();
    playerTris.clear();
    capTris.clear();

    // Reserve right amount of memory for all walls and custom walls
    wallQuads.reserve_more_quad(walls.size() + cwManager.count());

    for(CWall& w : walls)
    {
        w.draw(getColorWall(), wallQuads);
    }

    cwManager.draw(wallQuads);

    if(status.started)
    {
        player.draw(getSides(), getColorMain(), getColorPlayer(), pivotQuads,
            capTris, playerTris, getColorCap(), Config::getAngleTiltIntensity(),
            Config::getShowSwapBlinkingEffect());
    }

    if(Config::get3D())
    {
        const float depth(styleData._3dDepth);
        const std::size_t numWallQuads(wallQuads.size());
        const std::size_t numPivotQuads(pivotQuads.size());
        const std::size_t numPlayerTris(playerTris.size());

        wallQuads3D.reserve(numWallQuads * depth);
        pivotQuads3D.reserve(numPivotQuads * depth);
        playerTris3D.reserve(numPlayerTris * depth);

        const float pulse3D{Config::getNoPulse() ? 1.f : status.pulse3D};
        const float effect{
            styleData._3dSkew * Config::get3DMultiplier() * pulse3D};

        const sf::Vector2f skew{1.f, 1.f + effect};
        backgroundCamera->setSkew(skew);

        const float radRot(
            ssvu::toRad(backgroundCamera->getRotation()) + (ssvu::pi / 2.f));
        const float sinRot(std::sin(radRot));
        const float cosRot(std::cos(radRot));

        for(std::size_t i = 0; i < depth; ++i)
        {
            wallQuads3D.unsafe_emplace_other(wallQuads);
            pivotQuads3D.unsafe_emplace_other(pivotQuads);
            playerTris3D.unsafe_emplace_other(playerTris);
        }

        const auto adjustAlpha = [&](sf::Color& c, const float i)
        {
            if(styleData._3dAlphaMult == 0.f)
            {
                return;
            }

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

            sf::Color overrideColor;

            if(!Config::getBlackAndWhite())
            {
                overrideColor = Utils::getColorDarkened(
                    styleData.get3DOverrideColor(), styleData._3dDarkenMult);
            }
            else
            {
                overrideColor = Utils::getColorDarkened(
                    sf::Color(255, 255, 255, styleData.getMainColor().a),
                    styleData._3dDarkenMult);
            }
            adjustAlpha(overrideColor, i);

            // Draw pivot layers
            for(std::size_t k = j * numPivotQuads; k < (j + 1) * numPivotQuads;
                ++k)
            {
                pivotQuads3D[k].position += newPos;
                pivotQuads3D[k].color = overrideColor;
            }

            if(styleData.get3DOverrideColor() == styleData.getMainColor())
            {
                overrideColor = Utils::getColorDarkened(
                    getColorWall(), styleData._3dDarkenMult);

                adjustAlpha(overrideColor, i);
            }

            // Draw wall layers
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
                    getColorPlayer(), styleData._3dDarkenMult);

                adjustAlpha(overrideColor, i);
            }

            // Draw player layers
            for(std::size_t k = j * numPlayerTris; k < (j + 1) * numPlayerTris;
                ++k)
            {
                playerTris3D[k].position += newPos;
                playerTris3D[k].color = overrideColor;
            }
        }
    }

    render(wallQuads3D, getRenderStates(RenderStage::WallQuads3D));
    render(pivotQuads3D, getRenderStates(RenderStage::PivotQuads3D));
    render(playerTris3D, getRenderStates(RenderStage::PlayerTris3D));

    if(Config::getShowPlayerTrail() && status.showPlayerTrail)
    {
        drawTrailParticles();
    }

    if(Config::getShowSwapParticles())
    {
        drawSwapParticles();
    }

    render(wallQuads, getRenderStates(RenderStage::WallQuads));
    render(capTris, getRenderStates(RenderStage::CapTris));
    render(pivotQuads, getRenderStates(RenderStage::PivotQuads));
    render(playerTris, getRenderStates(RenderStage::PlayerTris));

    window->setView(overlayCamera->apply());

    drawParticles();
    drawText(getRenderStates(RenderStage::Text));

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
        drawLevelInfo(getRenderStates(RenderStage::Text));
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

    sf::RenderWindow& renderWindow = window->getRenderWindow();
    window->setView(renderWindow.getDefaultView());

    Imgui::render(*window);
}

void HexagonGame::initFlashEffect(int r, int g, int b)
{
    flashPolygon.clear();
    flashPolygon.reserve(6);

    const sf::Color color{static_cast<std::uint8_t>(r),
        static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), 0};

    const auto width = static_cast<float>(Config::getWidth());
    const auto height = static_cast<float>(Config::getHeight());
    const float offset = 100.f;

    const sf::Vector2f nw{-offset, -offset};
    const sf::Vector2f sw{-offset, height + offset};
    const sf::Vector2f se{width + offset, height + offset};
    const sf::Vector2f ne{width + offset, -offset};

    flashPolygon.batch_unsafe_emplace_back_quad(color, nw, sw, se, ne);
}

void HexagonGame::drawKeyIcons()
{
    constexpr std::uint8_t offOpacity = 90;
    constexpr std::uint8_t onOpacity = 255;

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

void HexagonGame::drawLevelInfo(const sf::RenderStates& mStates)
{
    render(levelInfoRectangle, mStates);
    render(levelInfoTextLevel, mStates);
    render(levelInfoTextPack, mStates);
    render(levelInfoTextAuthor, mStates);
    render(levelInfoTextBy, mStates);
    render(levelInfoTextDM, mStates);
}

void HexagonGame::drawParticles()
{
    for(Particle& p : particles)
    {
        render(p.sprite);
    }
}

void HexagonGame::drawTrailParticles()
{
    for(TrailParticle& p : trailParticles)
    {
        render(p.sprite);
    }
}

void HexagonGame::drawSwapParticles()
{
    for(SwapParticle& p : swapParticles)
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

    // Set information text
    text.setString(os.str());
    text.setCharacterSize(getScaledCharacterSize(20.f));
    text.setOrigin({0.f, 0.f});

    // Set FPS Text, if option is enabled.
    if(Config::getShowFPS())
    {
        fpsText.setString(ssvu::toStr(window->getFPS()));
        fpsText.setCharacterSize(getScaledCharacterSize(20.f));
    }

    messageText.setCharacterSize(getScaledCharacterSize(32.f));
    messageText.setOrigin({ssvs::getGlobalWidth(messageText) / 2.f, 0.f});

    const float growth = std::sin(pbTextGrowth);
    pbText.setCharacterSize(getScaledCharacterSize(64.f) + growth * 10.f);
    pbText.setOrigin({ssvs::getGlobalWidth(pbText) / 2.f, 0.f});

    // ------------------------------------------------------------------------
    if(mustShowReplayUI())
    {
        const replay_file& rf = activeReplay->replayFile;

        os.str("");

        if(!levelStatus.scoreOverridden)
        {
            os << formatTime(rf.played_seconds()) << "s";
        }
        else
        {
            os << formatTime(rf._played_score);
        }

        os << " BY " << rf._player_name;

        os.flush();

        replayText.setCharacterSize(getScaledCharacterSize(16.f));
        replayText.setString(os.str());
    }
    else
    {
        replayText.setString("");
    }
}

void HexagonGame::drawText_TimeAndStatus(
    const sf::Color& offsetColor, const sf::RenderStates& mStates)
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

    const float padding =
        (Config::getTextPadding() * Config::getTextScaling()) /
        Config::getZoomFactor();

    const sf::Color colorText = getColorText();

    if(Config::getShowTimer())
    {
        timeText.setFillColor(colorText);
        timeText.setOrigin(ssvs::getLocalNW(timeText));
        timeText.setPosition({padding, padding});

        render(timeText, mStates);
    }

    if(Config::getShowStatusText())
    {
        text.setFillColor(colorText);
        text.setOrigin(ssvs::getLocalNW(text));
        text.setPosition({padding, ssvs::getGlobalBottom(timeText) + padding});

        render(text, mStates);
    }

    if(Config::getShowFPS())
    {
        fpsText.setFillColor(colorText);
        fpsText.setOrigin(ssvs::getLocalSW(fpsText));

        if(Config::getShowLevelInfo() || mustShowReplayUI())
        {
            fpsText.setPosition(
                {padding, ssvs::getGlobalTop(levelInfoRectangle) - padding});
        }
        else
        {
            fpsText.setPosition({padding, Config::getHeight() - padding});
        }

        render(fpsText, mStates);
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
        render(replayText, mStates);
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

void HexagonGame::drawText_Message(
    const sf::Color& offsetColor, const sf::RenderStates& mStates)
{
    drawTextMessagePBImpl(messageText, offsetColor,
        {Config::getWidth() / 2.f, Config::getHeight() / 5.5f}, getColorText(),
        1.f /* outlineThickness */,
        [this, &mStates](sf::Text& t) { render(t, mStates); });
}

void HexagonGame::drawText_PersonalBest(
    const sf::Color& offsetColor, const sf::RenderStates& mStates)
{
    drawTextMessagePBImpl(pbText, offsetColor,
        {Config::getWidth() / 2.f,
            Config::getHeight() - Config::getHeight() / 4.f},
        getColorText(), 4.f /* outlineThickness */,
        [this, &mStates](sf::Text& t) { render(t, mStates); });
}

void HexagonGame::drawText(const sf::RenderStates& mStates)
{
    const sf::Color offsetColor{
        Config::getBlackAndWhite() || styleData.getColors().empty()
            ? sf::Color::Black
            : getColor(0)};

    drawText_TimeAndStatus(offsetColor, mStates);
    drawText_Message(offsetColor, mStates);
    drawText_PersonalBest(offsetColor, mStates);
}

} // namespace hg
