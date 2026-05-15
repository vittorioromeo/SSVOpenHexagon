// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Imgui.hpp"
#include "SSVOpenHexagon/Utils/CameraView.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/Math.hpp"
#include "SSVOpenHexagon/Utils/Random.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"

namespace hg
{

[[nodiscard]] static sf::base::String formatTime(const double x)
{
    return Utils::toMinimalFloatString(SFML_BASE_MATH_FLOOR(x * 1000) / 1000.f);
}

static void setVisualCharacterSize(sf::Text& text, const float characterSize)
{
    const float baseSize = static_cast<float>(text.getCharacterSize());
    const float scale    = characterSize / baseSize;
    text.scale           = {scale, scale};
}

template <typename TDrawable>
void HexagonGame::renderWithView(const sf::View& view, TDrawable&& drawable)
{
    if (renderTarget == nullptr)
    {
        hg::lo("hg::HexagonGame::renderWithView") << "Attempted to render without a target\n";

        return;
    }

    sf::RenderStates states;
    states.view = view;
    renderTarget->draw(SFML_BASE_FORWARD(drawable), states);
}

template <typename TDrawable>
void HexagonGame::renderWithView(const sf::View& view, TDrawable&& drawable, sf::RenderStates states)
{
    if (renderTarget == nullptr)
    {
        hg::lo("hg::HexagonGame::renderWithView") << "Attempted to render without a target\n";

        return;
    }

    states.view = view;
    renderTarget->draw(SFML_BASE_FORWARD(drawable), states);
}

void HexagonGame::draw()
{
    // Window may be null when running headless. Render target may be null
    // when no draw target is wired up -- both forbid rendering.
    if (window == nullptr || renderTarget == nullptr || Config::getDisableGameRendering())
    {
        return;
    }

    const auto getRenderStates = [this](const RenderStage rs) -> sf::RenderStates
    {
        if (!Config::getShaders())
        {
            return sf::RenderStates{};
        }

        const sf::base::Optional<sf::base::SizeT> fragmentShaderId = status.fragmentShaderIds[static_cast<sf::base::SizeT>(rs)];

        if (!fragmentShaderId.hasValue())
        {
            return sf::RenderStates{};
        }

        (void)runLuaFunctionIfExists<int, float>("onRenderStage", static_cast<int>(rs), 60.f / window->getFPS());
        return sf::RenderStates{.shader = assets.getShaderByShaderId(*fragmentShaderId)};
    };

    SSVOH_ASSERT(backgroundCamera.hasValue());
    SSVOH_ASSERT(overlayCamera.hasValue());

    // The caller owns the clear when previewing into a render texture
    // (and the menu owns the clear when HG draws as a backdrop).
    if (!previewMode)
    {
        renderTarget->clear(sf::Color::Black);
    }

    if (!status.hasDied)
    {
        if (levelStatus.cameraShake > 0.f)
        {
            const sf::Vec2f shake(Utils::getRndR(-levelStatus.cameraShake, levelStatus.cameraShake),
                                  Utils::getRndR(-levelStatus.cameraShake, levelStatus.cameraShake));

            backgroundCamera->center = shake;
            overlayCamera->center    = shake + sf::Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 2.f};
        }
        else
        {
            backgroundCamera->center = sf::Vec2f{0.f, 0.f};
            overlayCamera->center    = sf::Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 2.f};
        }
    }

    backgroundCameraTransform.skew = sf::Vec2f{1.f, 1.f};

    if (Config::get3D())
    {
        const float pulse3D{Config::getNoPulse() ? 1.f : status.pulse3D};
        const float effect{styleData._3dSkew * Config::get3DMultiplier() * pulse3D};

        // Floor the multiplier slightly above zero. A custom level with a
        // large negative `_3dSkew` can drive `1 + effect` to 0 (or
        // negative), which `computeCameraView` propagates to `View::size.y`
        // and SFML asserts on at draw flush time. The visual difference
        // between skew.y == 0.001 and skew.y == 0 is imperceptible, so
        // clamping here is preferable to letting the level crash the
        // preview pipeline.
        constexpr float kMinSkew       = 0.001f;
        const float     skewY          = (1.f + effect < kMinSkew) ? kMinSkew : 1.f + effect;
        backgroundCameraTransform.skew = sf::Vec2f{1.f, skewY};
    }

    const sf::View backgroundView = Utils::computeCameraView(*backgroundCamera, backgroundCameraTransform);

    if (!Config::getNoBackground())
    {
        backgroundTris.clear();

        styleData.drawBackground(backgroundTris,
                                 sf::Vec2f{0.f, 0.f},
                                 levelStatus.sides,
                                 Config::getDarkenUnevenBackgroundChunk() && levelStatus.darkenUnevenBackgroundChunk,
                                 Config::getBlackAndWhite());

        renderWithView(backgroundView, backgroundTris, getRenderStates(RenderStage::BackgroundTris));
    }

    wallQuads3D.clear();
    pivotQuads3D.clear();
    playerTris3D.clear();
    wallQuads.clear();
    pivotQuads.clear();
    playerTris.clear();
    capTris.clear();

    // Reserve right amount of memory for all walls and custom walls
    wallQuads.reserveMoreQuad(walls.size() + cwManager.count());

    for (CWall& w : walls)
    {
        w.draw(getColorWall(), wallQuads);
    }

    cwManager.draw(wallQuads);

    if (status.started)
    {
        if (previewMode)
        {
            player.drawPivot(getSides(), getColorMain(), pivotQuads, capTris, getColorCap());
        }
        else
        {
            // Player isn't rendered in preview mode -- the menu doesn't show
            // a controllable player while a level animates as a backdrop.
            player.draw(getSides(),
                        getColorMain(),
                        getColorPlayer(),
                        pivotQuads,
                        capTris,
                        playerTris,
                        getColorCap(),
                        Config::getAngleTiltIntensity(),
                        Config::getShowSwapBlinkingEffect());
        }
    }

    if (Config::get3D())
    {
        // `_3dDepth` is exposed as a `float` (Lua reflection), but everything
        // downstream wants integral loop / slice counts. Resolve once here so
        // append, modify, and reserve all agree.
        const sf::base::SizeT depthInt(static_cast<sf::base::SizeT>(styleData._3dDepth));
        const sf::base::SizeT numWallQuads(wallQuads.size());
        const sf::base::SizeT numPivotQuads(pivotQuads.size());
        const sf::base::SizeT numPlayerTris(playerTris.size());

        wallQuads3D.reserve(numWallQuads * depthInt);
        pivotQuads3D.reserve(numPivotQuads * depthInt);
        playerTris3D.reserve(numPlayerTris * depthInt);

        const float effect{styleData._3dSkew * Config::get3DMultiplier() * (Config::getNoPulse() ? 1.f : status.pulse3D)};

        const float radRot(Utils::toRad(backgroundCamera->rotation.asDegrees()) + (Utils::pi / 2.f));
        const float sinRot(SFML_BASE_MATH_SINF(radRot));
        const float cosRot(SFML_BASE_MATH_COSF(radRot));

        const auto adjustAlpha = [&](sf::Color& c, const float i)
        {
            if (styleData._3dAlphaMult == 0.f)
            {
                return;
            }

            const float newAlpha = (static_cast<float>(c.a) / styleData._3dAlphaMult) - i * styleData._3dAlphaFalloff;

            c.a = Utils::componentClamp(newAlpha);
        };

        // Wall and player layers only diverge from the pivot layer's color
        // when the 3D override is the same as the main color (otherwise the
        // override applies uniformly).
        const bool overrideIsMain = styleData.get3DOverrideColor() == styleData.getMainColor();

        // Fused per-layer pass: append the layer's slice, then offset and
        // recolor the just-appended vertices in place. Layer ordering is
        // back-to-front so alpha blending stacks correctly.
        for (sf::base::SizeT j = 0; j < depthInt; ++j)
        {
            const float i = static_cast<float>(depthInt - j - 1);

            const float offset(styleData._3dSpacing * ((i + 1.f) * styleData._3dPerspectiveMult) * (effect * 3.6f) * 1.4f);

            const sf::Vec2f newPos(offset * cosRot, offset * sinRot);

            // Pivot color: 3D override (or white in B&W) darkened + alpha-adjusted.
            sf::Color pivotColor = Config::getBlackAndWhite()
                                       ? Utils::getColorDarkened(sf::Color(255, 255, 255, styleData.getMainColor().a),
                                                                 styleData._3dDarkenMult)
                                       : Utils::getColorDarkened(styleData.get3DOverrideColor(), styleData._3dDarkenMult);
            adjustAlpha(pivotColor, i);

            sf::Color wallColor   = pivotColor;
            sf::Color playerColor = pivotColor;

            if (overrideIsMain)
            {
                wallColor = Utils::getColorDarkened(getColorWall(), styleData._3dDarkenMult);
                adjustAlpha(wallColor, i);

                playerColor = Utils::getColorDarkened(getColorPlayer(), styleData._3dDarkenMult);
                adjustAlpha(playerColor, i);
            }

            const auto offsetAndColor =
                [&newPos](auto& buf, const sf::base::SizeT begin, const sf::base::SizeT end, const sf::Color color)
            {
                for (sf::base::SizeT k = begin; k < end; ++k)
                {
                    buf[k].position += newPos;
                    buf[k].color = color;
                }
            };

            const sf::base::SizeT pivotBegin  = pivotQuads3D.size();
            const sf::base::SizeT wallBegin   = wallQuads3D.size();
            const sf::base::SizeT playerBegin = playerTris3D.size();

            pivotQuads3D.unsafeAppend(pivotQuads);
            wallQuads3D.unsafeAppend(wallQuads);
            playerTris3D.unsafeAppend(playerTris);

            offsetAndColor(pivotQuads3D, pivotBegin, pivotBegin + numPivotQuads, pivotColor);
            offsetAndColor(wallQuads3D, wallBegin, wallBegin + numWallQuads, wallColor);
            offsetAndColor(playerTris3D, playerBegin, playerBegin + numPlayerTris, playerColor);
        }
    }

    renderWithView(backgroundView, wallQuads3D, getRenderStates(RenderStage::WallQuads3D));
    renderWithView(backgroundView, pivotQuads3D, getRenderStates(RenderStage::PivotQuads3D));
    renderWithView(backgroundView, playerTris3D, getRenderStates(RenderStage::PlayerTris3D));

    if (Config::getShowPlayerTrail() && status.showPlayerTrail)
    {
        drawTrailParticles();
    }

    if (Config::getShowSwapParticles())
    {
        drawSwapParticles();
    }

    renderWithView(backgroundView, wallQuads, getRenderStates(RenderStage::WallQuads));
    renderWithView(backgroundView, capTris, getRenderStates(RenderStage::CapTris));
    renderWithView(backgroundView, pivotQuads, getRenderStates(RenderStage::PivotQuads));
    renderWithView(backgroundView, playerTris, getRenderStates(RenderStage::PlayerTris));

    drawParticles();

    // Text overlays, key icons, level info, and flash effect belong to
    // gameplay UI -- not to the preview backdrop the menu wants.
    if (!previewMode)
    {
        drawText(getRenderStates(RenderStage::Text));

        if (Config::getShowKeyIcons() || mustShowReplayUI())
        {
            drawKeyIcons();
        }

        if (Config::getShowLevelInfo() || mustShowReplayUI())
        {
            drawLevelInfo(getRenderStates(RenderStage::Text));
        }

        if (Config::getFlash())
        {
            renderWithView(Utils::computeCameraView(*overlayCamera, overlayCameraTransform), flashPolygon);
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
    flashPolygon.reserveQuad(1);

    const sf::Color color{static_cast<sf::base::U8>(r), static_cast<sf::base::U8>(g), static_cast<sf::base::U8>(b), 0};

    const auto  width  = static_cast<float>(Config::getWidth());
    const auto  height = static_cast<float>(Config::getHeight());
    const float offset = 100.f;

    const sf::Vec2f nw{-offset, -offset};
    const sf::Vec2f sw{-offset, height + offset};
    const sf::Vec2f se{width + offset, height + offset};
    const sf::Vec2f ne{width + offset, -offset};

    flashPolygon.batchUnsafeEmplaceBackQuad(color, nw, sw, se, ne);
}

void HexagonGame::drawKeyIcons()
{
    constexpr sf::base::U8 offOpacity  = 90;
    constexpr sf::base::U8 onOpacity   = 255;
    const sf::View         overlayView = Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    const sf::Color colorText = getColorText();

    const sf::Color offColor{colorText.r, colorText.g, colorText.b, offOpacity};
    const sf::Color onColor{colorText.r, colorText.g, colorText.b, onOpacity};

    keyIconLeft.color  = (getInputMovement() == -1) ? onColor : offColor;
    keyIconRight.color = (getInputMovement() == 1) ? onColor : offColor;
    keyIconFocus.color = getInputFocused() ? onColor : offColor;
    keyIconSwap.color  = getInputSwap() ? onColor : offColor;

    renderWithView(overlayView, keyIconLeft, sf::RenderStates{.texture = txKeyIconLeft});
    renderWithView(overlayView, keyIconRight, sf::RenderStates{.texture = txKeyIconRight});
    renderWithView(overlayView, keyIconFocus, sf::RenderStates{.texture = txKeyIconFocus});
    renderWithView(overlayView, keyIconSwap, sf::RenderStates{.texture = txKeyIconSwap});

    // ------------------------------------------------------------------------

    if (mustShowReplayUI())
    {
        replayIcon.color = onColor;
        renderWithView(overlayView, replayIcon, sf::RenderStates{.texture = txReplayIcon});
    }
}

void HexagonGame::drawLevelInfo(const sf::RenderStates& mStates)
{
    const sf::View overlayView = Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

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
    const sf::View overlayView = Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    for (Particle& p : particles)
    {
        renderWithView(overlayView, p.sprite, sf::RenderStates{.texture = txStarParticle});
    }
}

void HexagonGame::drawTrailParticles()
{
    const sf::View backgroundView = Utils::computeCameraView(*backgroundCamera, backgroundCameraTransform);

    for (TrailParticle& p : trailParticles)
    {
        renderWithView(backgroundView, p.sprite, sf::RenderStates{.texture = txSmallCircle});
    }
}

void HexagonGame::drawSwapParticles()
{
    const sf::View backgroundView = Utils::computeCameraView(*backgroundCamera, backgroundCameraTransform);

    for (SwapParticle& p : swapParticles)
    {
        renderWithView(backgroundView, p.sprite, sf::RenderStates{.texture = txSmallCircle});
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
    os.setStr("");

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

        os << "CUSTOM WALLS: " << cwManager.count() << " / " << cwManager.maxHandles() << '\n';
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
            os << calledDeprecatedFunctions.size() << " WARNINGS RAISED (CHECK CONSOLE)\n";
        }
        else if (calledDeprecatedFunctions.size() > 0)
        {
            os << "1 WARNING RAISED (CHECK CONSOLE)\n";
        }

        const auto& trackedVariables(levelStatus.trackedVariables);
        if (Config::getShowTrackedVariables() && !trackedVariables.empty())
        {
            os << '\n';
            for (const auto& tv : trackedVariables)
            {
                if (!lua.doesVariableExist(tv.key.cStr()))
                {
                    continue;
                }

                const sf::base::String value{lua.readVariable<sf::base::String>(tv.key.cStr())};

                os << Utils::toUppercase(tv.value) << ": " << Utils::toUppercase(value) << '\n';
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
        textUI->timeText.setString(lua.readVariable<sf::base::String>(levelStatus.scoreOverride.cStr()));
    }

    const auto getScaledCharacterSize = [&](const float size)
    { return size / Config::getZoomFactor() * Config::getTextScaling(); };

    setVisualCharacterSize(textUI->timeText, getScaledCharacterSize(70.f));

    // Set information text
    textUI->text.setString(os.to<sf::base::String>());
    setVisualCharacterSize(textUI->text, getScaledCharacterSize(20.f));
    textUI->text.origin = {0.f, 0.f};

    // Set FPS Text, if option is enabled.
    if (Config::getShowFPS())
    {
        textUI->fpsText.setString(sf::base::toString(window->getFPS()));
        setVisualCharacterSize(textUI->fpsText, getScaledCharacterSize(20.f));
    }

    setVisualCharacterSize(textUI->messageText, getScaledCharacterSize(32.f));
    textUI->messageText.origin = {textUI->messageText.getGlobalWidth() / 2.f, 0.f};

    const float growth = SFML_BASE_MATH_SINF(pbTextGrowth);
    setVisualCharacterSize(textUI->pbText, getScaledCharacterSize(64.f) + growth * 10.f);
    textUI->pbText.origin = textUI->pbText.getLocalCenter();

    // ------------------------------------------------------------------------

    if (mustShowReplayUI())
    {
        const replay_file& rf = activeReplay->replayFile;

        os.setStr("");

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

        setVisualCharacterSize(textUI->replayText, getScaledCharacterSize(16.f));
        textUI->replayText.setString(os.to<sf::base::String>());
    }
    else
    {
        textUI->replayText.setString("");
    }
}

void HexagonGame::drawText_TimeAndStatus(const sf::Color offsetColor, const sf::RenderStates& mStates)
{
    if (!textUI.hasValue())
    {
        return;
    }

    const sf::View overlayView = Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

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

    const float padding = (Config::getTextPadding() * Config::getTextScaling()) / Config::getZoomFactor();

    const sf::Color colorText = getColorText();

    if (Config::getShowTimer())
    {
        textUI->timeText.setFillColor(colorText);
        textUI->timeText.origin   = textUI->timeText.getLocalTopLeft();
        textUI->timeText.position = {padding, padding};

        renderWithView(overlayView, textUI->timeText, mStates);
    }

    if (Config::getShowStatusText())
    {
        textUI->text.setFillColor(colorText);
        textUI->text.origin   = textUI->text.getLocalTopLeft();
        textUI->text.position = {padding, textUI->timeText.getGlobalBottom() + padding};

        renderWithView(overlayView, textUI->text, mStates);
    }

    if (Config::getShowFPS())
    {
        textUI->fpsText.setFillColor(colorText);
        textUI->fpsText.origin = textUI->fpsText.getLocalBottomLeft();

        if (Config::getShowLevelInfo() || mustShowReplayUI())
        {
            textUI->fpsText.position = {padding, levelInfoRectangle.getGlobalTop() - padding};
        }
        else
        {
            textUI->fpsText.position = {padding, Config::getHeight() - padding};
        }

        renderWithView(overlayView, textUI->fpsText, mStates);
    }

    if (mustShowReplayUI())
    {
        const float scaling = Config::getKeyIconsScale() / Config::getZoomFactor();

        const float replayPadding = 8.f * scaling;

        textUI->replayText.setFillColor(colorText);
        textUI->replayText.origin   = textUI->replayText.getLocalCenterRight();
        textUI->replayText.position = replayIcon.getGlobalCenterLeft() - sf::Vec2f{replayPadding, 0};
        renderWithView(overlayView, textUI->replayText, mStates);
    }
}

template <typename FRender>
static void drawTextMessagePBImpl(sf::Text&       text,
                                  const sf::Color offsetColor,
                                  const sf::Vec2f pos,
                                  const sf::Color color,
                                  float           outlineThickness,
                                  FRender&&       fRender)
{
    if (text.getString().empty())
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

void HexagonGame::drawText_Message(const sf::Color offsetColor, const sf::RenderStates& mStates)
{
    const sf::View overlayView = Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    drawTextMessagePBImpl(textUI->messageText,
                          offsetColor,
                          {Config::getWidth() / 2.f, Config::getHeight() / 5.5f},
                          getColorText(),
                          1.f /* outlineThickness */,
                          [this, &mStates, overlayView](sf::Text& t) { renderWithView(overlayView, t, mStates); });
}

void HexagonGame::drawText_PersonalBest(const sf::Color offsetColor, const sf::RenderStates& mStates)
{
    const sf::View overlayView = Utils::computeCameraView(*overlayCamera, overlayCameraTransform);

    drawTextMessagePBImpl(textUI->pbText,
                          offsetColor,
                          {Config::getWidth() / 2.f, Config::getHeight() - Config::getHeight() / 4.f},
                          getColorText(),
                          4.f /* outlineThickness */,
                          [this, &mStates, overlayView](sf::Text& t) { renderWithView(overlayView, t, mStates); });
}

void HexagonGame::drawText(const sf::RenderStates& mStates)
{
    const sf::Color offsetColor{
        Config::getBlackAndWhite() || styleData.getColors().empty() ? sf::Color::Black : getColor(0)};

    drawText_TimeAndStatus(offsetColor, mStates);
    drawText_Message(offsetColor, mStates);
    drawText_PersonalBest(offsetColor, mStates);
}

} // namespace hg
