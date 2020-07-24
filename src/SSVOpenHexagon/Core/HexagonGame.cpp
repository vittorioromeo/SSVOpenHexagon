// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/SoundPlayer/SoundPlayer.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <cassert>

using namespace hg::Utils;


namespace hg
{

[[nodiscard]] static random_number_generator initializeRng()
{
    const random_number_generator::seed_type seed = ssvu::getRndEngine()();
    return random_number_generator{seed};
}

void HexagonGame::createWall(int mSide, float mThickness,
    const SpeedData& mSpeed, const SpeedData& mCurve, float mHueMod)
{
    walls.emplace_back(*this, centerPos, mSide, mThickness,
        Config::getSpawnDistance(), mSpeed, mCurve);

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
      assets(mAssets),
      window(mGameWindow), player{ssvs::zeroVec2f}, rng{initializeRng()},
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

    add2StateInput(game, Config::getTriggerRotateCW(), inputImplCW);
    add2StateInput(game, Config::getTriggerRotateCCW(), inputImplCCW);
    add2StateInput(game, Config::getTriggerFocus(), inputFocused);
    add2StateInput(game, Config::getTriggerSwap(), inputSwap);
    game.addInput(
        Config::getTriggerExit(), [this](ssvu::FT /*unused*/) { goToMenu(); });
    game.addInput(
        Config::getTriggerForceRestart(),
        [this](ssvu::FT /*unused*/) {
            status.mustStateChange = StateChange::MustRestart;
        },
        ssvs::Input::Type::Once);
    game.addInput(
        Config::getTriggerRestart(),
        [this](ssvu::FT /*unused*/) {
            if(status.hasDied)
            {
                status.mustStateChange = StateChange::MustRestart;
            }
        },
        ssvs::Input::Type::Once);
    game.addInput(
        Config::getTriggerReplay(),
        [this](ssvu::FT /*unused*/) {
            if(status.hasDied)
            {
                status.mustStateChange = StateChange::MustReplay;
            }
        },
        ssvs::Input::Type::Once);
    game.addInput(
        Config::getTriggerScreenshot(),
        [this](ssvu::FT /*unused*/) { mustTakeScreenshot = true; },
        ssvs::Input::Type::Once);

    initKeyIcons();
}

void HexagonGame::newGame(const std::string& mPackId, const std::string& mId,
    bool mFirstPlay, float mDifficultyMult, bool executeLastReplay)
{
    initFlashEffect();

    firstPlay = mFirstPlay;
    setLevelData(assets.getLevelData(mId), mFirstPlay);
    difficultyMult = mDifficultyMult;

    const double tempPlayedFrametime = status.getPlayedAccumulatedFrametime();
    status = HexagonGameStatus{};

    if(!executeLastReplay)
    {
        // TODO: this can be used to restore normal speed
        // window.setTimer<ssvs::TimerStatic>(0.5f, 0.5f);

        rng = initializeRng();

        // Save data for immediate replay.
        lastSeed = rng.seed();
        lastReplayData = replay_data{};

        // Clear any existing active replay.
        activeReplay.reset();
    }
    else
    {
        if(!activeReplay.has_value())
        {
            lastPlayedFrametime = tempPlayedFrametime;
        }

        // TODO: this can be used to speed up the replay
        // window.setTimer<ssvs::TimerStatic>(0.5f, 0.1f);

        activeReplay.emplace(replay_file{
            ._version{0},
            ._player_name{assets.getCurrentLocalProfile().getName()}, // TODO
            ._seed{lastSeed},
            ._data{lastReplayData},
            ._pack_id{mPackId},
            ._level_id{mId},
            ._difficulty_mult{mDifficultyMult},
            ._played_frametime{lastPlayedFrametime},
        });

        activeReplay->replayPackName =
            Utils::toUppercase(assets.getPackData(mPackId).name);

        activeReplay->replayLevelName = Utils::toUppercase(levelData->name);

        rng = random_number_generator{activeReplay->replayFile._seed};
    }

    // Audio cleanup
    assets.stopSounds();
    stopLevelMusic();
    // assets.playSound("go.ogg");
    if(!Config::getNoMusic())
    {
        playLevelMusic();
        assets.musicPlayer.pause();

        auto* current(assets.getMusicPlayer().getCurrent());
        if(current != nullptr)
        {
            current->setPitch(
                (Config::getMusicSpeedDMSync() ? pow(difficultyMult, 0.12f)
                                               : 1.f) *
                Config::getMusicSpeedMult());
        }
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
    player = CPlayer{ssvs::zeroVec2f};

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

    // LUA context and game status cleanup
    inputImplCCW = inputImplCW = inputImplBothCWCCW = false;

    if(!mFirstPlay)
    {
        runLuaFunction<void>("onUnload");
    }

    lua = Lua::LuaContext{};
    initLua();

    // ------------------------------------------------------------------------
    // Initialize Lua random seed from random generator one:
    try
    {
        lua.executeCode("math.randomseed(u_getAttemptRandomSeed())");
    }
    catch(...)
    {
        ssvu::lo("HexagonGame::negGame")
            << "Failure to initialize Lua random generator seed\n";
    }

    runLuaFile(levelData->luaScriptPath);
    runLuaFunction<void>("onInit");
    runLuaFunction<void>("onLoad");
    restartId = mId;
    restartFirstTime = false;
    setSides(levelStatus.sides);

    // Set initial values for some status fields from Lua
    status.beatPulseDelay += levelStatus.beatPulseInitialDelay;
    timeUntilRichPresenceUpdate = -1.f; // immediate update
}

void HexagonGame::death(bool mForce)
{
    fpsWatcher.disable();
    assets.playSound("death.ogg", ssvs::SoundPlayer::Mode::Abort);

    if(!mForce && (Config::getInvincible() || levelStatus.tutorialMode))
    {
        return;
    }
    assets.playSound("gameOver.ogg", ssvs::SoundPlayer::Mode::Abort);
    runLuaFunctionIfExists<void>("onDeath");

    if(!assets.pIsLocal() && Config::isEligibleForScore())
    {
        Online::trySendDeath();
    }

    status.flashEffect = 255;
    overlayCamera.setView(
        {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
            sf::Vector2f(Config::getWidth(), Config::getHeight())});
    backgroundCamera.setCenter(ssvs::zeroVec2f);
    shakeCamera(effectTimelineManager, overlayCamera);
    shakeCamera(effectTimelineManager, backgroundCamera);

    status.hasDied = true;
    stopLevelMusic();
    checkAndSaveScore();

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

    runLuaFunction<void>("onIncrement");
}

void HexagonGame::checkAndSaveScore()
{
    const float time = status.getTimeSeconds();

    // These are requirements that need to be met for a score to be valid
    if(!Config::isEligibleForScore())
    {
        ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
            << "Not saving score - not eligible - "
            << Config::getUneligibilityReason() << "\n";
        return;
    }

    if(status.scoreInvalid)
    {
        ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
            << "Not saving score - score invalidated\n";
    }

    if(assets.pIsLocal())
    {
        std::string localValidator{
            getLocalValidator(levelData->id, difficultyMult)};

        if(assets.getLocalScore(localValidator) < time)
        {
            assets.setLocalScore(localValidator, time);
        }

        assets.saveCurrentLocalProfile();
    }
    else
    {
        // These are requirements that need to be met for a score to be sent
        // online
        if(time < 8)
        {
            ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
                << "Not sending score - less than 8 seconds\n";
            return;
        }

        if(Online::getServerVersion() == -1)
        {
            ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
                << "Not sending score - connection error\n";
            return;
        }

        if(Online::getServerVersion() > Config::getVersion())
        {
            ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
                << "Not sending score - version mismatch\n";
            return;
        }

        Online::trySendScore(levelData->id, difficultyMult, time);
    }
}

void HexagonGame::goToMenu(bool mSendScores, bool mError)
{
    assets.stopSounds();
    if(!mError)
    {
        assets.playSound("beep.ogg");
    }
    fpsWatcher.disable();

    if(mSendScores && !status.hasDied && !mError)
    {
        checkAndSaveScore();
    }
    runLuaFunction<void>("onUnload");
    window.setGameState(mgPtr->getGame());
    mgPtr->init(mError);
}

void HexagonGame::addMessage(
    std::string mMessage, double mDuration, bool mSoundToggle)
{
    Utils::uppercasify(mMessage);

    messageTimeline.append_do([this, mSoundToggle, mMessage] {
        if(mSoundToggle)
        {
            assets.playSound("beep.ogg");
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

[[nodiscard]] const std::string& HexagonGame::getPackId() const
{
    return levelData->packId;
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

void HexagonGame::invalidateScore(std::string mReason)
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

void HexagonGame::setSides(unsigned int mSides)
{
    assets.playSound("beep.ogg");

    if(mSides < 3)
    {
        mSides = 3;
    }

    levelStatus.sides = mSides;
}

[[nodiscard]] bool HexagonGame::getInputFocused() const
{
    return inputFocused;
}

[[nodiscard]] bool HexagonGame::getInputSwap() const
{
    return inputSwap || hg::Joystick::aRisingEdge();
}

[[nodiscard]] int HexagonGame::getInputMovement() const
{
    return inputMovement;
}

[[nodiscard]] bool HexagonGame::mustReplayInput() const noexcept
{
    return activeReplay.has_value() && !activeReplay->replayPlayer.done();
}

[[nodiscard]] bool HexagonGame::mustShowReplayUI() const noexcept
{
    return activeReplay.has_value();
}

} // namespace hg
