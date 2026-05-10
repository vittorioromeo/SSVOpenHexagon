// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Core/Frametime.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/HexagonClient.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/LuaScripting.hpp"
#include "SSVOpenHexagon/Core/RandomNumberGenerator.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/Easing.hpp"
#include "SSVOpenHexagon/Utils/Math.hpp"
#include "SSVOpenHexagon/Utils/MoveTowards.hpp"
#include "SSVOpenHexagon/Utils/Random.hpp"
#include "SSVOpenHexagon/Utils/Split.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"
#include "SSVOpenHexagon/Utils/Timeline2.hpp"

#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Angle.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <random>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifndef SSVOH_ANDROID
    #include "SFML/ImGui/ImGuiContext.hpp"
    #include "SFML/ImGui/IncludeImGui.hpp"
#endif

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm/Remove.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"

#include <stdexcept>

#include <cstring>

namespace
{

template <typename TC, typename TP>
[[gnu::always_inline]] inline void eraseRemoveIf(TC& mContainer, TP mPredicate)
{
    mContainer.erase(sf::base::removeIf(mContainer.begin(), mContainer.end(), mPredicate), mContainer.end());
}

} // namespace

namespace hg
{

void HexagonGame::fastForwardTo(const double target)
{
    const HRTimePoint tpBegin = HRClock::now();

    const auto exceededProcessingTime = [&]
    {
        constexpr int maxProcessingSeconds = 3;
        return hrSecondsSince(tpBegin) > maxProcessingSeconds;
    };

    while (!status.hasDied && status.getTimeSeconds() < target && !exceededProcessingTime())
    {
        update(Config::TIME_STEP, 1.0f /* timescale */);
        postUpdate();
    }
}

void HexagonGame::advanceByTicks(const int nTicks)
{
    for (int i = 0; i < nTicks; ++i)
    {
        update(Config::TIME_STEP, 1.0f /* timescale */);
        postUpdate();
    }
}

void HexagonGame::update(float mFT, const float timescale)
{
    // ------------------------------------------------------------------------
    // Fast-forwarding for level testing
    if (fastForwardTarget.hasValue())
    {
        const double target = fastForwardTarget.value();
        fastForwardTarget.reset();

        fastForwardTo(target);

        if (audio != nullptr)
        {
            audio->setMusicPlayingOffsetSeconds(audio->getMusicPlayingOffsetSeconds() + status.getTimeSeconds());
        }

        return;
    }

    // ------------------------------------------------------------------------
    // Advance by ticks for level testing
    if (advanceTickCount.hasValue())
    {
        const int nTicks = advanceTickCount.value();
        advanceTickCount.reset();

        const bool wasPaused = debugPause;

        debugPause = false;
        advanceByTicks(nTicks);
        debugPause = wasPaused;

        return;
    }

    // ------------------------------------------------------------------------
    // Update client
    if (hexagonClient != nullptr)
    {
        hexagonClient->update();
    }

    // ------------------------------------------------------------------------
    // Scale simulation delta frame time
    mFT *= timescale;

    // ------------------------------------------------------------------------
    // Update Discord and Steam "rich presence".
    // Discord "rich presence" is also updated in `HexagonGame::start`.
    // Skipped in preview mode -- the user isn't actually playing this level.

    if (window != nullptr && !previewMode)
    {
        sf::base::String nameStr = levelData->name;
        nameFormat(nameStr);

        const sf::base::String diffStr = diffFormat(difficultyMult);
        const sf::base::String timeStr = timeFormat(status.getTimeSeconds());

        constexpr float DELAY_TO_UPDATE = 5.f; // X seconds
        timeUntilRichPresenceUpdate -= getFTToSeconds(mFT);

        if (timeUntilRichPresenceUpdate <= 0.f)
        {
            if (steamManager != nullptr)
            {
                steamManager->set_rich_presence_in_game(nameStr.cStr(), diffStr.cStr(), timeStr.cStr());
            }

            timeUntilRichPresenceUpdate = DELAY_TO_UPDATE;
        }

        updateRichPresenceCallbacks();
    }

    // ------------------------------------------------------------------------

    if (mustStart)
    {
        mustStart = false;
        start();
    }

    if (!debugPause)
    {
        updateFlash(mFT);

        if (previewMode)
        {
            // Preview / menu-backdrop: input is fully muted. Inputs from
            // a global keybind (e.g. user playing a different level above)
            // would otherwise leak through and steer the preview's player.
            inputMovement = 0;
            inputSwap     = false;
            inputFocused  = false;
        }
        else if (!mustReplayInput())
        {
            updateInput();
        }
        else
        {
            SSVOH_ASSERT(activeReplay.hasValue());

            if (!status.started)
            {
                if (window != nullptr)
                {
                    // This avoids initial speedup when viewing replays.
                    window->resetTimer();
                }

                mustStart = true;
            }
            else
            {
                const input_bitset ib = activeReplay->replayPlayer.get_current_and_move_forward();

                if (ib[static_cast<unsigned int>(input_bit::left)])
                {
                    inputMovement = -1;
                }
                else if (ib[static_cast<unsigned int>(input_bit::right)])
                {
                    inputMovement = 1;
                }
                else
                {
                    inputMovement = 0;
                }

                inputSwap    = ib[static_cast<unsigned int>(input_bit::swap)];
                inputFocused = ib[static_cast<unsigned int>(input_bit::focus)];
            }
        }

        // --------------------------------------------------------------------
        // Update key icons + level info -- both belong to gameplay HUD and
        // touch `textUI` state that the preview HG doesn't necessarily
        // populate. Skipped entirely when running as a menu backdrop.
        if (!previewMode)
        {
            if (Config::getShowKeyIcons() || mustShowReplayUI())
            {
                updateKeyIcons();
            }

            if (Config::getShowLevelInfo() || mustShowReplayUI())
            {
                updateLevelInfo();
            }
        }

        // --------------------------------------------------------------------
        // Update input leniency time after death to avoid accidental
        // restart.
        if (deathInputIgnore > 0.f)
        {
            deathInputIgnore -= mFT;
        }

        // --------------------------------------------------------------------
        if (status.started)
        {
            styleData.computeColors();

            player.update(getInputFocused(), getLevelStatus().swapEnabled, mFT);

            if (!status.hasDied)
            {
                const sf::base::Optional<bool>
                    preventPlayerInput = runLuaFunctionIfExists<bool, float, int, bool, bool>("onInput",
                                                                                              mFT,
                                                                                              getInputMovement(),
                                                                                              getInputFocused(),
                                                                                              getInputSwap());

                if (!preventPlayerInput.hasValue() || !(*preventPlayerInput))
                {
                    player.updateInputMovement(getInputMovement(), getPlayerSpeedMult(), getInputFocused(), mFT);

                    // Play "swap ready blip" sound and create particles
                    if (!playerNowReadyToSwap && player.isReadyToSwap())
                    {
                        playerNowReadyToSwap = true;

                        if (Config::getPlaySwapReadySound())
                        {
                            playSoundOverride("swapBlip.ogg");
                        }

                        swapParticlesSpawnInfo.emplace(SwapParticleSpawnInfo{.ready{true},
                                                                             .position{player.getPosition()},
                                                                             .angle{player.getPlayerAngle()}});
                    }

                    // Create particles after swap
                    if (getLevelStatus().swapEnabled && getInputSwap() && player.isReadyToSwap())
                    {
                        swapParticlesSpawnInfo.emplace(SwapParticleSpawnInfo{.ready{false},
                                                                             .position{player.getPosition()},
                                                                             .angle{player.getPlayerAngle()}});

                        performPlayerSwap(true /* mPlaySound */);
                        player.resetSwap(getSwapCooldown());
                        player.setJustSwapped(true);
                        playerNowReadyToSwap = false;
                    }
                    else
                    {
                        player.setJustSwapped(false);
                    }
                }

                status.accumulateFrametime(mFT);
                if (levelStatus.scoreOverridden)
                {
                    status.updateCustomScore(lua.readVariable<float>(levelStatus.scoreOverride.cStr()));
                }

                updateEvents(mFT);
                updateIncrement();

                if (mustChangeSides && walls.empty())
                {
                    sideChange(rng->get_int(levelStatus.sidesMin, levelStatus.sidesMax));
                }

                updateLevel(mFT);
                updateCustomTimelines();

                if (Config::getBeatPulse())
                {
                    updateBeatPulse(mFT);
                }

                updatePulse(mFT);

                if (!Config::getBlackAndWhite())
                {
                    styleData.update(mFT, std::pow(difficultyMult, 0.8f));
                }

                player.updatePosition(getRadius());

                updateWalls(mFT);
                eraseRemoveIf(walls, [](const CWall& w) { return w.isDead(); });

                updateCustomWalls(mFT);
            }
            else
            {
                levelStatus.rotationSpeed *= 0.99f;
            }

            // This is done even with 3D disabled as it's used to affect the RNG
            // state for replay validation.
            updatePulse3D(mFT);

            if (!Config::getNoRotation())
            {
                updateRotation(mFT);
            }

            updateCameraShake(mFT);

            if (!status.hasDied)
            {
                const auto fixup = [](const float x) -> random_number_generator::state_type
                {
                    // Avoid UB when converting to unsigned type:
                    return x < 0.f ? -x : x;
                };

                rng->advance(fixup(status.pulse));
                rng->advance(fixup(status.pulse3D));
                rng->advance(fixup(status.fastSpin));
                rng->advance(fixup(status.flashEffect));
                rng->advance(fixup(levelStatus.rotationSpeed));
                // TODO (P1): stuff from style?
            }
        }

        if (window != nullptr)
        {
            SSVOH_ASSERT(overlayCamera.hasValue());
            SSVOH_ASSERT(backgroundCamera.hasValue());

            updateParticles(mFT);

            if (Config::getShowPlayerTrail() && status.showPlayerTrail)
            {
                updateTrailParticles(mFT);
            }

            if (Config::getShowSwapParticles())
            {
                updateSwapParticles(mFT);
            }
        }
    }

    updateText(mFT);

    // Score / state-change machinery is meaningless in preview mode --
    // we never want a backdrop to push the user back to the menu, swap
    // levels via auto-restart, or invalidate a non-existent score.
    if (status.started && !previewMode)
    {
        if (status.mustStateChange != StateChange::None)
        {
            const bool executeLastReplay = status.mustStateChange == StateChange::MustReplay;

            if (!executeLastReplay && !assets.anyLocalProfileActive())
            {
                // If playing a replay from file, there is no local profile
                // active, so just go to the menu when attempting to restart
                // the level.

                goToMenu();
                return;
            }

            newGame(getPackId(), restartId, restartFirstTime, difficultyMult, executeLastReplay);
        }

// TODO (P2): score invalidation due to performance
#if 0
        if(!status.scoreInvalid && Config::getOfficial())
        {
            invalidateScore("PERFORMANCE ISSUES");
        }
#endif

        if (!Config::get3D() && levelStatus._3DRequired)
        {
            invalidateScore("3D REQUIRED");
        }

        if (!Config::getShaders() && levelStatus.shadersRequired)
        {
            invalidateScore("SHADERS REQUIRED");
        }
    }
}

void HexagonGame::updateWalls(float mFT)
{
    // Always advance wall motion -- that's the visual the menu backdrop
    // and previews need. Collision processing is gated on `!previewMode`
    // so the stand-in player can never die when previewing.
    if (previewMode)
    {
        for (CWall& w : walls)
        {
            w.update(levelStatus.wallSpawnDistance, getRadius(), centerPos, mFT);
        }
        return;
    }

    bool             collided{false};
    const float      radiusSquared{status.radius * status.radius + 8.f};
    const sf::Vec2f& pPos{player.getPosition()};

    for (CWall& w : walls)
    {
        w.update(levelStatus.wallSpawnDistance, getRadius(), centerPos, mFT);

        // If there is no collision skip to the next wall.
        if (!w.isOverlapping(pPos))
        {
            continue;
        }

        // Kill after a swap or if player could not be pushed out to safety.
        if (player.getJustSwapped())
        {
            performPlayerKill();

            if (steamManager != nullptr)
            {
                steamManager->unlock_achievement("a22_swapdeath");
            }
        }
        else if (player.push(getInputMovement(), getRadius(), w, centerPos, radiusSquared, mFT))
        {
            performPlayerKill();
        }

        collided = true;
    }

    // There was no collision, so we can stop here.
    if (!collided)
    {
        return;
    }

    // Second round, always deadly...
    for (CWall& w : walls)
    {
        if (!w.isOverlapping(pPos))
        {
            continue;
        }

        if (player.getJustSwapped())
        {
            if (steamManager != nullptr)
            {
                steamManager->unlock_achievement("a22_swapdeath");
            }
        }

        performPlayerKill();
    }
}

void HexagonGame::updateCustomWalls(float mFT)
{
    if (cwManager.handleCollision(getInputMovement(), getRadius(), player, mFT))
    {
        performPlayerKill();

        if (player.getJustSwapped())
        {
            if (steamManager != nullptr)
            {
                steamManager->unlock_achievement("a22_swapdeath");
            }
        }
    }
}

void HexagonGame::start()
{
    status.start();

    if (textUI.hasValue())
    {
        textUI->messageText.setString("");
    }

    playSoundOverride("go.ogg");

    if (!mustReplayInput())
    {
        sf::base::String nameStr = levelData->name;
        nameFormat(nameStr);

        sf::base::String packStr = getPackName();
        nameFormat(packStr);

        const sf::base::String diffStr = diffFormat(difficultyMult);

        if (discordManager != nullptr)
        {
            discordManager->set_rich_presence_in_game(nameStr + " [x" + diffStr + "]", packStr);
        }

        const sf::base::String& validator = levelData->getValidator(difficultyMult);

        if (hexagonClient != nullptr && hexagonClient->getState() == HexagonClient::State::LoggedIn_Ready &&
            Config::getOfficial() && !levelData->unscored && hexagonClient->isLevelSupportedByServer(validator))
        {
            hexagonClient->trySendStartedGame(validator);
        }
    }
    else
    {
        if (discordManager != nullptr)
        {
            discordManager->set_rich_presence_on_replay();
        }
    }

    // Don't resume music for preview HG instances -- they share the
    // single audio module with the gameplay HG, so resuming here would
    // unpause whatever music the previous gameplay session had loaded
    // (most visibly: scrolling LevelSelect after exiting a level would
    // restart that level's music in the menu).
    if (audio != nullptr && !Config::getNoMusic() && !previewMode)
    {
        audio->resumeMusic();
    }

    runVoidLuaFunctionIfExists("onLoad");
}

static void setInputImplIfFalse(bool& var, const bool x)
{
    if (!var)
    {
        var = x;
    }
}

void HexagonGame::updateInput_UpdateJoystickControls()
{
    if (window == nullptr)
    {
        return;
    }

    Joystick::update(Config::getJoystickDeadzone());

    setInputImplIfFalse(inputImplCCW, Joystick::pressed(Joystick::Jdir::Left));
    setInputImplIfFalse(inputImplCW, Joystick::pressed(Joystick::Jdir::Right));
    setInputImplIfFalse(inputSwap, Joystick::pressed(Joystick::Jid::Swap));
    setInputImplIfFalse(inputFocused, Joystick::pressed(Joystick::Jid::Focus));

    if (Joystick::risingEdge(Joystick::Jid::Exit))
    {
        goToMenu();
    }
    else if (Joystick::risingEdge(Joystick::Jid::ForceRestart) ||
             (status.hasDied && Joystick::risingEdge(Joystick::Jid::Restart)))
    {
        status.mustStateChange = StateChange::MustRestart;
    }
    else if (status.hasDied && Joystick::risingEdge(Joystick::Jid::Replay))
    {
        status.mustStateChange = StateChange::MustReplay;
    }
}

void HexagonGame::updateInput_UpdateTouchControls()
{
    if (window == nullptr)
    {
        return;
    }

    for (const auto& p : window->getFingerDownPositions())
    {
        if (p.x < window->getRenderWindow().getSize().x / 2.f)
        {
            setInputImplIfFalse(inputImplCCW, true);
        }
        else
        {
            setInputImplIfFalse(inputImplCW, true);
        }
    }
}

void HexagonGame::updateInput_ResolveInputImplToInputMovement()
{
    if (inputImplCW && !inputImplCCW)
    {
        inputMovement = inputImplLastMovement = 1;
        return;
    }

    if (!inputImplCW && inputImplCCW)
    {
        inputMovement = inputImplLastMovement = -1;
        return;
    }

    if (inputImplCW && inputImplCCW)
    {
        inputMovement = -inputImplLastMovement;
        return;
    }

    inputMovement = inputImplLastMovement = 0;
}

void HexagonGame::updateInput_RecordCurrentInputToLastReplayData()
{
    if (!status.started || status.hasDied)
    {
        return;
    }

    const bool left  = getInputMovement() == -1;
    const bool right = getInputMovement() == 1;
    const bool swap  = getInputSwap();
    const bool focus = getInputFocused();

    lastReplayData.record_input(left, right, swap, focus);
}

void HexagonGame::updateInput()
{
    if (imguiLuaConsoleHasInput())
    {
        return;
    }

    if (!status.started && (!Config::getRotateToStart() || inputImplCCW || inputImplCW))
    {
        mustStart = true;
    }

    if (alwaysSpinRight)
    {
        inputImplCCW = false;
        inputImplCW  = true;
        inputSwap    = false;
        inputFocused = false;
    }
    else if (executeRandomInputs) // TODO (P2): For testing
    {
        static std::random_device rd;
        static std::mt19937       en(rd());

        inputImplCCW = std::uniform_int_distribution<int>{0, 1}(en);
        inputImplCW  = std::uniform_int_distribution<int>{0, 1}(en);
        inputSwap    = std::uniform_int_distribution<int>{0, 1}(en);
        inputFocused = std::uniform_int_distribution<int>{0, 1}(en);
    }
    else
    {
        // Keyboard and mouse state is handled by callbacks set in the
        // constructor.
        updateInput_UpdateJoystickControls(); // Joystick state.
        updateInput_UpdateTouchControls();    // Touchscreen state.
    }

    updateInput_ResolveInputImplToInputMovement();
    updateInput_RecordCurrentInputToLastReplayData();
}

void HexagonGame::updateEvents(float)
{
    if (const auto o = eventTimelineRunner.update(eventTimeline, status.getTimeTP());
        o == Utils::timeline2_runner::outcome::finished)
    {
        eventTimeline.clear();
        eventTimelineRunner = {};
    }

    if (const auto o = messageTimelineRunner.update(messageTimeline, status.getCurrentTP());
        o == Utils::timeline2_runner::outcome::finished)
    {
        messageTimeline.clear();
        messageTimelineRunner = {};
    }
}

void HexagonGame::updateCustomTimelines()
{
    _customTimelineManager.updateAllTimelines(status.getCurrentTP());
}

void HexagonGame::updateIncrement()
{
    if (!levelStatus.incEnabled)
    {
        return;
    }

    if (status.getIncrementTimeSeconds() < levelStatus.incTime)
    {
        return;
    }

    ++levelStatus.currentIncrements;
    incrementDifficulty();
    status.resetIncrementTime();
    mustChangeSides = true;
}

void HexagonGame::updateLevel(float mFT)
{
    if (status.isTimePaused())
    {
        return;
    }

    runLuaFunctionIfExists<float>("onUpdate", mFT);

    const auto o = timelineRunner.update(timeline, status.getTimeTP());

    if (o == Utils::timeline2_runner::outcome::finished && !mustChangeSides)
    {
        timeline.clear();
        runVoidLuaFunctionIfExists("onStep");
        timelineRunner = {};
    }
}

void HexagonGame::updatePulse(float mFT)
{
    if (!levelStatus.manualPulseControl)
    {
        if (status.pulseDelay <= 0)
        {
            const float pulseAdd{status.pulseDirection > 0 ? levelStatus.pulseSpeed : -levelStatus.pulseSpeedR};

            const float pulseLimit{status.pulseDirection > 0 ? levelStatus.pulseMax : levelStatus.pulseMin};

            status.pulse += pulseAdd * mFT * getMusicDMSyncFactor();

            if ((status.pulseDirection > 0 && status.pulse >= pulseLimit) ||
                (status.pulseDirection < 0 && status.pulse <= pulseLimit))
            {
                status.pulse = pulseLimit;
                status.pulseDirection *= -1;

                if (status.pulseDirection < 0)
                {
                    status.pulseDelay = levelStatus.pulseDelayMax;
                }
            }
        }

        status.pulseDelay -= mFT * getMusicDMSyncFactor();
    }
    refreshPulse();
}

void HexagonGame::refreshPulse()
{
    if (window != nullptr)
    {
        SSVOH_ASSERT(backgroundCamera.hasValue());

        const float p{Config::getNoPulse() ? 1.f : (status.pulse / levelStatus.pulseMin)};
        const float rotation{backgroundCamera->rotation.asDegrees()};

        // Match the target's aspect ratio. For preview-mode HG instances
        // `renderTarget` is a fixed-size off-screen texture, so we must
        // not bake the live window dimensions into the view -- otherwise
        // resizing the window would stretch the preview every frame as
        // soon as the next pulse tick fires.
        const sf::Vec2f targetSize = (previewMode && renderTarget != nullptr)
                                         ? renderTarget->getSize().to<sf::Vec2f>()
                                         : sf::Vec2f{static_cast<float>(Config::getWidth()),
                                                     static_cast<float>(Config::getHeight())};

        *backgroundCamera = sf::View{.center = sf::Vec2f{0.f, 0.f}, .size = targetSize * Config::getZoomFactor() * p};

        backgroundCamera->rotation = sf::degrees(rotation);
    }
}

void HexagonGame::updateBeatPulse(float mFT)
{
    if (!levelStatus.manualBeatPulseControl)
    {
        if (status.beatPulseDelay <= 0)
        {
            status.beatPulse      = levelStatus.beatPulseMax;
            status.beatPulseDelay = levelStatus.beatPulseDelayMax;
        }
        else
        {
            status.beatPulseDelay -= mFT * getMusicDMSyncFactor();
        }

        if (status.beatPulse > 0)
        {
            status.beatPulse -= (2.f * mFT * getMusicDMSyncFactor()) * levelStatus.beatPulseSpeedMult;
        }
    }
    refreshBeatPulse();
}

void HexagonGame::refreshBeatPulse()
{
    const float radiusMin{Config::getBeatPulse() ? levelStatus.radiusMin : 75};
    status.radius = radiusMin * (status.pulse / levelStatus.pulseMin) + status.beatPulse;
}

void HexagonGame::updateRotation(float mFT)
{
    auto nextRotation(getRotationSpeed() * 10.f);
    if (status.fastSpin > 0)
    {
        nextRotation += std::abs((Utils::getSmootherStep(0, levelStatus.fastSpin, status.fastSpin) / 3.5f) * 17.f) *
                        Utils::getSign(nextRotation);

        status.fastSpin -= mFT;
    }

    if (window != nullptr)
    {
        SSVOH_ASSERT(backgroundCamera.hasValue());
        backgroundCamera->rotation += sf::degrees(nextRotation * mFT);
    }
}

void HexagonGame::updateCameraShake(float mFT)
{
    if (!backgroundCamera.hasValue() || !overlayCamera.hasValue())
    {
        return;
    }

    if (status.cameraShake <= 0.f)
    {
        if (preShakeCenters.hasValue())
        {
            backgroundCamera->center = preShakeCenters->background;
            overlayCamera->center    = preShakeCenters->overlay;

            preShakeCenters.reset();
        }

        return;
    }

    status.cameraShake -= mFT;

    if (!preShakeCenters.hasValue())
    {
        preShakeCenters.emplace(backgroundCamera->center, overlayCamera->center);
    }

    SSVOH_ASSERT(backgroundCamera.hasValue());
    SSVOH_ASSERT(overlayCamera.hasValue());
    SSVOH_ASSERT(preShakeCenters.hasValue());

    const auto makeShakeVec = [this]
    {
        // Clamp to non-negative: `cameraShake -= mFT` above can dip below
        // zero on a frame where `mFT` exceeds the remaining shake, which
        // would make `rng->get_real(-i, i)` see `min > max` and assert.
        // The next frame's early-out resets `preShakeCenters`.
        const float i = sf::base::max(0.f, status.cameraShake);
        return sf::Vec2f(rng->get_real(-i, i), rng->get_real(-i, i));
    };

    backgroundCamera->center = preShakeCenters->background + makeShakeVec();
    overlayCamera->center    = preShakeCenters->overlay + makeShakeVec();
}

void HexagonGame::updateFlash(float mFT)
{
    if (status.flashEffect > 0)
    {
        status.flashEffect -= 3 * mFT;
    }

    status.flashEffect = sf::base::clamp(status.flashEffect, 0.f, 255.f);

    // `flashPolygon` is allocated lazily by `initFlashEffect` (typically
    // called from Lua). Iterating with `begin()` on a never-reserved
    // vector trips an assert -- skip the alpha-update entirely until the
    // polygon has been initialised.
    if (flashPolygon.size() == 0u)
        return;

    for (sf::Vertex& vertex : flashPolygon)
    {
        vertex.color.a = status.flashEffect;
    }
}

void HexagonGame::updatePulse3D(float mFT)
{
    status.pulse3D += styleData._3dPulseSpeed * status.pulse3DDirection * mFT;
    if (status.pulse3D > styleData._3dPulseMax)
    {
        status.pulse3DDirection = -1.f;
    }
    else if (status.pulse3D < styleData._3dPulseMin)
    {
        status.pulse3DDirection = 1.f;
    }
}

void HexagonGame::updateParticles(float mFT)
{
    SSVOH_ASSERT(window != nullptr);

    const auto isOutOfBounds = [](const Particle& p)
    {
        const sf::Sprite& sp      = p.sprite;
        const sf::Vec2f   pos     = sp.position;
        constexpr float   padding = 256.f;

        return (pos.x < 0 - padding || pos.x > Config::getWidth() + padding || pos.y < 0 - padding ||
                pos.y > Config::getHeight() + padding);
    };

    const auto makePBParticle = [this]
    {
        SSVOH_ASSERT(txStarParticle != nullptr);
        Particle p{sf::Sprite{.textureRect = txStarParticle->getRect()}};

        p.sprite.position = {hg::Utils::getRndR(-64.f, Config::getWidth() + 64.f), -64.f};
        p.sprite.rotation = sf::degrees(hg::Utils::getRndR(0.f, 360.f));

        const float scale = hg::Utils::getRndR(0.75f, 1.35f);
        p.sprite.scale    = {scale, scale};

        sf::Color c    = getColorMain();
        c.a            = hg::Utils::getRndI(90, 145);
        p.sprite.color = c;

        p.velocity        = {hg::Utils::getRndR(-12.f, 12.f), hg::Utils::getRndR(4.f, 18.f)};
        p.angularVelocity = hg::Utils::getRndR(-6.f, 6.f);

        return p;
    };

    eraseRemoveIf(particles, isOutOfBounds);

    for (Particle& p : particles)
    {
        sf::Sprite& sp = p.sprite;
        sp.position += p.velocity * mFT;
        sp.rotation += sf::degrees(p.angularVelocity * mFT);
    }

    if (mustSpawnPBParticles)
    {
        nextPBParticleSpawn -= mFT;
        if (nextPBParticleSpawn <= 0.f)
        {
            particles.emplaceBack(makePBParticle());
            nextPBParticleSpawn = 2.75f;
        }
    }
}

void HexagonGame::updateTrailParticles(float mFT)
{
    SSVOH_ASSERT(window != nullptr);

    const auto isDead = [&](const TrailParticle& p) { return p.sprite.color.a <= 3; };

    const auto makeTrailParticle = [this]
    {
        SSVOH_ASSERT(txSmallCircle != nullptr);
        TrailParticle p{sf::Sprite{.textureRect = txSmallCircle->getRect()}};

        p.sprite.position = player.getPosition();
        p.sprite.origin   = txSmallCircle->getSize().to<sf::Vec2f>() / 2.f;

        const float scale = Config::getPlayerTrailScale();
        p.sprite.scale    = {scale, scale};

        sf::Color c = getColorPlayerTrail();

        c.a            = Config::getPlayerTrailAlpha();
        p.sprite.color = c;

        p.angle = player.getPlayerAngle();

        return p;
    };

    eraseRemoveIf(trailParticles, isDead);

    for (TrailParticle& p : trailParticles)
    {
        sf::Color color = p.sprite.color;

        const float newAlpha = Utils::getMoveTowardsZero(static_cast<float>(color.a), Config::getPlayerTrailDecay() * mFT);

        color.a        = static_cast<sf::base::U8>(newAlpha);
        p.sprite.color = color;

        p.sprite.scale *= 0.98f;

        p.sprite.position = sf::Vec2f::fromAngle(status.radius + 2.4f, sf::radians(p.angle));
    }

    if (player.hasChangedAngle())
    {
        trailParticles.emplaceBack(makeTrailParticle());
    }
}

void HexagonGame::updateSwapParticles(float mFT)
{
    SSVOH_ASSERT(window != nullptr);

    const auto isDead = [&](const SwapParticle& p) { return p.sprite.color.a <= 3; };

    const auto makeSwapParticle =
        [this](const SwapParticleSpawnInfo& si, const float expand, const float speedMult, const float scaleMult, const float alpha)
    {
        SSVOH_ASSERT(txSmallCircle != nullptr);
        SwapParticle p{sf::Sprite{.textureRect = txSmallCircle->getRect()}};

        p.sprite.position = si.position;
        p.sprite.origin   = txSmallCircle->getSize().to<sf::Vec2f>() / 2.f;

        const float scale = hg::Utils::getRndR(0.65f, 1.35f) * scaleMult;
        p.sprite.scale    = {scale, scale};

        sf::Color c = getColorPlayerTrail();

        c.a            = alpha;
        p.sprite.color = c;

        p.velocity = sf::Vec2f::fromAngle(hg::Utils::getRndR(0.1f, 10.f) * speedMult,
                                          sf::radians(si.angle + hg::Utils::getRndR(-expand, expand)));

        return p;
    };

    eraseRemoveIf(swapParticles, isDead);

    for (SwapParticle& p : swapParticles)
    {
        sf::Color color = p.sprite.color;

        const float newAlpha = Utils::getMoveTowardsZero(static_cast<float>(color.a), 3.5f * mFT);

        color.a        = static_cast<sf::base::U8>(newAlpha);
        p.sprite.color = color;

        p.sprite.scale *= 0.98f;
        p.sprite.position += p.velocity * mFT;
    }

    if (swapParticlesSpawnInfo.hasValue())
    {
        if (swapParticlesSpawnInfo->ready == false)
        {
            for (int i = 0; i < 20; ++i)
            {
                swapParticles.emplaceBack(
                    makeSwapParticle(*swapParticlesSpawnInfo,
                                     0.45f /* expand */,
                                     1.f /* speedMult */,
                                     1.f /* scaleMult */,
                                     45.f /* alpha */));
            }

            for (int i = 0; i < 10; ++i)
            {
                swapParticles.emplaceBack(
                    makeSwapParticle(*swapParticlesSpawnInfo,
                                     3.14f /* expand */,
                                     0.45f /* speedMult */,
                                     0.75f /* scaleMult */,
                                     35.f /* alpha */));
            }
        }
        else
        {
            for (int i = 0; i < 14; ++i)
            {
                swapParticles.emplaceBack(
                    makeSwapParticle(*swapParticlesSpawnInfo,
                                     3.14f /* expand */,
                                     1.3f /* speedMult */,
                                     0.4f /* scaleMult */,
                                     140.f /* alpha */));
            }
        }

        swapParticlesSpawnInfo.reset();
    }
}

#ifndef SSVOH_ANDROID
static int ilcTextEditCallbackStub(ImGuiInputTextCallbackData* data)
{
    auto hg = (HexagonGame*)data->UserData;
    return hg->ilcTextEditCallback(data);
}

static int Stricmp(const char* s1, const char* s2)
{
    int d;
    while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1)
    {
        s1++;
        s2++;
    }
    return d;
}

static int Strnicmp(const char* s1, const char* s2, int n)
{
    int d = 0;
    while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1)
    {
        s1++;
        s2++;
        n--;
    }
    return d;
}
#endif

int HexagonGame::ilcTextEditCallback([[maybe_unused]] ImGuiInputTextCallbackData* data)
{
#ifndef SSVOH_ANDROID
    switch (data->EventFlag)
    {
        case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Locate beginning of current word
            const char* word_end   = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                {
                    break;
                }

                word_start--;
            }

            // Skip starting `?` for Lua docs
            if (*word_start == '?')
            {
                ++word_start;
            }

            // Build a list of candidates
            ImVector<const char*> candidates;
            for (const sf::base::String& fnName : LuaScripting::getAllFunctionNames())
            {
                if (Strnicmp(fnName.cStr(), word_start, (int)(word_end - word_start)) == 0)
                {
                    candidates.push_back(fnName.cStr());
                }
            }

            if (candidates.Size == 0)
            {
                char buf[255];
                std::snprintf(buf, sizeof(buf), "No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);

                // No match
                ilcCmdLog.emplaceBack(buf);
            }
            else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and
                // replace it entirely so we've got nice casing.
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display
                // "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int  c                      = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                        if (i == 0)
                            c = std::toupper(candidates[i][match_len]);
                        else if (c == 0 || c != std::toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));

                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                ilcCmdLog.emplaceBack("Possible matches:\n");
                for (int i = 0; i < candidates.Size; i++)
                {
                    ilcCmdLog.emplaceBack(Utils::concat("- ", candidates[i], '\n'));
                }
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory:
        {
            const int prev_history_pos = ilcHistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (ilcHistoryPos == -1)
                {
                    ilcHistoryPos = ilcHistory.size() - 1;
                }
                else if (ilcHistoryPos > 0)
                {
                    ilcHistoryPos--;
                }
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (ilcHistoryPos != -1)
                {
                    if (++ilcHistoryPos >= static_cast<int>(ilcHistory.size()))
                    {
                        ilcHistoryPos = -1;
                    }
                }
            }

            if (prev_history_pos != ilcHistoryPos)
            {
                const char* history_str = (ilcHistoryPos >= 0) ? ilcHistory[ilcHistoryPos].cStr() : "";

                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
    }
#endif

    return 0;
}

void HexagonGame::postUpdate_ImguiLuaConsole()
{
#ifndef SSVOH_ANDROID
    if (window == nullptr)
    {
        return;
    }

    if (!Config::getDebug() || Config::getOfficial())
    {
        ilcShowConsole = ilcShowConsoleNext = false;
        return;
    }

    SSVOH_ASSERT(imguiCtx.hasValue());

    if (ilcShowConsoleNext)
    {
        ilcShowConsole     = !ilcShowConsole;
        ilcShowConsoleNext = false;

        imguiCtx->processEvent(window->getRenderWindow(), sf::Event::FocusGained{});
    }

    if (!ilcShowConsole)
    {
        return;
    }

    imguiCtx->update(*window, ilcDeltaClock.restart());

    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Lua Console");

    ImGui::Text("Enter `!help` to show help.");
    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing() + 150;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const sf::base::String& sItem : ilcCmdLog)
    {
        const char* item = sItem.cStr();

        const auto color = [&]() -> sf::base::Optional<ImVec4>
        {
            if (std::strstr(item, "[error]"))
            {
                return sf::base::makeOptional<ImVec4>(1.0f, 0.4f, 0.4f, 1.0f);
            }

            if (std::strstr(item, "[warning]"))
            {
                return sf::base::makeOptional<ImVec4>(1.0f, 0.4f, 1.0f, 1.0f);
            }

            if (std::strstr(item, "[lua]"))
            {
                return sf::base::makeOptional<ImVec4>(0.4f, 1.0f, 0.4f, 1.0f);
            }

            if (std::strncmp(item, "# ", 2) == 0)
            {
                return sf::base::makeOptional<ImVec4>(1.0f, 0.8f, 0.6f, 1.0f);
            }

            if (std::strstr(item, "[?]"))
            {
                return sf::base::makeOptional<ImVec4>(0.4f, 0.4f, 1.0f, 1.0f);
            }

            return sf::base::nullOpt;
        }();

        if (color.hasValue())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, *color);
        }

        sf::base::Vector<sf::base::String> split;
        sf::base::SizeT                    last = 0;

        for (sf::base::SizeT j = 0; j < sItem.size(); ++j)
        {
            if (sItem[j] == '\n')
            {
                split.emplaceBack(sf::base::String(sItem.toStringView().substrByPosLen(last, j - last)));
                last = j + 1;
            }
        }

        const sf::base::String lastPiece = sf::base::String(sItem.toStringView().substrByPosLen(last));
        if (!lastPiece.empty())
        {
            split.emplaceBack(lastPiece);
        }

        for (const sf::base::String& s : split)
        {
            constexpr sf::base::SizeT lineLimit = 80;
            if (s.size() <= lineLimit)
            {
                ImGui::TextUnformatted(s.cStr());
            }
            else
            {
                constexpr sf::base::SizeT charsPerSubstr = lineLimit;
                const sf::base::SizeT     nSubstrs       = s.size() / charsPerSubstr;

                for (sf::base::SizeT j = 0; j < nSubstrs + 1; ++j)
                {
                    const auto sv = s.toStringView().substrByPosLen(j * charsPerSubstr, charsPerSubstr);
                    ImGui::TextUnformatted(sv.data(), sv.data() + sv.size());
                }
            }
        }

        if (color.hasValue())
        {
            ImGui::PopStyleColor();
        }
    }

    ImGui::SetScrollHereY(1.0f);
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                           ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

    if (ImGui::InputText("Command", ilcCmdBuffer, sizeof(ilcCmdBuffer), input_text_flags, &ilcTextEditCallbackStub, (void*)this))
    {
        const sf::base::String cmdString = sf::base::String(ilcCmdBuffer);
        ilcCmdBuffer[0]                  = '\0';

        ilcCmdLog.emplaceBack(Utils::concat("# ", cmdString, '\n'));

        ilcHistoryPos = -1;
        for (int i = ilcHistory.size() - 1; i >= 0; --i)
        {
            if (ilcHistory[i] == cmdString)
            {
                ilcHistory.erase(ilcHistory.begin() + i);
                break;
            }
        }

        ilcHistory.emplaceBack(cmdString);

        const sf::base::Vector<sf::base::String> cmdSplit = Utils::split<sf::base::String>(cmdString);

        if (Stricmp(cmdString.cStr(), "!CLEAR") == 0)
        {
            ilcCmdLog.clear();
        }
        else if (Stricmp(cmdString.cStr(), "!HELP") == 0)
        {
            ilcCmdLog.emplaceBack(R"(Built-in commands:
!clear          Clears the console
!help           Display this help
!ff <seconds>   Fast-forward simulation to specified time
!advt <ticks>   Advance simulation by specified number of ticks
?fn             Display Lua docs for function `fn`
)");
        }
        else if (cmdSplit.size() > 1 && cmdSplit[0] == "!ff")
        {
            // `std::strtod` lives in `<cstdlib>` and operates on `const char*`, so it doesn't drag
            // `<string>` in the way `std::stod` would.
            const sf::base::String& secondsStr = cmdSplit[1];
            char*                   end        = nullptr;
            errno                              = 0;
            const double seconds               = std::strtod(secondsStr.cStr(), &end);

            if (end == secondsStr.cStr())
            {
                ilcCmdLog.emplaceBack("[error]: invalid argument for <seconds>\n");
            }
            else if (errno == ERANGE)
            {
                ilcCmdLog.emplaceBack("[error]: out of range for <seconds>\n");
            }
            else
            {
                ilcCmdLog.emplaceBack(Utils::concat("[ff]: fast forwarding to ", seconds, '\n'));
                fastForwardTarget.emplace(seconds);
            }
        }
        else if (cmdSplit.size() > 1 && cmdSplit[0] == "!advt")
        {
            const sf::base::String& ticksStr = cmdSplit[1];
            char*                   end      = nullptr;
            errno                            = 0;
            const long parsed                = std::strtol(ticksStr.cStr(), &end, 10);

            if (end == ticksStr.cStr())
            {
                ilcCmdLog.emplaceBack("[error]: invalid argument for <seconds>\n");
            }
            else if (errno == ERANGE || parsed > INT_MAX || parsed < INT_MIN)
            {
                ilcCmdLog.emplaceBack("[error]: out of range for <seconds>\n");
            }
            else
            {
                const int ticks = static_cast<int>(parsed);
                ilcCmdLog.emplaceBack(Utils::concat("[advt]: advancing simulation by ", ticks, " ticks\n"));
                advanceTickCount.emplace(ticks >= 0 ? ticks : 0);
            }
        }
        else if (cmdString[0] == '?')
        {
            const sf::base::String rest = Utils::getRTrim(sf::base::String(cmdString.toStringView().substrByPosLen(1)));
            const sf::base::String docs = LuaScripting::getDocsForFunction(rest);
            ilcCmdLog.emplaceBack(Utils::concat("[?]: ", docs));
        }
        else
        {
            try
            {
                try
                {
                    lua.executeCode(Utils::concat("u_log(", cmdString, ")\n").cStr());
                } catch (std::runtime_error& mError)
                {
                    lua.executeCode((cmdString + "\n").cStr());
                }
            } catch (std::runtime_error& mError)
            {
                sf::base::String temp = "[error]: ";
                temp += mError.what();
                temp += '\n';

                ilcCmdLog.emplaceBack(temp);
            } catch (...)
            {
                ilcCmdLog.emplaceBack("[error]: unknown\n");
            }
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::Separator();

    ImGui::Text("update ms: %.2f", window->getMsUpdate());
    ImGui::SameLine();
    ImGui::Text("draw ms: %.2f", window->getMsDraw());

    static float simSpeed = Config::getTimescale();
    ImGui::DragFloat("Timescale", &simSpeed, 0.005f);
    Config::setTimescale(simSpeed);

    ImGui::SameLine();

    static bool invincible = Config::getInvincible();
    ImGui::Checkbox("Invincible", &invincible);
    Config::setInvincible(invincible);

    ImGui::Separator();

    {
        if (ImGui::InputText("Track", ilcTrackBuffer, sizeof(ilcTrackBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            const sf::base::String codeToTrack = Utils::getLRTrim(sf::base::String(ilcTrackBuffer));
            ilcLuaTracked.emplaceBack(Utils::concat("u_impl_addTrackedResult(", codeToTrack, ")\n"));
            ilcLuaTrackedNames.emplaceBack(codeToTrack);

            ilcTrackBuffer[0] = '\0';
            ImGui::SetItemDefaultFocus();
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::SameLine();

        if (ImGui::Button("Untrack All"))
        {
            ilcLuaTracked.clear();
            ilcLuaTrackedNames.clear();
        }
    }

    if (ilcLuaTracked.size() > 0)
    {
        ilcLuaTrackedResults.clear();
        bool problem = false;

        for (sf::base::SizeT i = 0; i < ilcLuaTracked.size(); ++i)
        {
            const sf::base::String& code = ilcLuaTracked[i];

            try
            {
                lua.executeCode(code.cStr());
            } catch (std::runtime_error& e)
            {
                ilcCmdLog.emplaceBack(Utils::concat("[error]: error '", e.what(), "' while tracking ", code, '\n'));

                ilcLuaTracked.erase(ilcLuaTracked.begin() + i);
                ilcLuaTrackedNames.erase(ilcLuaTrackedNames.begin() + i);
                problem = true;
                break;
            } catch (...)
            {

                ilcCmdLog.emplaceBack(Utils::concat("[error]: unknown error while tracking ", code, '\n'));

                ilcLuaTracked.erase(ilcLuaTracked.begin() + i);
                ilcLuaTrackedNames.erase(ilcLuaTrackedNames.begin() + i);
                problem = true;
                break;
            }
        }

        if (!problem)
        {
            ImGui::Separator();
            SSVOH_ASSERT(ilcLuaTracked.size() == ilcLuaTrackedNames.size());
            SSVOH_ASSERT(ilcLuaTracked.size() == ilcLuaTrackedResults.size());

            if (ImGui::BeginTable("TrackedResults", 2))
            {
                for (sf::base::SizeT i = 0; i < ilcLuaTracked.size(); ++i)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(ilcLuaTrackedNames[i].cStr());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(ilcLuaTrackedResults[i].cStr());
                }

                ImGui::EndTable();
            }
        }
    }

    ImGui::End();
#endif
}

void HexagonGame::postUpdate()
{
    postUpdate_ImguiLuaConsole();
}

} // namespace hg
