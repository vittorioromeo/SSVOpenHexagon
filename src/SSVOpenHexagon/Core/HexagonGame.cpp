// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/SoundPlayer/SoundPlayer.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/Graphics.hpp>

#include <cassert>

using namespace hg::Utils;


namespace hg
{

namespace
{

[[nodiscard]] double getReplayScore(const HexagonGameStatus& status)
{
    return status.getCustomScore() != 0.f
               ? status.getCustomScore()
               : status.getPlayedAccumulatedFrametime();
}

} // namespace

[[nodiscard]] static random_number_generator initializeRng()
{
    const random_number_generator::seed_type seed = ssvu::getRndEngine()();
    return random_number_generator{seed};
}

void HexagonGame::createWall(int mSide, float mThickness,
    const SpeedData& mSpeed, const SpeedData& mCurve, float mHueMod)
{
    walls.emplace_back(*this, centerPos, mSide, mThickness,
        levelStatus.wallSpawnDistance, mSpeed, mCurve);

    walls.back().setHueMod(mHueMod);
}

void HexagonGame::initKeyIcons()
{
    for(const auto& t :
        {"keyArrow.png", "keyFocus.png", "keySwap.png", "replayIcon.png"})
    {
        assets.get<sf::Texture>(t).setSmooth(true);
    }

    keyIconLeft.setTexture(assets.get<sf::Texture>("keyArrow.png"));
    keyIconRight.setTexture(assets.get<sf::Texture>("keyArrow.png"));
    keyIconFocus.setTexture(assets.get<sf::Texture>("keyFocus.png"));
    keyIconSwap.setTexture(assets.get<sf::Texture>("keySwap.png"));
    replayIcon.setTexture(assets.get<sf::Texture>("replayIcon.png"));

    updateKeyIcons();
}

void HexagonGame::updateKeyIcons()
{
    constexpr float halfSize = 32.f;
    constexpr float size = halfSize * 2.f;

    keyIconLeft.setOrigin({halfSize, halfSize});
    keyIconRight.setOrigin({halfSize, halfSize});
    keyIconFocus.setOrigin({halfSize, halfSize});
    keyIconSwap.setOrigin({halfSize, halfSize});

    keyIconLeft.setRotation(180);

    const float scaling = Config::getKeyIconsScale() / Config::getZoomFactor();

    keyIconLeft.setScale(scaling, scaling);
    keyIconRight.setScale(scaling, scaling);
    keyIconFocus.setScale(scaling, scaling);
    keyIconSwap.setScale(scaling, scaling);

    const float scaledHalfSize = halfSize * scaling;
    const float scaledSize = size * scaling;
    const float padding = 8.f * scaling;
    const float finalPadding = scaledSize + padding;
    const sf::Vector2f finalPaddingX{finalPadding, 0.f};

    const sf::Vector2f bottomRight{
        Config::getWidth() - padding - scaledHalfSize,
        Config::getHeight() - padding - scaledHalfSize};

    keyIconSwap.setPosition(bottomRight);
    keyIconFocus.setPosition(keyIconSwap.getPosition() - finalPaddingX);
    keyIconRight.setPosition(keyIconFocus.getPosition() - finalPaddingX);
    keyIconLeft.setPosition(keyIconRight.getPosition() - finalPaddingX);

    // ------------------------------------------------------------------------

    replayIcon.setOrigin({size, size});
    replayIcon.setScale(scaling, scaling);

    const sf::Vector2f topRight{
        Config::getWidth() - padding - scaledSize, padding + scaledSize};

    replayIcon.setPosition(topRight);
}

HexagonGame::HexagonGame(Steam::steam_manager& mSteamManager,
    Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
    ssvs::GameWindow& mGameWindow)
    : steamManager(mSteamManager), discordManager(mDiscordManager),
      assets(mAssets), window(mGameWindow),
      player{ssvs::zeroVec2f, getSwapCooldown()}, rng{initializeRng()},
      fpsWatcher(window)
{
    game.onUpdate += [this](ssvu::FT mFT) { update(mFT); };

    game.onPostUpdate += [this] {
        inputImplLastMovement = inputMovement;
        inputImplBothCWCCW = inputImplCW && inputImplCCW;
    };

    game.onDraw += [this] { draw(); };

    window.onRecreation += [this] {
        initFlashEffect();
        initKeyIcons();
    };

    // ------------------------------------------------------------------------
    // keyboard binds
    Config::keyboardBindsSanityCheck();

    using Tid = Config::Tid;

    add2StateInput(
        game, Config::getTriggerRotateCCW(), inputImplCCW, Tid::RotateCCW);
    add2StateInput(
        game, Config::getTriggerRotateCW(), inputImplCW, Tid::RotateCW);
    add2StateInput(game, Config::getTriggerFocus(), inputFocused, Tid::Focus);
    add2StateInput(game, Config::getTriggerSwap(), inputSwap, Tid::Swap);

    game.addInput(
        {{sf::Keyboard::Key::Escape}},
        [this](ssvu::FT /*unused*/) { goToMenu(); }, // hardcoded
        ssvs::Input::Type::Always);

    game.addInput(
        Config::getTriggerExit(),
        [this](ssvu::FT /*unused*/) { goToMenu(); }, // editable
        ssvs::Input::Type::Always, Tid::Exit);

    game.addInput(
        Config::getTriggerForceRestart(),
        [this](ssvu::FT /*unused*/) {
            status.mustStateChange = StateChange::MustRestart;
        },
        ssvs::Input::Type::Once, Tid::ForceRestart);

    game.addInput(
        Config::getTriggerRestart(),
        [this](ssvu::FT /*unused*/) {
            if(status.hasDied)
            {
                status.mustStateChange = StateChange::MustRestart;
            }
        },
        ssvs::Input::Type::Once, Tid::Restart);

    game.addInput(
        Config::getTriggerReplay(),
        [this](ssvu::FT /*unused*/) {
            if(status.hasDied)
            {
                status.mustStateChange = StateChange::MustReplay;
            }
        },
        ssvs::Input::Type::Once, Tid::Replay);

    game.addInput(
        Config::getTriggerScreenshot(),
        [this](ssvu::FT /*unused*/) { mustTakeScreenshot = true; },
        ssvs::Input::Type::Once, Tid::Screenshot);

    // ------------------------------------------------------------------------
    // joystick binds
    Config::joystickBindsSanityCheck();

    hg::Joystick::setJoystickBind(
        Config::getJoystickSelect(), hg::Joystick::Jid::Select);

    hg::Joystick::setJoystickBind(
        Config::getJoystickExit(), hg::Joystick::Jid::Exit);

    hg::Joystick::setJoystickBind(
        Config::getJoystickFocus(), hg::Joystick::Jid::Focus);

    hg::Joystick::setJoystickBind(
        Config::getJoystickSwap(), hg::Joystick::Jid::Swap);

    hg::Joystick::setJoystickBind(
        Config::getJoystickForceRestart(), hg::Joystick::Jid::ForceRestart);

    hg::Joystick::setJoystickBind(
        Config::getJoystickRestart(), hg::Joystick::Jid::Restart);

    hg::Joystick::setJoystickBind(
        Config::getJoystickReplay(), hg::Joystick::Jid::Replay);

    hg::Joystick::setJoystickBind(
        Config::getJoystickScreenshot(), hg::Joystick::Jid::Screenshot);

    // ------------------------------------------------------------------------
    // key icons
    initKeyIcons();
}

void HexagonGame::setLastReplay(const replay_file& mReplayFile)
{
    lastSeed = mReplayFile._seed;
    lastReplayData = mReplayFile._data;
    lastFirstPlay = mReplayFile._first_play;
    lastPlayedScore = mReplayFile._played_score;

    activeReplay.emplace(mReplayFile);
}

void HexagonGame::updateRichPresenceCallbacks()
{
    // Update Steam Rich Presence
    if(!steamHung)
    {
        if(!steamManager.run_callbacks())
        {
            steamAttempt += 1;
            if(steamAttempt > 20)
            {
                steamHung = true;
                ssvu::lo("Steam") << "Too many failed callbacks. Stopping "
                                     "Steam callbacks.\n";
            }
        }
    }

    // Update Discord Rich Presence
    if(!discordHung)
    {
        if(!discordManager.run_callbacks())
        {
            discordAttempt += 1;
            if(discordAttempt > 20)
            {
                discordHung = true;
                ssvu::lo("Discord") << "Too many failed callbacks. Stopping "
                                       "Discord callbacks.\n";
            }
        }
    }
}

void HexagonGame::newGame(const std::string& mPackId, const std::string& mId,
    bool mFirstPlay, float mDifficultyMult, bool executeLastReplay)
{
    if(executeLastReplay)
    {
        fpsWatcher.disable();
    }

    initFlashEffect();

    packId = mPackId;
    levelId = mId;

    if(executeLastReplay && activeReplay.has_value())
    {
        firstPlay = activeReplay->replayFile._first_play;
    }
    else
    {
        firstPlay = mFirstPlay;
    }

    setLevelData(assets.getLevelData(mId), mFirstPlay);
    difficultyMult = mDifficultyMult;

    const double tempReplayScore = getReplayScore(status);
    status = HexagonGameStatus{};

    if(!executeLastReplay)
    {
        // TODO: this can be used to restore normal speed
        // window.setTimer<ssvs::TimerStatic>(0.5f, 0.5f);

        rng = initializeRng();

        // Save data for immediate replay.
        lastSeed = rng.seed();
        lastReplayData = replay_data{};
        lastFirstPlay = mFirstPlay;

        // Clear any existing active replay.
        activeReplay.reset();
    }
    else
    {
        if(!activeReplay.has_value())
        {
            lastPlayedScore = tempReplayScore;

            activeReplay.emplace(replay_file{
                ._version{0},
                ._player_name{
                    assets.getCurrentLocalProfile().getName()}, // TODO
                ._seed{lastSeed},
                ._data{lastReplayData},
                ._pack_id{mPackId},
                ._level_id{mId},
                ._first_play{lastFirstPlay},
                ._difficulty_mult{mDifficultyMult},
                ._played_score{lastPlayedScore},
            });
        }

        activeReplay->replayPlayer.reset();

        activeReplay->replayPackName =
            Utils::toUppercase(assets.getPackData(mPackId).name);

        activeReplay->replayLevelName = Utils::toUppercase(levelData->name);

        // TODO: this can be used to speed up the replay
        // window.setTimer<ssvs::TimerStatic>(0.5f, 0.1f);

        rng = random_number_generator{activeReplay->replayFile._seed};
        firstPlay = activeReplay->replayFile._first_play;
    }

    // Audio cleanup
    assets.stopSounds();
    stopLevelMusic();
    // assets.playSound("go.ogg");
    if(!Config::getNoMusic())
    {
        playLevelMusic();
        assets.musicPlayer.pause();

        sf::Music* current(assets.getMusicPlayer().getCurrent());
        if(current != nullptr)
        {
            setMusicPitch(*current);
        }
    }
    else
    {
        assets.musicPlayer.stop();
    }

    // Events cleanup
    messageText.setString("");

    // Event timeline cleanup
    eventTimeline.clear();
    eventTimelineRunner = {};

    // Message timeline cleanup
    messageTimeline.clear();
    messageTimelineRunner = {};

    // Manager cleanup
    walls.clear();
    cwManager.clear();
    player = CPlayer{ssvs::zeroVec2f, getSwapCooldown()};

    // Timeline cleanup
    timeline.clear();
    timelineRunner = {};

    effectTimelineManager.clear();
    mustChangeSides = false;

    // FPSWatcher reset
    fpsWatcher.reset();
    // if(Config::getOfficial()) fpsWatcher.enable();

    // Reset zoom
    overlayCamera.setView(
        {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
            sf::Vector2f(Config::getWidth(), Config::getHeight())});
    backgroundCamera.setView(
        {ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(),
                              Config::getHeight() * Config::getZoomFactor()}});
    backgroundCamera.setRotation(0);

    // Reset skew
    overlayCamera.setSkew(sf::Vector2f{1.f, 1.f});
    backgroundCamera.setSkew(sf::Vector2f{1.f, 1.f});

    // Set drawing mode
    setDrawFunc();

    // LUA context and game status cleanup
    inputImplCCW = inputImplCW = inputImplBothCWCCW = false;

    lua = Lua::LuaContext{};
    calledDeprecatedFunctions.clear();
    initLua();
    runLuaFile(levelData->luaScriptPath);

    if(!firstPlay)
    {
        runLuaFunction<void>("onUnload");
        assets.playSound("restart.ogg");
    }
    else
    {
        assets.playSound("select.ogg");
    }

    runLuaFunction<void>("onInit");

    restartId = mId;
    restartFirstTime = false;
    setSides(levelStatus.sides);

    // Set initial values for some status fields from Lua
    status.beatPulseDelay += levelStatus.beatPulseInitialDelay;
    timeUntilRichPresenceUpdate = -1.f; // immediate update

    // Store the keys/buttons to be pressed to replay and restart after you die.
    using Tid = Config::Tid;
    status.restartInput = Config::getKeyboardBindNames(Tid::Restart);
    status.replayInput = Config::getKeyboardBindNames(Tid::Replay);

    // Format strings to only show the first key to avoid extremely long
    // messages
    int commaPos = status.restartInput.find(',');
    if(commaPos > 0)
    {
        status.restartInput.erase(commaPos);
    }
    commaPos = status.replayInput.find(',');
    if(commaPos > 0)
    {
        status.replayInput.erase(commaPos);
    }

    // Add joystick buttons if any and finalize message
    std::string joystickButton =
        Config::getJoystickBindNames(Joystick::Jid::Restart);
    if(!status.restartInput.empty())
    {
        if(!joystickButton.empty())
        {
            status.restartInput += " OR JOYSTICK " + joystickButton;
        }
        status.restartInput = "PRESS " + status.restartInput + " TO RESTART\n";
    }
    else if(!joystickButton.empty())
    {
        status.restartInput =
            "PRESS JOYSTICK " + joystickButton + " TO RESTART\n";
    }
    else
    {
        status.restartInput = "NO RESTART BUTTON SET\n";
    }
    joystickButton = Config::getJoystickBindNames(Joystick::Jid::Replay);
    if(!status.replayInput.empty())
    {
        if(!joystickButton.empty())
        {
            status.replayInput += " OR JOYSTICK " + joystickButton;
        }
        status.replayInput = "PRESS " + status.replayInput + " TO REPLAY\n";
    }
    else if(!joystickButton.empty())
    {
        status.replayInput =
            "PRESS JOYSTICK " + joystickButton + " TO REPLAY\n";
    }
    else
    {
        status.replayInput = "NO REPLAY BUTTON SET\n";
    }
}

void HexagonGame::death(bool mForce)
{
    if(status.hasDied)
    {
        return;
    }

    fpsWatcher.disable();
    assets.playSound(levelStatus.deathSound, ssvs::SoundPlayer::Mode::Abort);

    if(!mForce && (Config::getInvincible() || levelStatus.tutorialMode))
    {
        return;
    }

    assets.playSound("gameOver.ogg", ssvs::SoundPlayer::Mode::Abort);
    runLuaFunctionIfExists<void>("onDeath");

    status.flashEffect = 255;
    overlayCamera.setView(
        {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
            sf::Vector2f(Config::getWidth(), Config::getHeight())});
    backgroundCamera.setCenter(ssvs::zeroVec2f);
    shakeCamera(effectTimelineManager, overlayCamera);
    shakeCamera(effectTimelineManager, backgroundCamera);

    status.hasDied = true;
    stopLevelMusic();

    if(inReplay())
    {
        // Do not save scores or update rich presence if watching a replay.
        return;
    }

    // Gather player's Personal Best
    std::string pbStr = "(";
    if(status.getTimeSeconds() >
        assets.getLocalScore(getLocalValidator(levelData->id, difficultyMult)))
    {
        pbStr += "New PB!)";
    }
    else
    {
        pbStr += "PB: " +
                 timeFormat(assets.getLocalScore(
                     getLocalValidator(levelData->id, difficultyMult))) +
                 "s)";
    }

    std::string nameStr = levelData->name;
    nameFormat(nameStr);
    const std::string diffStr = diffFormat(difficultyMult);
    const std::string timeStr = timeFormat(status.getTimeSeconds());
    discordManager.set_rich_presence_in_game(
        nameStr + " [x" + diffStr + "]", "Survived " + timeStr + "s", true);

    const bool localNewBest =
        checkAndSaveScore() == CheckSaveScoreResult::Local_NewBest;

    // TODO: more options? Always save replay? Prompt?
    if(Config::getSaveLocalBestReplayToFile() && localNewBest)
    {
        const replay_file rf{
            ._version{0},
            ._player_name{assets.getCurrentLocalProfile().getName()}, // TODO
            ._seed{lastSeed},
            ._data{lastReplayData},
            ._pack_id{packId},
            ._level_id{levelId},
            ._first_play{firstPlay},
            ._difficulty_mult{difficultyMult},
            ._played_score{getReplayScore(status)},
        };

        const std::string filename = rf.create_filename();

        std::filesystem::create_directory("Replays/");

        std::filesystem::path p;
        p /= "Replays/";
        p /= filename;

        if(rf.serialize_to_file(p))
        {
            ssvu::lo("Replay")
                << "Successfully saved new local best replay file '" << p
                << "'\n";
        }
        else
        {
            ssvu::lo("Replay")
                << "Failed to save new local best replay file '" << p << "'\n";
        }
    }

    if(Config::getAutoRestart())
    {
        status.mustStateChange = StateChange::MustRestart;
    }
}

void HexagonGame::incrementDifficulty()
{
    assets.playSound("levelUp.ogg");

    const float signMult = (levelStatus.rotationSpeed > 0.f) ? 1.f : -1.f;

    levelStatus.rotationSpeed += levelStatus.rotationSpeedInc * signMult;

    const auto& rotationSpeedMax(levelStatus.rotationSpeedMax);
    if(abs(levelStatus.rotationSpeed) > rotationSpeedMax)
    {
        levelStatus.rotationSpeed = rotationSpeedMax * signMult;
    }

    levelStatus.rotationSpeed *= -1.f;
    status.fastSpin = levelStatus.fastSpin;
}

void HexagonGame::sideChange(unsigned int mSideNumber)
{
    levelStatus.speedMult += levelStatus.speedInc;
    levelStatus.delayMult += levelStatus.delayInc;

    if(levelStatus.rndSideChangesEnabled)
    {
        setSides(mSideNumber);
    }

    mustChangeSides = false;

    assets.playSound(levelStatus.levelUpSound);
    runLuaFunction<void>("onIncrement");
}

HexagonGame::CheckSaveScoreResult HexagonGame::checkAndSaveScore()
{
    const float score = levelStatus.scoreOverridden
                            ? lua.readVariable<float>(levelStatus.scoreOverride)
                            : status.getTimeSeconds();

    // These are requirements that need to be met for a score to be valid
    if(!Config::isEligibleForScore())
    {
        ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
            << "Not saving score - not eligible - "
            << Config::getUneligibilityReason() << "\n";

        return CheckSaveScoreResult::Ineligible;
    }

    if(status.scoreInvalid)
    {
        ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
            << "Not saving score - score invalidated\n";

        return CheckSaveScoreResult::Invalid;
    }

    if(assets.pIsLocal())
    {
        std::string localValidator{
            getLocalValidator(levelData->id, difficultyMult)};

        // TODO: this crashes when going back to menu from replay drag and drop
        if(assets.getLocalScore(localValidator) < score)
        {
            assets.setLocalScore(localValidator, score);
            assets.saveCurrentLocalProfile();

            return CheckSaveScoreResult::Local_NewBest;
        }

        return CheckSaveScoreResult::Local_NoNewBest;
    }

    assert(false);
    return CheckSaveScoreResult::Local_NoNewBest;
}

void HexagonGame::goToMenu(bool mSendScores, bool mError)
{
    assets.stopSounds();
    
    wallQuads.clear();
    playerTris.clear();
    capQuads.clear();
    capTris.clear();
    wallQuads3D.clear();
    playerTris3D.clear();

    if(!mError)
    {
        assets.playSound("beep.ogg");
    }

    calledDeprecatedFunctions.clear();
    fpsWatcher.disable();

    if(mSendScores && !status.hasDied && !mError && !inReplay())
    {
        checkAndSaveScore();
    }

    // Stop infinite feedback from occurring if the error is happening on
    // onUnload.
    if(!mError)
    {
        runLuaFunction<void>("onUnload");
    }

    window.setGameState(mgPtr->getGame());
    mgPtr->returnToLevelSelection();
    mgPtr->init(mError);
}

void HexagonGame::raiseWarning(
    const std::string& mFunctionName, const std::string& mAdditionalInfo)
{
    // Only raise the warning once to avoid redundancy
    if(!calledDeprecatedFunctions.contains(mFunctionName))
    {
        calledDeprecatedFunctions.emplace(mFunctionName);
        // Raise warning to the console
        std::cout << "[Lua] WARNING: The function \"" << mFunctionName
                  << "\" (used in level \"" << levelData->name
                  << "\") is deprecated. " << mAdditionalInfo << std::endl;
    }
}

void HexagonGame::addMessage(
    std::string mMessage, double mDuration, bool mSoundToggle)
{
    Utils::uppercasify(mMessage);

    messageTimeline.append_do([this, mSoundToggle, mMessage] {
        if(mSoundToggle)
        {
            assets.playSound(levelStatus.beepSound);
        }
        messageText.setString(mMessage);
    });

    messageTimeline.append_wait_for_sixths(mDuration);
    messageTimeline.append_do([this] { messageText.setString(""); });
}

void HexagonGame::clearMessages()
{
    messageTimeline.clear();
}

void HexagonGame::setLevelData(
    const LevelData& mLevelData, bool mMusicFirstPlay)
{
    levelData = &mLevelData;
    levelStatus = LevelStatus{};
    styleData = assets.getStyleData(levelData->packId, levelData->styleId);
    musicData = assets.getMusicData(levelData->packId, levelData->musicId);
    musicData.firstPlay = mMusicFirstPlay;
}

[[nodiscard]] const std::string& HexagonGame::getPackId() const noexcept
{
    return levelData->packId;
}

[[nodiscard]] const std::string&
HexagonGame::getPackDisambiguator() const noexcept
{
    return assets.getPackData(getPackId()).disambiguator;
}

[[nodiscard]] const std::string& HexagonGame::getPackAuthor() const noexcept
{
    return assets.getPackData(getPackId()).author;
}

[[nodiscard]] const std::string& HexagonGame::getPackName() const noexcept
{
    return assets.getPackData(getPackId()).name;
}

[[nodiscard]] int HexagonGame::getPackVersion() const noexcept
{
    return assets.getPackData(getPackId()).version;
}

void HexagonGame::playLevelMusic()
{
    if(!Config::getNoMusic())
    {
        const MusicData::Segment segment =
            musicData.playRandomSegment(getPackId(), assets);
        status.beatPulseDelay += segment.beatPulseDelayOffset;
    }
}

void HexagonGame::playLevelMusicAtTime(float mSeconds)
{
    if(!Config::getNoMusic())
    {
        musicData.playSeconds(getPackId(), assets, mSeconds);
    }
}

void HexagonGame::stopLevelMusic()
{
    if(!Config::getNoMusic())
    {
        assets.stopMusics();
    }
}

void HexagonGame::invalidateScore(const std::string& mReason)
{
    status.scoreInvalid = true;
    status.invalidReason = mReason;
    ssvu::lo("HexagonGame::invalidateScore")
        << "Invalidating official game (" << mReason << ")\n";
}

auto HexagonGame::getColorMain() const -> sf::Color
{
    if(Config::getBlackAndWhite())
    {
        //			if(status.drawing3D) return Color{255, 255, 255,
        // status.overrideColor.a};
        return sf::Color(255, 255, 255, styleData.getMainColor().a);
    }
    //	else if(status.drawing3D) return status.overrideColor;
    {
        return styleData.getMainColor();
    }
}

auto HexagonGame::getColorPlayer() const -> sf::Color
{
    if(Config::getBlackAndWhite())
    {
        return sf::Color(255, 255, 255, styleData.getPlayerColor().a);
    }

    return styleData.getPlayerColor();
}

auto HexagonGame::getColorText() const -> sf::Color
{
    if(Config::getBlackAndWhite())
    {
        return sf::Color(255, 255, 255, styleData.getTextColor().a);
    }

    return styleData.getTextColor();
}

void HexagonGame::setSides(unsigned int mSides)
{
    assets.playSound(levelStatus.beepSound);

    if(mSides < 3)
    {
        mSides = 3;
    }

    levelStatus.sides = mSides;
    player.setSides(levelStatus.sides + 1);
}

[[nodiscard]] bool HexagonGame::getInputFocused() const
{
    // TODO: the joystick thing should be in updateInput, this should be a blind
    // getter
    return inputFocused || (!inReplay() && hg::Joystick::focusPressed());
}

[[nodiscard]] float HexagonGame::getPlayerSpeedMult() const
{
    return levelStatus.playerSpeedMult;
}

[[nodiscard]] bool HexagonGame::getInputSwap() const
{
    // TODO: the joystick thing should be in updateInput, this should be a blind
    // getter
    return inputSwap || (!inReplay() && hg::Joystick::swapPressed());
}

[[nodiscard]] int HexagonGame::getInputMovement() const
{
    return inputMovement;
}

[[nodiscard]] bool HexagonGame::inReplay() const noexcept
{
    return activeReplay.has_value();
}

[[nodiscard]] bool HexagonGame::mustReplayInput() const noexcept
{
    return inReplay() && !activeReplay->replayPlayer.done();
}

[[nodiscard]] bool HexagonGame::mustShowReplayUI() const noexcept
{
    return inReplay();
}

[[nodiscard]] float HexagonGame::getSwapCooldown() const noexcept
{
    return std::max(36.f * levelStatus.swapCooldownMult, 8.f);
}

} // namespace hg
