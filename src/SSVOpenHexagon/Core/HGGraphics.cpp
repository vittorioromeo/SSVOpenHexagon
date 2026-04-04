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
#include "SSVOpenHexagon/Utils/Math.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"

#include "SSVOpenHexagon/Utils/Log.hpp"
#include <SSVUtils/Core/Utils/Rnd.hpp>
#include <SSVUtils/Core/String/ToStr.hpp>

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <SFML/Base/IntTypes.hpp>

namespace hg {

[[nodiscard]] static std::string formatTime(const double x)
{
    return ssvu::toStr(std::floor(x * 1000) / 1000.f);
}

static void setVisualCharacterSize(sf::Text& text, const float characterSize)
{
    const float baseSize = static_cast<float>(text.getCharacterSize());
    const float scale = characterSize / baseSize;
    text.scale = {scale, scale};
}

template <typename TDrawable>
void HexagonGame::renderWithView(const sf::View& view, TDrawable&& drawable)
{
    if (window == nullptr)
    {
        hg::lo("hg::HexagonGame::renderWithView")
            << "Attempted to render without a game window\n";

        return;
    }

    sf::RenderStates states;
    states.view = view;
    window->getRenderWindow().draw(SSVOH_FWD(drawable), states);
}

template <typename TDrawable>
void HexagonGame::renderWithView(
    const sf::View& view, TDrawable&& drawable, sf::RenderStates states)
{
    if (window == nullptr)
    {
        hg::lo("hg::HexagonGame::renderWithView")
            << "Attempted to render without a game window\n";

        return;
    }

    states.view = view;
    window->getRenderWindow().draw(SSVOH_FWD(drawable), states);
}

void HexagonGame::draw()
{
    if (window == nullptr || Config::getDisableGameRendering())
    {
        return;
    }

    const auto getRenderStates = [this](
                                     const RenderStage rs) -> sf::RenderStates
    {
        if (!Config::getShaders())
        {
            return sf::RenderStates{};
        }

        const sf::base::Optional<sf::base::SizeT> fragmentShaderId =
            status.fragmentShaderIds[static_cast<sf::base::SizeT>(rs)];

        if (!fragmentShaderId.hasValue())
        {
            return sf::RenderStates{};
        }

        runLuaFunctionIfExists<int, float>(
            "onRenderStage", static_cast<int>(rs), 60.f / window->getFPS());
        return sf::RenderStates{
            .shader = assets.getShaderByShaderId(*fragmentShaderId)};
    };

    SSVOH_ASSERT(backgroundCamera.hasValue());
    SSVOH_ASSERT(overlayCamera.hasValue());

    window->clear(sf::Color::Black);

    if (!status.hasDied)
    {
        if (levelStatus.cameraShake > 0.f)
        {
            const sf::Vec2f shake(ssvu::getRndR(-levelStatus.cameraShake,
                                      levelStatus.cameraShake),
                ssvu::getRndR(
                    -levelStatus.cameraShake, levelStatus.cameraShake));

            backgroundCamera->center = shake;
            overlayCamera->center = shake + sf::Vec2f{Config::getWidth() / 2.f,
                                                Config::getHeight() / 2.f};
        }
        else
        {
            backgroundCamera->center = sf::Vec2f{0.f, 0.f};
            overlayCamera->center =
                sf::Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 2.f};
        }
    }

    backgroundCameraTransform.skew = sf::Vec2f{1.f, 1.f};

    if (Config::get3D())
    {
        const float pulse3D{Config::getNoPulse() ? 1.f : status.pulse3D};
        const float effect{
            styleData._3dSkew * Config::get3DMultiplier() * pulse3D};

        backgroundCameraTransform.skew = sf::Vec2f{1.f, 1.f + effect};
    }

    const sf::View backgroundView =
        Utils::computeCameraView(*backgroundCamera, backgroundCameraTransform);

    if (!Config::getNoBackground())
    {
        backgroundTris.clear();

        styleData.drawBackground(backgroundTris, sf::Vec2f{0.f, 0.f},
            levelStatus.sides,
            Config::getDarkenUnevenBackgroundChunk() &&
                levelStatus.darkenUnevenBackgroundChunk,
            Config::getBlackAndWhite());

        renderWithView(backgroundView, backgroundTris,
            getRenderStates(RenderStage::BackgroundTris));
    }

    wallQuads3D.clear();
    pivotQuads3D.clear();
    playerTris3D.clear();
    wallQuads.clear();
    pivotQuads.clear();
    playerTris.clear();
    capTris.clear();

    // Reserve right amount of memory for all walls and custom walls
    wallQuads.reserve_more_quad(walls.size() + cwManager.count());

    for (CWall& w : walls)
    {
        w.draw(getColorWall(), wallQuads);
    }

    cwManager.draw(wallQuads);

    if (status.started)
    {
        player.draw(getSides(), getColorMain(), getColorPlayer(), pivotQuads,
            capTris, playerTris, getColorCap(), Config::getAngleTiltIntensity(),
            Config::getShowSwapBlinkingEffect());
    }

    if (Config::get3D())
    {
        const float depth(styleData._3dDepth);
        const sf::base::SizeT numWallQuads(wallQuads.size());
        const sf::base::SizeT numPivotQuads(pivotQuads.size());
        const sf::base::SizeT numPlayerTris(playerTris.size());

        wallQuads3D.reserve(numWallQuads * depth);
        pivotQuads3D.reserve(numPivotQuads * depth);
        playerTris3D.reserve(numPlayerTris * depth);

        const float effect{styleData._3dSkew * Config::get3DMultiplier() *
                           (Config::getNoPulse() ? 1.f : status.pulse3D)};

        const float radRot(
            Utils::toRad(backgroundCamera->rotation.asDegrees()) +
            (Utils::pi / 2.f));
        const float sinRot(std::sin(radRot));
        const float cosRot(std::cos(radRot));

        for (sf::base::SizeT i = 0; i < depth; ++i)
        {
            wallQuads3D.unsafe_emplace_other(wallQuads);
            pivotQuads3D.unsafe_emplace_other(pivotQuads);
            playerTris3D.unsafe_emplace_other(playerTris);
        }

        const auto adjustAlpha = [&](sf::Color& c, const float i)
        {
            if (styleData._3dAlphaMult == 0.f)
            {
                return;
            }

            const float newAlpha =
                (static_cast<float>(c.a) / styleData._3dAlphaMult) -
                i * styleData._3dAlphaFalloff;

            c.a = Utils::componentClamp(newAlpha);
        };

        for (int j(0); j < static_cast<int>(depth); ++j)
        {
            const float i(depth - j - 1);

            const float offset(styleData._3dSpacing *
                               (float(i + 1.f) * styleData._3dPerspectiveMult) *
                               (effect * 3.6f) * 1.4f);

            const sf::Vec2f newPos(offset * cosRot, offset * sinRot);

            sf::Color overrideColor;

            if (!Config::getBlackAndWhite())
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
            for (sf::base::SizeT k = j * numPivotQuads;
                k < (j + 1) * numPivotQuads; ++k)
            {
                pivotQuads3D[k].position += newPos;
                pivotQuads3D[k].color = overrideColor;
            }

            if (styleData.get3DOverrideColor() == styleData.getMainColor())
            {
                overrideColor = Utils::getColorDarkened(
                    getColorWall(), styleData._3dDarkenMult);

                adjustAlpha(overrideColor, i);
            }

            // Draw wall layers
            for (sf::base::SizeT k = j * numWallQuads;
                k < (j + 1) * numWallQuads; ++k)
            {
                wallQuads3D[k].position += newPos;
                wallQuads3D[k].color = overrideColor;
            }

            // Apply player color if no 3D override is present.
            if (styleData.get3DOverrideColor() == styleData.getMainColor())
            {
                overrideColor = Utils::getColorDarkened(
                    getColorPlayer(), styleData._3dDarkenMult);

                adjustAlpha(overrideColor, i);
            }

            // Draw player layers
            for (sf::base::SizeT k = j * numPlayerTris;
                k < (j + 1) * numPlayerTris; ++k)
            {
                playerTris3D[k].position += newPos;
                playerTris3D[k].color = overrideColor;
            }
        }
    }

    renderWithView(
        backgroundView, wallQuads3D, getRenderStates(RenderStage::WallQuads3D));
    renderWithView(backgroundView, pivotQuads3D,
        getRenderStates(RenderStage::PivotQuads3D));
    renderWithView(backgroundView, playerTris3D,
        getRenderStates(RenderStage::PlayerTris3D));

    if (Config::getShowPlayerTrail() && status.showPlayerTrail)
    {
        drawTrailParticles();
    }

    if (Config::getShowSwapParticles())
    {
        drawSwapParticles();
    }

    renderWithView(
        backgroundView, wallQuads, getRenderStates(RenderStage::WallQuads));
    renderWithView(
        backgroundView, capTris, getRenderStates(RenderStage::CapTris));
    renderWithView(
        backgroundView, pivotQuads, getRenderStates(RenderStage::PivotQuads));
    renderWithView(
        backgroundView, playerTris, getRenderStates(RenderStage::PlayerTris));

    drawParticles();
    drawText(getRenderStates(RenderStage::Text));

    // ------------------------------------------------------------------------
    // Draw key icons.
    if (Config::getShowKeyIcons() || mustShowReplayUI())
    {
        drawKeyIcons();
    }

    // ------------------------------------------------------------------------
    // Draw level info.
    if (Config::getShowLevelInfo() || mustShowReplayUI())
    {
        drawLevelInfo(getRenderStates(RenderStage::Text));
    }

    // ------------------------------------------------------------------------
    if (Config::getFlash())
    {
        renderWithView(
            Utils::computeCameraView(*overlayCamera, overlayCameraTransform),
            flashPolygon);
    }

    if (mustTakeScreenshot)
    {
        if (window != nullptr)
        {
            window->saveScreenshot("screenshot.png");
        }

        mustTakeScreenshot = false;
    }

    drawImguiLuaConsole();
}

void HexagonGame::drawImguiLuaConsole()
{
    if (window == nullptr)
    {
        return;
    }

    if (!ilcShowConsole)
    {
        return;
    }

    SSVOH_ASSERT(overlayCamera.hasValue());

    sf::RenderWindow& renderWindow = window->getRenderWindow();

    SSVOH_ASSERT(imguiCtx.hasValue());
    imguiCtx->render(renderWindow);
}

void HexagonGame::initFlashEffect(int r, int g, int b)
{
    flashPolygon.clear();
    flashPolygon.reserve(6);

    const sf::Color color{static_cast<sf::base::U8>(r),
        static_cast<sf::base::U8>(g), static_cast<sf::base::U8>(b), 0};

    const auto width = static_cast<float>(Config::getWidth());
    const auto height = static_cast<float>(Config::getHeight());
    const float offset = 100.f;

    const sf::Vec2f nw{-offset, -offset};
    const sf::Vec2f sw{-offset, height + offset};
    const sf::Vec2f se{width + offset, height + offset};
    const sf::Vec2f ne{width + offset, -offset};

    flashPolygon.batch_unsafe_emplace_back_quad(color, nw, sw, se, ne);
}

void HexagonGame::drawKeyIcons()
{
    constexpr sf::base::U8 offOpacity = 90;
    constexpr sf::base::U8 onOpacity = 255;
    const sf::View overlayView =
        Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    const sf::Color colorText = getColorText();

    const sf::Color offColor{colorText.r, colorText.g, colorText.b, offOpacity};
    const sf::Color onColor{colorText.r, colorText.g, colorText.b, onOpacity};

    keyIconLeft.color = (getInputMovement() == -1) ? onColor : offColor;
    keyIconRight.color = (getInputMovement() == 1) ? onColor : offColor;
    keyIconFocus.color = getInputFocused() ? onColor : offColor;
    keyIconSwap.color = getInputSwap() ? onColor : offColor;

    renderWithView(
        overlayView, keyIconLeft, sf::RenderStates{.texture = txKeyIconLeft});
    renderWithView(
        overlayView, keyIconRight, sf::RenderStates{.texture = txKeyIconRight});
    renderWithView(
        overlayView, keyIconFocus, sf::RenderStates{.texture = txKeyIconFocus});
    renderWithView(
        overlayView, keyIconSwap, sf::RenderStates{.texture = txKeyIconSwap});

    // ------------------------------------------------------------------------

    if (mustShowReplayUI())
    {
        replayIcon.color = onColor;
        renderWithView(
            overlayView, replayIcon, sf::RenderStates{.texture = txReplayIcon});
    }
}

void HexagonGame::drawLevelInfo(const sf::RenderStates& mStates)
{
    const sf::View overlayView =
        Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    renderWithView(overlayView, levelInfoRectangle, mStates);

    if (textUI.hasValue())
    {
        renderWithView(overlayView, textUI->levelInfoTextLevel, mStates);
        renderWithView(overlayView, textUI->levelInfoTextPack, mStates);
        renderWithView(overlayView, textUI->levelInfoTextAuthor, mStates);
        renderWithView(overlayView, textUI->levelInfoTextBy, mStates);
        renderWithView(overlayView, textUI->levelInfoTextDM, mStates);
    }
}

void HexagonGame::drawParticles()
{
    const sf::View overlayView =
        Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    for (Particle& p : particles)
    {
        renderWithView(
            overlayView, p.sprite, sf::RenderStates{.texture = txStarParticle});
    }
}

void HexagonGame::drawTrailParticles()
{
    const sf::View backgroundView =
        Utils::computeCameraView(*backgroundCamera, backgroundCameraTransform);

    for (TrailParticle& p : trailParticles)
    {
        renderWithView(backgroundView, p.sprite,
            sf::RenderStates{.texture = txSmallCircle});
    }
}

void HexagonGame::drawSwapParticles()
{
    const sf::View backgroundView =
        Utils::computeCameraView(*backgroundCamera, backgroundCameraTransform);

    for (SwapParticle& p : swapParticles)
    {
        renderWithView(backgroundView, p.sprite,
            sf::RenderStates{.texture = txSmallCircle});
    }
}

void HexagonGame::updateText(float mFT)
{
    if (window == nullptr || !textUI.hasValue())
    {
        return;
    }

    // ------------------------------------------------------------------------
    // Update "personal best" text animation.
    pbTextGrowth += 0.08f * mFT;
    if (pbTextGrowth > Utils::pi * 2.f)
    {
        pbTextGrowth = 0;
    }

    // ------------------------------------------------------------------------
    os.str("");

    if (debugPause)
    {
        os << "(!) PAUSED (!)\n";
    }

    if (levelStatus.tutorialMode)
    {
        os << "TUTORIAL MODE\n";
    }
    else if (Config::getOfficial())
    {
        os << "OFFICIAL MODE\n";
    }

    if (Config::getDebug())
    {
        os << "DEBUG MODE\n";

        os << "CUSTOM WALLS: " << cwManager.count() << " / "
           << cwManager.maxHandles() << '\n';
    }

    if (status.started)
    {
        if (levelStatus.swapEnabled)
        {
            os << "SWAP ENABLED\n";
        }

        if (Config::getInvincible())
        {
            os << "INVINCIBILITY ON\n";
        }

        if (const float timescale = Config::getTimescale(); timescale != 1.f)
        {
            os << "TIMESCALE " << timescale << '\n';
        }

        if (status.scoreInvalid)
        {
            os << "SCORE INVALIDATED (" << status.invalidReason << ")\n";
        }

        if (status.hasDied)
        {
            os << status.restartInput;
            os << status.replayInput;
        }

        if (calledDeprecatedFunctions.size() > 1)
        {
            os << calledDeprecatedFunctions.size()
               << " WARNINGS RAISED (CHECK CONSOLE)\n";
        }
        else if (calledDeprecatedFunctions.size() > 0)
        {
            os << "1 WARNING RAISED (CHECK CONSOLE)\n";
        }

        const auto& trackedVariables(levelStatus.trackedVariables);
        if (Config::getShowTrackedVariables() && !trackedVariables.empty())
        {
            os << '\n';
            for (const auto& [variableName, display] : trackedVariables)
            {
                if (!lua.doesVariableExist(variableName))
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
    else if (Config::getRotateToStart())
    {
        os << "ROTATE TO START\n";
        textUI->messageText.setString("ROTATE TO START");
    }

    os.flush();

    // Set in game timer text
    if (!levelStatus.scoreOverridden)
    {
        // By default, use the timer for scoring
        if (status.started)
        {
            textUI->timeText.setString(formatTime(status.getTimeSeconds()));
        }
        else
        {
            textUI->timeText.setString("0");
        }
    }
    else
    {
        // Alternative scoring
        textUI->timeText.setString(
            lua.readVariable<std::string>(levelStatus.scoreOverride));
    }

    const auto getScaledCharacterSize = [&](const float size)
    { return size / Config::getZoomFactor() * Config::getTextScaling(); };

    setVisualCharacterSize(textUI->timeText, getScaledCharacterSize(70.f));

    // Set information text
    textUI->text.setString(os.str());
    setVisualCharacterSize(textUI->text, getScaledCharacterSize(20.f));
    textUI->text.origin = {0.f, 0.f};

    // Set FPS Text, if option is enabled.
    if (Config::getShowFPS())
    {
        textUI->fpsText.setString(ssvu::toStr(window->getFPS()));
        setVisualCharacterSize(textUI->fpsText, getScaledCharacterSize(20.f));
    }

    setVisualCharacterSize(textUI->messageText, getScaledCharacterSize(32.f));
    textUI->messageText.origin = {
        textUI->messageText.getGlobalWidth() / 2.f, 0.f};

    const float growth = std::sin(pbTextGrowth);
    setVisualCharacterSize(
        textUI->pbText, getScaledCharacterSize(64.f) + growth * 10.f);
    textUI->pbText.origin = textUI->pbText.getLocalCenter();

    // ------------------------------------------------------------------------

    if (mustShowReplayUI())
    {
        const replay_file& rf = activeReplay->replayFile;

        os.str("");

        if (!levelStatus.scoreOverridden)
        {
            os << formatTime(rf.played_seconds()) << "s";
        }
        else
        {
            os << formatTime(rf._played_score);
        }

        os << " BY " << rf._player_name;

        os.flush();

        setVisualCharacterSize(
            textUI->replayText, getScaledCharacterSize(16.f));
        textUI->replayText.setString(os.str());
    }
    else
    {
        textUI->replayText.setString("");
    }
}

void HexagonGame::drawText_TimeAndStatus(
    const sf::Color& offsetColor, const sf::RenderStates& mStates)
{
    if (!textUI.hasValue())
    {
        return;
    }

    const sf::View overlayView =
        Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    if (Config::getDrawTextOutlines())
    {
        textUI->timeText.setOutlineColor(offsetColor);
        textUI->text.setOutlineColor(offsetColor);
        textUI->fpsText.setOutlineColor(offsetColor);
        textUI->replayText.setOutlineColor(offsetColor);

        textUI->timeText.setOutlineThickness(2.f);
        textUI->text.setOutlineThickness(1.f);
        textUI->fpsText.setOutlineThickness(1.f);
        textUI->replayText.setOutlineThickness(1.f);
    }
    else
    {
        textUI->timeText.setOutlineThickness(0.f);
        textUI->text.setOutlineThickness(0.f);
        textUI->fpsText.setOutlineThickness(0.f);
        textUI->replayText.setOutlineThickness(0.f);
    }

    const float padding =
        (Config::getTextPadding() * Config::getTextScaling()) /
        Config::getZoomFactor();

    const sf::Color colorText = getColorText();

    if (Config::getShowTimer())
    {
        textUI->timeText.setFillColor(colorText);
        textUI->timeText.origin = textUI->timeText.getLocalTopLeft();
        textUI->timeText.position = {padding, padding};

        renderWithView(overlayView, textUI->timeText, mStates);
    }

    if (Config::getShowStatusText())
    {
        textUI->text.setFillColor(colorText);
        textUI->text.origin = textUI->text.getLocalTopLeft();
        textUI->text.position = {
            padding, textUI->timeText.getGlobalBottom() + padding};

        renderWithView(overlayView, textUI->text, mStates);
    }

    if (Config::getShowFPS())
    {
        textUI->fpsText.setFillColor(colorText);
        textUI->fpsText.origin = textUI->fpsText.getLocalBottomLeft();

        if (Config::getShowLevelInfo() || mustShowReplayUI())
        {
            textUI->fpsText.position = {
                padding, levelInfoRectangle.getGlobalTop() - padding};
        }
        else
        {
            textUI->fpsText.position = {padding, Config::getHeight() - padding};
        }

        renderWithView(overlayView, textUI->fpsText, mStates);
    }

    if (mustShowReplayUI())
    {
        const float scaling =
            Config::getKeyIconsScale() / Config::getZoomFactor();

        const float replayPadding = 8.f * scaling;

        textUI->replayText.setFillColor(colorText);
        textUI->replayText.origin = textUI->replayText.getLocalCenterRight();
        textUI->replayText.position =
            replayIcon.getGlobalCenterLeft() - sf::Vec2f{replayPadding, 0};
        renderWithView(overlayView, textUI->replayText, mStates);
    }
}

template <typename FRender>
static void drawTextMessagePBImpl(sf::Text& text, const sf::Color& offsetColor,
    const sf::Vec2f pos, const sf::Color& color, float outlineThickness,
    FRender&& fRender)
{
    if (text.getString().isEmpty())
    {
        return;
    }

    if (Config::getDrawTextOutlines())
    {
        text.setOutlineColor(offsetColor);
        text.setOutlineThickness(outlineThickness);
    }
    else
    {
        text.setOutlineThickness(0.f);
    }

    text.position = pos;
    text.setFillColor(color);

    fRender(text);
}

void HexagonGame::drawText_Message(
    const sf::Color& offsetColor, const sf::RenderStates& mStates)
{
    const sf::View overlayView =
        Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    drawTextMessagePBImpl(textUI->messageText, offsetColor,
        {Config::getWidth() / 2.f, Config::getHeight() / 5.5f}, getColorText(),
        1.f /* outlineThickness */, [this, &mStates, overlayView](sf::Text& t)
        { renderWithView(overlayView, t, mStates); });
}

void HexagonGame::drawText_PersonalBest(
    const sf::Color& offsetColor, const sf::RenderStates& mStates)
{
    const sf::View overlayView =
        Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    drawTextMessagePBImpl(textUI->pbText, offsetColor,
        {Config::getWidth() / 2.f,
            Config::getHeight() - Config::getHeight() / 4.f},
        getColorText(), 4.f /* outlineThickness */,
        [this, &mStates, overlayView](sf::Text& t)
        { renderWithView(overlayView, t, mStates); });
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
