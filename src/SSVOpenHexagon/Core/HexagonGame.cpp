// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Core/HexagonClient.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/LevelValidator.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/SoundPlayer/SoundPlayer.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/Graphics.hpp>

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

[[nodiscard]] random_number_generator initializeRng()
{
    const random_number_generator::seed_type seed = ssvu::getRndEngine()();
    return random_number_generator{seed};
}

} // namespace

void HexagonGame::createWall(int mSide, float mThickness,
    const SpeedData& mSpeed, const SpeedData& mCurve, float mHueMod)
{
    walls.emplace_back(getSides(), getWallAngleLeft(), getWallAngleRight(),
        getWallSkewLeft(), getWallSkewRight(), centerPos, mSide, mThickness,
        levelStatus.wallSpawnDistance, mSpeed, mCurve);

    walls.back().setHueMod(mHueMod);
}

void HexagonGame::initKeyIcons()
{
    if(window == nullptr)
    {
        return;
    }

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
    if(window == nullptr)
    {
        return;
    }

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
    replayIcon.setScale(scaling / 2.f, scaling / 2.f);

    const sf::Vector2f topRight{Config::getWidth() - padding - scaledHalfSize,
        padding + scaledHalfSize};

    replayIcon.setPosition(topRight);
}

void HexagonGame::updateLevelInfo()
{
    if(window == nullptr)
    {
        return;
    }

    const float levelInfoScaling = 1.f;
    const float scaling = levelInfoScaling / Config::getZoomFactor();
    const float padding = 8.f * scaling;

    const sf::Vector2f size{325.f, 75.f};
    const sf::Vector2f halfSize{size / 2.f};
    const sf::Vector2f scaledHalfSize{halfSize * scaling};

    levelInfoRectangle.setSize(size);
    levelInfoRectangle.setScale(scaling, scaling);

    const sf::Color offsetColor{
        Config::getBlackAndWhite() || styleData.getColors().empty()
            ? sf::Color::Black
            : styleData.getColor(0)};

    levelInfoRectangle.setFillColor(offsetColor);
    levelInfoRectangle.setOutlineColor(styleData.getMainColor());
    levelInfoRectangle.setOrigin(halfSize);
    levelInfoRectangle.setOutlineThickness(3.f);

    const sf::Vector2f bottomLeft{padding + scaledHalfSize.x,
        Config::getHeight() - padding - scaledHalfSize.y};

    levelInfoRectangle.setPosition(bottomLeft);

    const float tPadding = padding;

    const auto trim = [](std::string s) {
        if(s.size() > 28)
        {
            return s.substr(0, 28);
        }

        return s;
    };

    levelInfoTextLevel.setFillColor(styleData.getMainColor());
    levelInfoTextLevel.setCharacterSize(20.f / Config::getZoomFactor());
    levelInfoTextLevel.setString(trim(Utils::toUppercase(levelData->name)));
    levelInfoTextLevel.setOrigin(ssvs::getLocalNW(levelInfoTextLevel));
    levelInfoTextLevel.setPosition(ssvs::getGlobalNW(levelInfoRectangle) +
                                   sf::Vector2f{tPadding, tPadding});

    const auto prepareText = [&](sf::Text& text, const float characterSize,
                                 const std::string& string) {
        text.setFillColor(styleData.getTextColor());
        text.setCharacterSize(characterSize / Config::getZoomFactor());
        text.setString(string);
    };

    prepareText(
        levelInfoTextPack, 14.f, trim(Utils::toUppercase(getPackName())));
    levelInfoTextPack.setOrigin(ssvs::getLocalNW(levelInfoTextPack));
    levelInfoTextPack.setPosition(
        ssvs::getGlobalSW(levelInfoTextLevel) + sf::Vector2f{0.f, tPadding});

    prepareText(
        levelInfoTextAuthor, 20.f, trim(Utils::toUppercase(getPackAuthor())));
    levelInfoTextAuthor.setOrigin(ssvs::getLocalSE(levelInfoTextAuthor));
    levelInfoTextAuthor.setPosition(ssvs::getGlobalSE(levelInfoRectangle) -
                                    sf::Vector2f{tPadding, tPadding});

    prepareText(levelInfoTextBy, 12.f, "BY");
    levelInfoTextBy.setOrigin(ssvs::getLocalSE(levelInfoTextBy));
    levelInfoTextBy.setPosition(
        ssvs::getGlobalSW(levelInfoTextAuthor) - sf::Vector2f{tPadding, 0.f});

    if(levelData->difficultyMults.size() > 1)
    {
        prepareText(levelInfoTextDM, 14.f, diffFormat(difficultyMult) + "x");
        levelInfoTextDM.setOrigin(ssvs::getLocalSW(levelInfoTextDM));
        levelInfoTextDM.setPosition(ssvs::getGlobalSW(levelInfoRectangle) +
                                    sf::Vector2f{tPadding, -tPadding});
    }
    else
    {
        levelInfoTextDM.setString("");
    }
}

[[nodiscard]] bool HexagonGame::imguiLuaConsoleHasInput()
{
    return ilcShowConsole && (ImGui::GetIO().WantCaptureKeyboard ||
                                 ImGui::GetIO().WantCaptureMouse);
}

HexagonGame::HexagonGame(Steam::steam_manager& mSteamManager,
    Discord::discord_manager* mDiscordManager, HGAssets& mAssets,
    ssvs::GameWindow* mGameWindow, HexagonClient* mHexagonClient)
    : steamManager(mSteamManager), discordManager(mDiscordManager),
      assets(mAssets), window(mGameWindow), hexagonClient{mHexagonClient},
      player{ssvs::zeroVec2f, getSwapCooldown()}, rng{initializeRng()}
{
    if(window != nullptr)
    {
        const float width = Config::getWidth();
        const float height = Config::getHeight();
        const float zoomFactor = Config::getZoomFactor();

        backgroundCamera.emplace(*window,
            sf::View{ssvs::zeroVec2f,
                sf::Vector2f{width * zoomFactor, height * zoomFactor}});

        overlayCamera.emplace(
            *window, sf::View{sf::Vector2f{width / 2.f, height / 2.f},
                         sf::Vector2f{width, height}});
    }


    game.onUpdate += [this](ssvu::FT mFT) { update(mFT); };

    game.onPostUpdate += [this] { postUpdate(); };

    game.onDraw += [this] { draw(); };

    game.onAnyEvent +=
        [](const sf::Event& event) { ImGui::SFML::ProcessEvent(event); };

    if(window != nullptr)
    {
        window->onRecreation += [this] {
            initFlashEffect();
            initKeyIcons();
        };
    }

    // ------------------------------------------------------------------------
    // Keyboard binds

    Config::keyboardBindsSanityCheck();

    using Tid = Config::Tid;

    const auto addTidInput = [&](const Tid tid, const ssvs::Input::Type type,
                                 auto action) {
        game.addInput(
            Config::getTrigger(tid), action, type, static_cast<int>(tid));
    };

    const auto addTid2StateInput = [&](const Tid tid, bool& value) {
        add2StateInput(
            game, Config::getTrigger(tid), value, static_cast<int>(tid));
    };

    addTid2StateInput(Tid::RotateCCW, inputImplCCW);
    addTid2StateInput(Tid::RotateCW, inputImplCW);
    addTid2StateInput(Tid::Focus, inputFocused);
    addTid2StateInput(Tid::Swap, inputSwap);

    const auto notInConsole = [this](auto&& f) {
        return [this, f](ssvu::FT /*unused*/) {
            if(!imguiLuaConsoleHasInput())
            {
                f();
            }
        };
    };

    game.addInput({{sf::Keyboard::Key::Escape}},
        notInConsole([this] { goToMenu(); }), // hardcoded
        ssvs::Input::Type::Always);

    addTidInput(Tid::Exit, ssvs::Input::Type::Always,
        notInConsole([this] { goToMenu(); }));

    addTidInput(Tid::ForceRestart, ssvs::Input::Type::Once,
        notInConsole(
            [this] { status.mustStateChange = StateChange::MustRestart; }));

    addTidInput(Tid::Restart, ssvs::Input::Type::Once, notInConsole([this] {
        if(deathInputIgnore <= 0.f && status.hasDied)
        {
            status.mustStateChange = StateChange::MustRestart;
        }
    }));

    addTidInput(Tid::Replay, ssvs::Input::Type::Once, notInConsole([this] {
        if(deathInputIgnore <= 0.f && status.hasDied)
        {
            status.mustStateChange = StateChange::MustReplay;
        }
    }));

    addTidInput(Tid::Screenshot, ssvs::Input::Type::Once,
        notInConsole([this] { mustTakeScreenshot = true; }));

    addTidInput(
        Tid::LuaConsole, ssvs::Input::Type::Once, [this](ssvu::FT /*unused*/) {
            if(Config::getDebug())
            {
                ilcShowConsoleNext = true;
            }
        });

    addTidInput(
        Tid::Pause, ssvs::Input::Type::Once, [this](ssvu::FT /*unused*/) {
            if(Config::getDebug())
            {
                debugPause = !debugPause;

                if(debugPause)
                {
                    assets.musicPlayer.pause();
                }
                else if(!status.hasDied)
                {
                    assets.musicPlayer.resume();
                }
            }
        });

    // ------------------------------------------------------------------------
    // Joystick binds

    Config::loadAllJoystickBinds();

    // ------------------------------------------------------------------------
    // key icons
    initKeyIcons();
}

HexagonGame::~HexagonGame()
{
    ssvu::lo("HexagonGame::~HexagonGame") << "Cleaning up game resources...\n";
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
    if(discordManager != nullptr && !discordHung)
    {
        if(!discordManager->run_callbacks())
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

void HexagonGame::playSound(
    const std::string& mId, ssvs::SoundPlayer::Mode mMode)
{
    if(window != nullptr)
    {
        assets.playSound(mId, mMode);
    }
}

void HexagonGame::newGame(const std::string& mPackId, const std::string& mId,
    bool mFirstPlay, float mDifficultyMult, bool executeLastReplay)
{
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

    debugPause = false;

    // Events cleanup
    messageText.setString("");
    pbText.setString("");

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
    mustStart = false;

    // Particles cleanup
    pbTextGrowth = 0.f;
    mustSpawnPBParticles = false;
    nextPBParticleSpawn = 0.f;
    particles.clear();

    if(window != nullptr)
    {
        SSVOH_ASSERT(overlayCamera.has_value());
        SSVOH_ASSERT(backgroundCamera.has_value());

        // Reset zoom
        overlayCamera->setView(
            {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
                sf::Vector2f(Config::getWidth(), Config::getHeight())});
        backgroundCamera->setView({ssvs::zeroVec2f,
            {Config::getWidth() * Config::getZoomFactor(),
                Config::getHeight() * Config::getZoomFactor()}});
        backgroundCamera->setRotation(0);

        // Reset skew
        overlayCamera->setSkew(sf::Vector2f{1.f, 1.f});
        backgroundCamera->setSkew(sf::Vector2f{1.f, 1.f});
    }

    // Lua context and game status cleanup
    inputImplCCW = inputImplCW = false;

    lua = Lua::LuaContext{};
    calledDeprecatedFunctions.clear();
    initLua();
    runLuaFile(levelData->luaScriptPath);

    if(!firstPlay)
    {
        runLuaFunctionIfExists<void>("onUnload");
        playSound("restart.ogg");
    }
    else
    {
        playSound("select.ogg");
    }

    runLuaFunctionIfExists<void>("onInit");

    restartId = mId;
    restartFirstTime = false;
    setSides(levelStatus.sides);

    // Set initial values for some status fields from Lua
    status.pulseDelay += levelStatus.pulseInitialDelay;
    status.beatPulseDelay += levelStatus.beatPulseInitialDelay;
    timeUntilRichPresenceUpdate = -1.f; // immediate update

    // Store the keys/buttons to be pressed to replay and restart after you
    // die.
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

    deathInputIgnore = 10.f;

    playSound(levelStatus.deathSound, ssvs::SoundPlayer::Mode::Abort);

    runLuaFunctionIfExists<void>("onPreDeath");

    if(!mForce && (Config::getInvincible() || levelStatus.tutorialMode))
    {
        return;
    }

    const bool isPersonalBest =
        !levelStatus.tutorialMode && !inReplay() &&
        (status.getTimeSeconds() >
            assets.getLocalScore(
                Utils::getLevelValidator(levelData->id, difficultyMult)));

    if(isPersonalBest)
    {
        pbText.setString("NEW PERSONAL BEST!");
        mustSpawnPBParticles = true;

        playSound("personalBest.ogg", ssvs::SoundPlayer::Mode::Abort);
    }
    else
    {
        playSound("gameOver.ogg", ssvs::SoundPlayer::Mode::Abort);
    }

    runLuaFunctionIfExists<void>("onDeath");

    status.flashEffect = 255;

    if(window != nullptr)
    {
        SSVOH_ASSERT(overlayCamera.has_value());
        SSVOH_ASSERT(backgroundCamera.has_value());

        overlayCamera->setView(
            {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
                sf::Vector2f(Config::getWidth(), Config::getHeight())});

        backgroundCamera->setCenter(ssvs::zeroVec2f);

        Utils::shakeCamera(effectTimelineManager, *overlayCamera);
        Utils::shakeCamera(effectTimelineManager, *backgroundCamera);
    }

    status.hasDied = true;
    stopLevelMusic();

    if(inReplay())
    {
        // Do not save scores or update rich presence if watching a replay.
        return;
    }

    // Gather player's Personal Best
    std::string pbStr = "(";
    if(isPersonalBest)
    {
        pbStr += "New PB!)";
    }
    else
    {
        pbStr += "PB: " +
                 timeFormat(assets.getLocalScore(
                     Utils::getLevelValidator(levelData->id, difficultyMult))) +
                 "s)";
    }

    std::string nameStr = levelData->name;
    nameFormat(nameStr);

    const std::string diffStr = diffFormat(difficultyMult);
    const std::string timeStr = timeFormat(status.getTimeSeconds());

    if(discordManager != nullptr)
    {
        discordManager->set_rich_presence_in_game(
            nameStr + " [x" + diffStr + "]", "Survived " + timeStr + "s", true);
    }

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

void HexagonGame::executeGameUntilDeath()
{
    while(!status.hasDied)
    {
        update(Config::TIME_STEP);
        postUpdate();
    }

    std::cout << "Player died.\nFinal time: " << status.getTimeSeconds()
              << '\n';
}

void HexagonGame::incrementDifficulty()
{
    playSound("levelUp.ogg");

    const float signMult = (levelStatus.rotationSpeed > 0.f) ? 1.f : -1.f;

    levelStatus.rotationSpeed += levelStatus.rotationSpeedInc * signMult;

    const auto& rotationSpeedMax(levelStatus.rotationSpeedMax);
    if(std::abs(levelStatus.rotationSpeed) > rotationSpeedMax)
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

    playSound(levelStatus.levelUpSound);
    runLuaFunctionIfExists<void>("onIncrement");
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
            << Config::getUneligibilityReason() << '\n';

        return CheckSaveScoreResult::Ineligible;
    }

    if(status.scoreInvalid)
    {
        ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
            << "Not saving score - score invalidated\n";

        return CheckSaveScoreResult::Invalid;
    }

    // Local score
    {
        std::string localValidator{
            Utils::getLevelValidator(levelData->id, difficultyMult)};

        if(assets.getLocalScore(localValidator) < score)
        {
            assets.setLocalScore(localValidator, score);
            assets.saveCurrentLocalProfile();

            return CheckSaveScoreResult::Local_NewBest;
        }

        return CheckSaveScoreResult::Local_NoNewBest;
    }

    SSVOH_ASSERT(false);
    return CheckSaveScoreResult::Local_NoNewBest;
}

void HexagonGame::goToMenu(bool mSendScores, bool mError)
{
    if(window == nullptr)
    {
        ssvu::lo("hg::HexagonGame::goToMenu")
            << "Attempted to go back to menu without a game window\n";

        return;
    }

    assets.stopSounds();

    ilcLuaTracked.clear();
    ilcLuaTrackedNames.clear();
    ilcLuaTrackedResults.clear();

    if(!mError)
    {
        playSound("beep.ogg");
    }

    calledDeprecatedFunctions.clear();

    if(mSendScores && !status.hasDied && !mError && !inReplay())
    {
        checkAndSaveScore();
    }

    // Stop infinite feedback from occurring if the error is happening on
    // onUnload.
    if(!mError)
    {
        runLuaFunctionIfExists<void>("onUnload");
    }

    SSVOH_ASSERT(mgPtr != nullptr);
    window->setGameState(mgPtr->getGame());
    mgPtr->returnToLevelSelection();
    mgPtr->init(mError);
}

void HexagonGame::raiseWarning(
    const std::string& mFunctionName, const std::string& mAdditionalInfo)
{
    // Only raise the warning once to avoid redundancy
    if(calledDeprecatedFunctions.contains(mFunctionName))
    {
        return;
    }

    calledDeprecatedFunctions.emplace(mFunctionName);

    // Raise warning to the console
    const std::string errorMsg = Utils::concat("[Lua] WARNING: The function \"",
        mFunctionName, "\" (used in level \"", levelData->name,
        "\") is deprecated. ", mAdditionalInfo);

    std::cout << errorMsg << std::endl;
    ilcCmdLog.emplace_back(Utils::concat("[warning]: ", errorMsg, '\n'));
}

void HexagonGame::addMessage(
    std::string mMessage, double mDuration, bool mSoundToggle)
{
    Utils::uppercasify(mMessage);

    messageTimeline.append_do([this, mSoundToggle, mMessage] {
        if(mSoundToggle)
        {
            playSound(levelStatus.beepSound);
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

[[nodiscard]] const PackData& HexagonGame::getPackData() const noexcept
{
    return assets.getPackData(getPackId());
}

[[nodiscard]] const std::string&
HexagonGame::getPackDisambiguator() const noexcept
{
    return getPackData().disambiguator;
}

[[nodiscard]] const std::string& HexagonGame::getPackAuthor() const noexcept
{
    return getPackData().author;
}

[[nodiscard]] const std::string& HexagonGame::getPackName() const noexcept
{
    return getPackData().name;
}

[[nodiscard]] int HexagonGame::getPackVersion() const noexcept
{
    return getPackData().version;
}

void HexagonGame::playLevelMusic()
{
    if(window == nullptr)
    {
        return;
    }

    if(!Config::getNoMusic())
    {
        const MusicData::Segment segment =
            musicData.playRandomSegment(getPackId(), assets);
        status.beatPulseDelay += segment.beatPulseDelayOffset;
    }
}

void HexagonGame::playLevelMusicAtTime(float mSeconds)
{
    if(window == nullptr)
    {
        return;
    }

    if(!Config::getNoMusic())
    {
        musicData.playSeconds(getPackId(), assets, mSeconds);
    }
}

void HexagonGame::stopLevelMusic()
{
    if(window == nullptr)
    {
        return;
    }

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
        return sf::Color(255, 255, 255, styleData.getMainColor().a);
    }

    return styleData.getMainColor();
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
    playSound(levelStatus.beepSound);

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

[[nodiscard]] float HexagonGame::getPlayerSpeedMult() const
{
    return levelStatus.playerSpeedMult;
}

[[nodiscard]] bool HexagonGame::getInputSwap() const
{
    return inputSwap;
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

void HexagonGame::performPlayerSwap(const bool mPlaySound)
{
    player.playerSwap();
    runLuaFunctionIfExists<void>("onCursorSwap");

    if(mPlaySound)
    {
        playSound(getLevelStatus().swapSound);
    }
}

void HexagonGame::performPlayerKill()
{
    const bool fatal =
        !Config::getInvincible() && !getLevelStatus().tutorialMode;

    player.kill(fatal);
    death();
}

} // namespace hg
