// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/HexagonClient.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/RandomNumberGenerator.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/LevelStatus.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/GameSystem/GameState.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Imgui.hpp"
#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"
#include "SSVOpenHexagon/Input/Utils.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/LevelValidator.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "pcg/pcg_extras.hpp"
#include "pcg/pcg_random.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <algorithm>
#include <filesystem>
#include <random>

#include <cctype>
#include <cmath>
#include <cstdio>

namespace hg
{

namespace
{

static void setVisualCharacterSize(sf::Text& text, const float characterSize)
{
    const float baseSize = static_cast<float>(text.getCharacterSize());
    const float scale    = characterSize / baseSize;
    text.scale           = {scale, scale};
}

[[nodiscard]] double getReplayScore(const HexagonGameStatus& status)
{
    return status.getCustomScore() != 0.f ? status.getCustomScore() : status.getPlayedAccumulatedFrametime();
}

[[nodiscard]] sf::base::UniquePtr<random_number_generator> initializeRng()
{
    thread_local pcg32_fast seed_rng = []
    {
        pcg_extras::seed_seq_from<std::random_device> seed_source;
        return pcg32_fast{seed_source};
    }();

    return sf::base::makeUnique<random_number_generator>(seed_rng());
}

} // namespace

HexagonGame::ActiveReplay::ActiveReplay(const replay_file& mReplayFile) :
    replayFile{mReplayFile},
    replayPlayer{replayFile._data}
{
}

void HexagonGame::createWall(int mSide, float mThickness, const SpeedData& mSpeed, const SpeedData& mCurve, float mHueMod)
{
    walls.emplaceBack(getSides(),
                      getWallAngleLeft(),
                      getWallAngleRight(),
                      getWallSkewLeft(),
                      getWallSkewRight(),
                      centerPos,
                      mSide,
                      mThickness,
                      levelStatus.wallSpawnDistance,
                      mSpeed,
                      mCurve,
                      mHueMod);
}

void HexagonGame::setMustStart(const bool x)
{
    mustStart = x;
}

static sf::Texture& getTextureOrNullTexture(HGAssets&                        assets,
                                            sf::base::Optional<sf::Texture>& nullTexture,
                                            const sf::base::String&          mId)
{
    if (!assets.hasTexture(mId))
    {
        SSVOH_ASSERT(nullTexture.hasValue());
        return *nullTexture;
    }

    return assets.getTexture(mId);
}

void HexagonGame::initKeyIcons()
{
    if (window == nullptr)
    {
        return;
    }

    for (const auto& t : {"keyArrow.png", "keyFocus.png", "keySwap.png", "replayIcon.png"})
    {
        getTextureOrNullTexture(assets, nullTexture, t).setSmooth(true);
    }

    updateKeyIcons();
}

void HexagonGame::updateKeyIcons()
{
    if (window == nullptr)
    {
        return;
    }

    constexpr float halfSize = 32.f;
    constexpr float size     = halfSize * 2.f;

    keyIconLeft.origin  = {halfSize, halfSize};
    keyIconRight.origin = {halfSize, halfSize};
    keyIconFocus.origin = {halfSize, halfSize};
    keyIconSwap.origin  = {halfSize, halfSize};

    keyIconLeft.rotation = sf::degrees(180);

    const float scaling = Config::getKeyIconsScale() / Config::getZoomFactor();

    keyIconLeft.scale  = {scaling, scaling};
    keyIconRight.scale = {scaling, scaling};
    keyIconFocus.scale = {scaling, scaling};
    keyIconSwap.scale  = {scaling, scaling};

    const float     scaledHalfSize = halfSize * scaling;
    const float     scaledSize     = size * scaling;
    const float     padding        = 8.f * scaling;
    const float     finalPadding   = scaledSize + padding;
    const sf::Vec2f finalPaddingX{finalPadding, 0.f};

    const sf::Vec2f bottomRight{Config::getWidth() - padding - scaledHalfSize,
                                Config::getHeight() - padding - scaledHalfSize};

    keyIconSwap.position = bottomRight;
    keyIconFocus.position -= finalPaddingX;
    keyIconRight.position -= finalPaddingX;
    keyIconLeft.position -= finalPaddingX;

    // ------------------------------------------------------------------------

    replayIcon.origin = {size, size};
    replayIcon.scale  = {scaling / 2.f, scaling / 2.f};

    const sf::Vec2f topRight{Config::getWidth() - padding - scaledHalfSize, padding + scaledHalfSize};

    replayIcon.position = topRight;
}

void HexagonGame::updateLevelInfo()
{
    if (window == nullptr)
    {
        return;
    }

    const float levelInfoScaling = 1.f;
    const float scaling          = levelInfoScaling / Config::getZoomFactor();
    const float padding          = 8.f * scaling;

    const sf::Vec2f size{325.f, 75.f};
    const sf::Vec2f halfSize{size / 2.f};
    const sf::Vec2f scaledHalfSize{halfSize * scaling};

    levelInfoRectangle.setSize(size);
    levelInfoRectangle.scale = {scaling, scaling};

    const sf::Color offsetColor{
        Config::getBlackAndWhite() || styleData.getColors().empty() ? sf::Color::Black : styleData.getColor(0)};

    levelInfoRectangle.setFillColor(offsetColor);
    levelInfoRectangle.setOutlineColor(styleData.getMainColor());
    levelInfoRectangle.origin = halfSize;
    levelInfoRectangle.setOutlineThickness(3.f);

    const sf::Vec2f bottomLeft{padding + scaledHalfSize.x, Config::getHeight() - padding - scaledHalfSize.y};

    levelInfoRectangle.position = bottomLeft;

    const float tPadding = padding;

    const auto trim = [](sf::base::String s)
    {
        if (s.size() > 26)
        {
            return sf::base::String(s.toStringView().substrByPosLen(0, 26));
        }

        return s;
    };

    if (textUI.hasValue())
    {
        textUI->levelInfoTextLevel.setFillColor(getColorText());
        setVisualCharacterSize(textUI->levelInfoTextLevel, 20.f / Config::getZoomFactor());
        textUI->levelInfoTextLevel.setString(trim(Utils::toUppercase(levelData->name)));
        textUI->levelInfoTextLevel.origin   = textUI->levelInfoTextLevel.getLocalTopLeft();
        textUI->levelInfoTextLevel.position = levelInfoRectangle.getGlobalTopLeft() + sf::Vec2f{tPadding, tPadding};

        const auto prepareText = [&](sf::Text& text, const float characterSize, const sf::base::String& string)
        {
            text.setFillColor(getColorText());
            setVisualCharacterSize(text, characterSize / Config::getZoomFactor());
            text.setString(string);
        };

        prepareText(textUI->levelInfoTextPack, 14.f, trim(Utils::toUppercase(getPackName())));
        textUI->levelInfoTextPack.origin = textUI->levelInfoTextPack.getLocalTopLeft();
        textUI->levelInfoTextPack.position = textUI->levelInfoTextLevel.getGlobalBottomLeft() + sf::Vec2f{0.f, tPadding};

        SSVOH_ASSERT(levelData != nullptr);

        prepareText(textUI->levelInfoTextAuthor, 20.f, trim(Utils::toUppercase(levelData->author)));
        textUI->levelInfoTextAuthor.origin = textUI->levelInfoTextAuthor.getLocalBottomRight();
        textUI->levelInfoTextAuthor.position = levelInfoRectangle.getGlobalBottomRight() - sf::Vec2f{tPadding, tPadding};

        prepareText(textUI->levelInfoTextBy, 12.f, "BY");
        textUI->levelInfoTextBy.origin   = textUI->levelInfoTextBy.getLocalBottomRight();
        textUI->levelInfoTextBy.position = textUI->levelInfoTextAuthor.getGlobalBottomLeft() - sf::Vec2f{tPadding, 0.f};

        if (levelData->difficultyMults.size() > 1)
        {
            prepareText(textUI->levelInfoTextDM, 14.f, diffFormat(difficultyMult) + "x");
            textUI->levelInfoTextDM.origin = textUI->levelInfoTextDM.getLocalBottomLeft();
            textUI->levelInfoTextDM.position = levelInfoRectangle.getGlobalBottomLeft() + sf::Vec2f{tPadding, -tPadding};
        }
        else
        {
            textUI->levelInfoTextDM.setString("");
        }
    }
}

void HexagonGame::nameFormat(sf::base::String& name)
{
    name[0] = std::toupper(name[0]);
}

[[nodiscard]] sf::base::String HexagonGame::diffFormat(float diff)
{
    char buf[255];
    std::snprintf(buf, sizeof(buf), "%g", diff);
    return buf;
}

[[nodiscard]] sf::base::String HexagonGame::timeFormat(float time)
{
    char buf[255];
    std::snprintf(buf, sizeof(buf), "%.3f", time);
    return buf;
}

[[nodiscard]] bool HexagonGame::imguiLuaConsoleHasInput()
{
    return ilcShowConsole && (Imgui::wantCaptureKeyboard() || Imgui::wantCaptureMouse());
}

[[nodiscard]] static sf::Text initText(const sf::Font& font, const char* text, const float characterSize)
{
    return sf::Text{font,
                    {.string = text, .characterSize = static_cast<unsigned int>(characterSize / Config::getZoomFactor())}};
}

HexagonGame::TextUI::TextUI(HGAssets& mAssets) :
    font{mAssets.getFont("OpenSquare-Regular.ttf")},
    fontBold{mAssets.getFont("OpenSquare-Bold.ttf")},
    messageText{initText(font, "", 38.f)},
    pbText{initText(fontBold, "", 65.f)},
    levelInfoTextLevel{initText(font, "", 20.f)},
    levelInfoTextPack{initText(font, "", 14.f)},
    levelInfoTextAuthor{initText(font, "", 20.f)},
    levelInfoTextBy{initText(font, "", 12.f)},
    levelInfoTextDM{initText(font, "", 14.f)},
    fpsText{initText(font, "0", 25.f)},
    timeText{initText(fontBold, "0", 70.f)},
    text{initText(font, "", 25.f)},
    replayText{initText(font, "", 20.f)}
{
}

HexagonGame::HexagonGame(Steam::steam_manager*     mSteamManager,
                         Discord::discord_manager* mDiscordManager,
                         HGAssets&                 mAssets,
                         Audio*                    mAudio,
                         ssvs::GameWindow*         mGameWindow,
                         HexagonClient*            mHexagonClient) :
    nullTexture(mGameWindow != nullptr ? sf::Texture::create({1u, 1u}) : sf::base::nullOpt),
    steamManager(mSteamManager),
    discordManager(mDiscordManager),
    assets(mAssets),
    audio(mAudio),
    window(mGameWindow),
    hexagonClient{mHexagonClient},
    player{sf::Vec2f{0.f, 0.f},
           getSwapCooldown(),
           Config::getPlayerSize(),
           Config::getPlayerSpeed(),
           Config::getPlayerFocusSpeed()},
    levelStatus{Config::getMusicSpeedDMSync(), Config::getSpawnDistance()},
    txStarParticle{nullptr},
    txSmallCircle{nullptr},
    txKeyIconLeft{nullptr},
    txKeyIconRight{nullptr},
    txKeyIconFocus{nullptr},
    txKeyIconSwap{nullptr},
    txReplayIcon{nullptr},
    keyIconLeft{},
    keyIconRight{},
    keyIconFocus{},
    keyIconSwap{},
    replayIcon{},
    levelInfoRectangle{{}},
    rng{initializeRng()}
{
    if (!assets.isHeadless())
    {
        textUI.emplace(assets);
    }

    if (window != nullptr)
    {
        // Default the render target to the game window. Callers running an
        // off-screen preview (`previewMode = true`) override this with a
        // pointer to their `sf::RenderTexture`.
        renderTarget = &window->getRenderWindow();

        imguiCtx.emplace();

        const float width      = Config::getWidth();
        const float height     = Config::getHeight();
        const float zoomFactor = Config::getZoomFactor();

        backgroundCamera.emplace(
            sf::View{.center = sf::Vec2f{0.f, 0.f}, .size = sf::Vec2f{width * zoomFactor, height * zoomFactor}});

        overlayCamera.emplace(sf::View{.center = sf::Vec2f{width / 2.f, height / 2.f}, .size = sf::Vec2f{width, height}});

        txStarParticle = &getTextureOrNullTexture(assets, nullTexture, "starParticle.png");
        txSmallCircle  = &getTextureOrNullTexture(assets, nullTexture, "smallCircle.png");

        txKeyIconLeft  = &getTextureOrNullTexture(assets, nullTexture, "keyArrow.png");
        txKeyIconRight = &getTextureOrNullTexture(assets, nullTexture, "keyArrow.png");
        txKeyIconFocus = &getTextureOrNullTexture(assets, nullTexture, "keyFocus.png");
        txKeyIconSwap  = &getTextureOrNullTexture(assets, nullTexture, "keySwap.png");
        txReplayIcon   = &getTextureOrNullTexture(assets, nullTexture, "replayIcon.png");

        keyIconLeft.textureRect  = txKeyIconLeft->getRect();
        keyIconRight.textureRect = txKeyIconRight->getRect();
        keyIconFocus.textureRect = txKeyIconFocus->getRect();
        keyIconSwap.textureRect  = txKeyIconSwap->getRect();
        replayIcon.textureRect   = txReplayIcon->getRect();
    }

    game.onUpdate += [this](float mFT) { update(mFT, Config::getTimescale()); };

    game.onPostUpdate += [this] { postUpdate(); };

    game.onDraw += [this] { draw(); };

    game.onAnyEvent += [this](const sf::Event& e)
    {
        if (imguiCtx.hasValue())
            imguiCtx->processEvent(window->getRenderWindow(), e);
    };

    if (window != nullptr)
    {
        window->onRecreation += [this] { initKeyIcons(); };
    }

    // ------------------------------------------------------------------------
    // Keyboard binds

    Config::keyboardBindsSanityCheck();

    using Tid = Config::Tid;

    const auto addTidInput = [&](const Tid tid, const ssvs::Input::Type type, auto action)
    { game.addInput(Config::getTrigger(tid), action, type, static_cast<int>(tid)); };

    const auto addTid2StateInput = [&](const Tid tid, bool& value)
    { add2StateInput(game, Config::getTrigger(tid), value, static_cast<int>(tid)); };

    addTid2StateInput(Tid::RotateCCW, inputImplCCW);
    addTid2StateInput(Tid::RotateCW, inputImplCW);
    addTid2StateInput(Tid::Focus, inputFocused);
    addTid2StateInput(Tid::Swap, inputSwap);

    const auto notInConsole = [this](auto&& f)
    {
        return [this, f](float /*unused*/)
        {
            if (!imguiLuaConsoleHasInput())
            {
                f();
            }
        };
    };

    game.addInput({{sf::Keyboard::Key::Escape}},
                  notInConsole([this] { goToMenu(); }), // hardcoded
                  ssvs::Input::Type::Always);

    addTidInput(Tid::Exit, ssvs::Input::Type::Always, notInConsole([this] { goToMenu(); }));

    addTidInput(Tid::ForceRestart, ssvs::Input::Type::Once, notInConsole([this] {
        status.mustStateChange = StateChange::MustRestart;
    }));

    addTidInput(Tid::Restart,
                ssvs::Input::Type::Once,
                notInConsole([this]
    {
        if (deathInputIgnore <= 0.f && status.hasDied)
        {
            status.mustStateChange = StateChange::MustRestart;
        }
    }));

    addTidInput(Tid::Replay,
                ssvs::Input::Type::Once,
                notInConsole([this]
    {
        if ((deathInputIgnore <= 0.f && status.hasDied) || inReplay())
        {
            status.mustStateChange = StateChange::MustReplay;
        }
    }));

    addTidInput(Tid::Screenshot, ssvs::Input::Type::Once, notInConsole([this] { mustTakeScreenshot = true; }));

    addTidInput(Tid::LuaConsole,
                ssvs::Input::Type::Once,
                [this](float /*unused*/)
    {
        if (Config::getDebug())
        {
            ilcShowConsoleNext = true;
        }
    });

    addTidInput(Tid::Pause,
                ssvs::Input::Type::Once,
                [this](float /*unused*/)
    {
        if (Config::getDebug())
        {
            debugPause = !debugPause;

            if (debugPause)
            {
                if (shouldPlayMusic())
                {
                    audio->pauseMusic();
                }
            }
            else if (!status.hasDied)
            {
                if (shouldPlayMusic())
                {
                    audio->resumeMusic();
                }
            }
        }
    });

    // ------------------------------------------------------------------------
    // Joystick binds

    Config::loadAllJoystickBinds();

    // ------------------------------------------------------------------------
    // Key icons

    initKeyIcons();
}

HexagonGame::~HexagonGame()
{
    hg::lo("HexagonGame::~HexagonGame") << "Cleaning up game resources...\n";
}

void HexagonGame::refreshTrigger(const ssvs::Input::Trigger& trigger, const int bindID)
{
    game.refreshTrigger(trigger, bindID);
}

void HexagonGame::setLastReplay(const replay_file& mReplayFile)
{
    lastSeed        = mReplayFile._seed;
    lastReplayData  = mReplayFile._data;
    lastFirstPlay   = mReplayFile._first_play;
    lastPlayedScore = mReplayFile._played_score;

    activeReplay.emplace(mReplayFile);
}

void HexagonGame::updateRichPresenceCallbacks()
{
    // Update Steam Rich Presence
    if (steamManager != nullptr && !steamHung)
    {
        if (!steamManager->run_callbacks())
        {
            steamAttempt += 1;
            if (steamAttempt > 20)
            {
                steamHung = true;
                hg::lo("Steam") << "Too many failed callbacks. Stopping "
                                   "Steam callbacks.\n";
            }
        }
    }

    // Update Discord Rich Presence
    if (discordManager != nullptr && !discordHung)
    {
        if (!discordManager->run_callbacks())
        {
            discordAttempt += 1;
            if (discordAttempt > 20)
            {
                discordHung = true;
                hg::lo("Discord") << "Too many failed callbacks. Stopping "
                                     "Discord callbacks.\n";
            }
        }
    }
}

[[nodiscard]] bool HexagonGame::shouldPlaySounds() const
{
    return window != nullptr && audio != nullptr && !Config::getNoSound() && !previewMode;
}

[[nodiscard]] bool HexagonGame::shouldPlayMusic() const
{
    return window != nullptr && audio != nullptr && !Config::getNoMusic() && !previewMode;
}

void HexagonGame::playSoundOverride(const sf::base::String& mId)
{
    if (shouldPlaySounds())
    {
        audio->playSoundOverride(mId);
    }
}

void HexagonGame::playSoundAbort(const sf::base::String& mId)
{
    if (shouldPlaySounds())
    {
        audio->playSoundAbort(mId);
    }
}

void HexagonGame::playPackSoundOverride(const sf::base::String& mPackId, const sf::base::String& mId)
{
    if (shouldPlaySounds())
    {
        audio->playPackSoundOverride(mPackId, mId);
    }
}

void HexagonGame::saveReplay()
{
    if (shouldSaveScore() && !status.hasDied)
    {
        (void)death_saveScoreIfNeeded(); // Saves local best

        const replay_file rf = death_createReplayFile();

        hg::lo("Replay") << "Attempting to send and save replay...\n";
        death_sendAndSaveReplay(rf);
    }
}

void HexagonGame::newGame(const sf::base::String& mPackId,
                          const sf::base::String& mId,
                          bool                    mFirstPlay,
                          float                   mDifficultyMult,
                          bool                    executeLastReplay)
{
    // Save replay when restarting without having died
    if (!mFirstPlay)
    {
        saveReplay();
    }

    SSVOH_ASSERT(assets.isValidPackId(mPackId));
    SSVOH_ASSERT(assets.isValidLevelId(mId));

    packId  = mPackId;
    levelId = mId;

    if (executeLastReplay && activeReplay.hasValue())
    {
        firstPlay = activeReplay->replayFile._first_play;
    }
    else
    {
        firstPlay = mFirstPlay;
    }

    SSVOH_ASSERT(assets.isValidLevelId(mId));
    setLevelData(assets.getLevelData(mId), mFirstPlay);

    difficultyMult = mDifficultyMult;

    const double tempReplayScore = getReplayScore(status);
    status                       = HexagonGameStatus{};

    if (!executeLastReplay)
    {
        rng = initializeRng();

        // Save data for immediate replay.
        lastSeed       = rng->seed();
        lastReplayData = replay_data{};
        lastFirstPlay  = mFirstPlay;

        // Clear any existing active replay.
        activeReplay.reset();
    }
    else
    {
        if (!activeReplay.hasValue())
        {
            lastPlayedScore = tempReplayScore;

            activeReplay.emplace(replay_file{
                ._version{0},

                // TODO (P1): should this stay local?
                ._player_name{assets.getCurrentLocalProfile().getName()},

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

        SSVOH_ASSERT(assets.isValidPackId(mPackId));

        activeReplay->replayPackName = Utils::toUppercase(assets.getPackData(mPackId).name);

        activeReplay->replayLevelName = Utils::toUppercase(levelData->name);

        rng       = sf::base::makeUnique<random_number_generator>(activeReplay->replayFile._seed);
        firstPlay = activeReplay->replayFile._first_play;
    }

    // Audio cleanup
    if (window != nullptr && audio != nullptr && !previewMode)
    {
        // Audio is silenced when previewing -- the menu doesn't want a
        // level's music to fight with whatever the menu itself plays.
        audio->stopSounds();
        stopLevelMusic();

        if (!Config::getNoMusic())
        {
            playLevelMusic();
            audio->pauseMusic();
            refreshMusicPitch();
        }
        else
        {
            audio->stopMusic();
        }
    }

    debugPause = false;

    // Events cleanup
    if (textUI.hasValue())
    {
        textUI->messageText.setString("");
        textUI->pbText.setString("");
    }

    // Event timeline cleanup
    eventTimeline.clear();
    eventTimelineRunner = {};

    // Message timeline cleanup
    messageTimeline.clear();
    messageTimelineRunner = {};

    // Custom timeline manager cleanup
    _customTimelineManager.clear();

    // Manager cleanup
    walls.clear();
    cwManager.clear();
    player = CPlayer{sf::Vec2f{0.f, 0.f},
                     getSwapCooldown(),
                     Config::getPlayerSize(),
                     Config::getPlayerSpeed(),
                     Config::getPlayerFocusSpeed()};

    // Timeline cleanup
    timeline.clear();
    timelineRunner = {};

    mustChangeSides = false;
    mustStart       = false;

    // Particles cleanup
    pbTextGrowth         = 0.f;
    mustSpawnPBParticles = false;
    swapParticlesSpawnInfo.reset();
    nextPBParticleSpawn = 0.f;
    particles.clear();
    trailParticles.clear();
    swapParticles.clear();

    // Re-init default flash effect
    initFlashEffect(255, 255, 255);

    if (window != nullptr)
    {
        SSVOH_ASSERT(overlayCamera.hasValue());
        SSVOH_ASSERT(backgroundCamera.hasValue());

        // Use the actual render target's aspect ratio for the camera. For
        // normal gameplay this is the window (matches Config dimensions);
        // for preview-mode HG instances `renderTarget` is a fixed-size
        // off-screen texture, so the hexagon visuals don't get stretched
        // when the user runs the game in a non-16:9 window.
        const sf::Vec2f targetSize = (previewMode && renderTarget != nullptr)
                                         ? renderTarget->getSize().to<sf::Vec2f>()
                                         : sf::Vec2f{static_cast<float>(Config::getWidth()),
                                                     static_cast<float>(Config::getHeight())};

        // Reset zoom
        *overlayCamera = sf::View{.center = targetSize * 0.5f, .size = targetSize};

        *backgroundCamera = sf::View{.center = sf::Vec2f{0.f, 0.f}, .size = targetSize * Config::getZoomFactor()};

        backgroundCamera->rotation = sf::degrees(0.f);

        // Reset skew
        overlayCameraTransform.skew    = sf::Vec2f{1.f, 1.f};
        backgroundCameraTransform.skew = sf::Vec2f{1.f, 1.f};
    }

    // Lua context and game status cleanup
    inputImplCCW = inputImplCW = false;
    playerNowReadyToSwap       = false;

    if (!firstPlay)
        runVoidLuaFunctionIfExists("onPreUnload");
    lua = Lua::LuaContext{};
    calledDeprecatedFunctions.clear();
    initLua();
    runLuaFile(levelData->luaScriptPath);

    if (!firstPlay)
    {
        runVoidLuaFunctionIfExists("onUnload");
        playSoundOverride("restart.ogg");
    }
    else
    {
        playSoundOverride("select.ogg");
    }

    runVoidLuaFunctionIfExists("onInit");

    restartId        = mId;
    restartFirstTime = false;
    setSides(levelStatus.sides);

    // Set initial values for some status fields from Lua
    status.pulseDelay += levelStatus.pulseInitialDelay;
    status.beatPulseDelay += levelStatus.beatPulseInitialDelay;
    timeUntilRichPresenceUpdate = -1.f; // immediate update

    // Prepare text for input hints on restart/replay
    if (window != nullptr)
    {
        // Store the keys/buttons to be pressed to replay and restart after you
        // die.
        using Tid           = Config::Tid;
        status.restartInput = Config::getKeyboardBindNames(Tid::Restart);
        status.replayInput  = Config::getKeyboardBindNames(Tid::Replay);

        // Format strings to only show the first key to avoid extremely long
        // messages
        int commaPos = status.restartInput.toStringView().find(',');
        if (commaPos > 0)
        {
            status.restartInput.erase(commaPos);
        }
        commaPos = status.replayInput.toStringView().find(',');
        if (commaPos > 0)
        {
            status.replayInput.erase(commaPos);
        }

        // Add joystick buttons if any and finalize message
        sf::base::String joystickButton = Config::getJoystickBindName(Joystick::Jid::Restart);
        if (!status.restartInput.empty())
        {
            if (!joystickButton.empty())
            {
                status.restartInput += " OR JOYSTICK " + joystickButton;
            }
            status.restartInput = "PRESS " + status.restartInput + " TO RESTART\n";
        }
        else if (!joystickButton.empty())
        {
            status.restartInput = "PRESS JOYSTICK " + joystickButton + " TO RESTART\n";
        }
        else
        {
            status.restartInput = "NO RESTART BUTTON SET\n";
        }

        joystickButton = Config::getJoystickBindName(Joystick::Jid::Replay);
        if (!status.replayInput.empty())
        {
            if (!joystickButton.empty())
            {
                status.replayInput += " OR JOYSTICK " + joystickButton;
            }
            status.replayInput = "PRESS " + status.replayInput + " TO REPLAY\n";
        }
        else if (!joystickButton.empty())
        {
            status.replayInput = "PRESS JOYSTICK " + joystickButton + " TO REPLAY\n";
        }
        else
        {
            status.replayInput = "NO REPLAY BUTTON SET\n";
        }
    }
}

void HexagonGame::death_shakeCamera()
{
    if (window == nullptr)
    {
        return;
    }

    SSVOH_ASSERT(overlayCamera.hasValue());
    SSVOH_ASSERT(backgroundCamera.hasValue());

    *overlayCamera = sf::View{.center = {Config::getWidth() / 2.f, Config::getHeight() / 2.f},
                              .size   = sf::Vec2f(Config::getWidth(), Config::getHeight())};

    backgroundCamera->center = sf::Vec2f{0.f, 0.f};

    status.cameraShake = 45.f * Config::getCameraShakeMultiplier();
}

void HexagonGame::death_flashEffect()
{
    initFlashEffect(255, 255, 255);
    status.flashEffect = 255;
}

void HexagonGame::death_updateRichPresence()
{
    if (window == nullptr)
    {
        return;
    }

    if (inReplay())
    {
        // Do not update rich presence if watching a replay.
        return;
    }

// TODO (P2): ??? meant for rich presence?
#if 0
    // Gather player's Personal Best
    sf::base::String pbStr = "(";
    if(isPersonalBest)
    {
        pbStr += "New PB!)";
    }
    else
    {
        pbStr += "PB: " +
                 timeFormat(assets.getLocalScore(levelData->getValidatorWithoutPackId(difficultyMult))) +
                 "s)";
    }
#endif

    sf::base::String nameStr = levelData->name;
    nameFormat(nameStr);

    const sf::base::String diffStr = diffFormat(difficultyMult);
    const sf::base::String timeStr = timeFormat(status.getTimeSeconds());

    if (discordManager != nullptr)
    {
        discordManager->set_rich_presence_in_game(nameStr + " [x" + diffStr + "]", "Survived " + timeStr + "s", true);
    }
}

[[nodiscard]] HexagonGame::SaveScoreIfNeededResult HexagonGame::death_saveScoreIfNeeded()
{
    if (window == nullptr)
    {
        return SaveScoreIfNeededResult::NoWindow;
    }

    if (!shouldSaveScore())
    {
        return SaveScoreIfNeededResult::ShouldNotSave;
    }

    const sf::base::String validatorWithoutPackid = levelData->getValidatorWithoutPackId(difficultyMult);

    const double score = status.getTimeSeconds();

    const bool isPersonalBest = score > assets.getLocalScore(validatorWithoutPackid);

    if (!isPersonalBest)
    {
        return SaveScoreIfNeededResult::NotPersonalBest;
    }

    assets.setLocalScore(validatorWithoutPackid, score);
    return SaveScoreIfNeededResult::PersonalBest;
}

void HexagonGame::death_saveScoreIfNeededAndShowPBEffects()
{
    const SaveScoreIfNeededResult r = death_saveScoreIfNeeded();

    if (r == SaveScoreIfNeededResult::NoWindow)
    {
        return;
    }

    if (r == SaveScoreIfNeededResult::ShouldNotSave || r == SaveScoreIfNeededResult::NotPersonalBest)
    {
        playSoundAbort("gameOver.ogg");
        return;
    }

    SSVOH_ASSERT(r == SaveScoreIfNeededResult::PersonalBest);

    if (textUI.hasValue())
    {
        textUI->pbText.setString("NEW PERSONAL BEST!");
    }

    mustSpawnPBParticles = true;

    playSoundAbort("personalBest.ogg");
}

void HexagonGame::death(bool mForce)
{
    // Preview instances must never enter the death/score-save cascade --
    // belt-and-suspenders alongside `performPlayerKill`'s early-return,
    // since Lua / timeline can call `death()` directly.
    if (previewMode)
    {
        return;
    }

    if (status.hasDied)
    {
        return;
    }

    deathInputIgnore = 10.f;

    playSoundAbort(levelStatus.deathSound);

    runVoidLuaFunctionIfExists("onPreDeath");

    if (!mForce && (Config::getInvincible() || levelStatus.tutorialMode))
    {
        return;
    }

    runVoidLuaFunctionIfExists("onDeath");
    death_shakeCamera();
    death_updateRichPresence();
    stopLevelMusic();

    death_flashEffect();

    status.hasDied = true;

    if (!inReplay())
    {
        const replay_file rf = death_createReplayFile();

        // TODO (P2): for testing
        if (onDeathReplayCreated)
        {
            onDeathReplayCreated(rf);
        }

        hg::lo("Replay") << "Attempting to send and save replay...\n";
        death_sendAndSaveReplay(rf);
    }

    death_saveScoreIfNeededAndShowPBEffects(); // Saves local best

    if (window != nullptr && Config::getAutoRestart())
    {
        status.mustStateChange = StateChange::MustRestart;
    }
}

[[nodiscard]] replay_file HexagonGame::death_createReplayFile()
{
    // TODO (P2): for testing
    const sf::base::String rfName = assets.anyLocalProfileActive() ? assets.getCurrentLocalProfile().getName() : "no_profile";

    return replay_file{
        ._version{0},
        ._player_name{rfName},
        ._seed{lastSeed},
        ._data{lastReplayData},
        ._pack_id{packId},
        ._level_id{levelId},
        ._first_play{firstPlay},
        ._difficulty_mult{difficultyMult},
        ._played_score{getReplayScore(status)},
    };
}

void HexagonGame::death_sendAndSaveReplay(const replay_file& rf)
{
    const sf::base::Optional<compressed_replay_file> crfOpt = compress_replay_file(rf);

    if (!crfOpt.hasValue())
    {
        hg::lo("Replay") << "Failed to compress replay, will not save to "
                            "file or send to server\n";

        return;
    }

    const compressed_replay_file& crf = crfOpt.value();

    // ------------------------------------------------------------------------
    // Send compressed replay to server.
    const auto lv = Utils::getLevelValidator(rf._level_id, rf._difficulty_mult); // TODO
    if (const sf::base::String levelValidator{lv.data(), lv.size()}; !death_sendReplay(levelValidator, crf))
    {
        hg::lo("Replay") << "Failure sending replay\n";
    }

    // ------------------------------------------------------------------------
    // Save compressed replay locally.
    if (const sf::base::String filename = Utils::concat(rf.create_filename(), ".z"); !death_saveReplay(filename, crf))
    {
        hg::lo("Replay") << "Failure saving replay\n";
    }
}

[[nodiscard]] bool HexagonGame::death_sendReplay(const sf::base::String& levelValidator, const compressed_replay_file& crf)
{
    if (hexagonClient == nullptr || hexagonClient->getState() != HexagonClient::State::LoggedIn_Ready ||
        !Config::getOfficial())
    {
        return false;
    }

    hg::lo("Replay") << "Sending compressed replay to server...\n";

    if (!hexagonClient->trySendCompressedReplay(levelValidator, crf))
    {
        hg::lo("Replay") << "Could not send compressed replay to server\n";
        return false;
    }

    steamManager->unlock_achievement("a24_onlinescore");
    return true;
}

[[nodiscard]] bool HexagonGame::death_saveReplay(sf::base::String filename, const compressed_replay_file& crf)
{
    sf::base::String dirPath = "Replays/" + levelId + "/" + diffFormat(difficultyMult) + "x/";

    // Replace invalid characters for Windows file paths.
    for (const char c : {':', '*', '?', '"', '<', '>', '|'})
    {
        std::replace(dirPath.begin(), dirPath.end(), c, '_');
        std::replace(filename.begin(), filename.end(), c, '_');
    }

    std::filesystem::create_directories(dirPath.cStr());
    sf::Path p;
    p /= dirPath.cStr();
    p /= filename.cStr();

    if (!crf.serialize_to_file(p))
    {
        hg::lo("Replay") << "Failed to save new compressed replay file '" << p << "'\n";

        return false;
    }

    hg::lo("Replay") << "Successfully saved new compressed replay file '" << p << "'\n";

    return true;
}

[[nodiscard]] sf::base::Optional<HexagonGame::GameExecutionResult> HexagonGame::executeGameUntilDeath(
    const int   maxProcessingSeconds,
    const float timescale)
{
    const HRTimePoint tpBegin = HRClock::now();

    const auto exceededProcessingTime = [&] { return hrSecondsSince(tpBegin) > maxProcessingSeconds; };

    while (!status.hasDied)
    {
        update(Config::TIME_STEP, timescale);
        postUpdate();

        if (exceededProcessingTime())
        {
            return sf::base::nullOpt;
        }
    }

    return sf::base::makeOptional(GameExecutionResult{
        .playedTimeSeconds = status.getPlayedAccumulatedFrametimeInSeconds(), //
        .pausedTimeSeconds = status.getPausedAccumulatedFrametimeInSeconds(), //
        .totalTimeSeconds  = status.getTotalAccumulatedFrametimeInSeconds(),  //
        .customScore       = status.getCustomScore()                          //
    });
}

[[nodiscard]] sf::base::Optional<HexagonGame::GameExecutionResult> HexagonGame::runReplayUntilDeathAndGetScore(
    const replay_file& mReplayFile,
    const int          maxProcessingSeconds,
    const float        timescale)
{
    SSVOH_ASSERT(assets.isValidPackId(mReplayFile._pack_id));
    SSVOH_ASSERT(assets.isValidLevelId(mReplayFile._level_id));

    setLastReplay(mReplayFile);

    newGame(mReplayFile._pack_id,
            mReplayFile._level_id,
            mReplayFile._first_play,
            mReplayFile._difficulty_mult,
            /* mExecuteLastReplay */ true);

    return executeGameUntilDeath(maxProcessingSeconds, timescale);
}

void HexagonGame::incrementDifficulty()
{
    playSoundOverride("levelUp.ogg");

    const float signMult = (levelStatus.rotationSpeed > 0.f) ? 1.f : -1.f;

    levelStatus.rotationSpeed += levelStatus.rotationSpeedInc * signMult;

    const auto& rotationSpeedMax(levelStatus.rotationSpeedMax);
    if (std::abs(levelStatus.rotationSpeed) > rotationSpeedMax)
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

    if (levelStatus.rndSideChangesEnabled)
    {
        setSides(mSideNumber);
    }

    mustChangeSides = false;

    playSoundOverride(levelStatus.levelUpSound);
    runVoidLuaFunctionIfExists("onIncrement");
}

[[nodiscard]] bool HexagonGame::shouldSaveScore()
{
    if (!assets.anyLocalProfileActive())
    {
        hg::lo("hg::HexagonGame::shouldSaveScore()") << "No local profile active, rejecting\n";

        return false;
    }

    if (!Config::isEligibleForScore())
    {
        hg::lo("hg::HexagonGame::shouldSaveScore()")
            << "Not saving score - not eligible - " << Config::getUneligibilityReason() << '\n';

        return false;
    }

    if (status.scoreInvalid)
    {
        hg::lo("hg::HexagonGame::shouldSaveScore()") << "Not saving score - score invalidated\n";

        return false;
    }

    if (levelStatus.tutorialMode)
    {
        hg::lo("hg::HexagonGame::shouldSaveScore()") << "Not saving score - in tutorial mode\n";

        return false;
    }

    if (levelData->unscored)
    {
        hg::lo("hg::HexagonGame::shouldSaveScore()") << "Not saving score - unscored level\n";

        return false;
    }

    if (inReplay())
    {
        hg::lo("hg::HexagonGame::shouldSaveScore()") << "Not saving score - currently in replay\n";

        return false;
    }

    return true;
}

#if 0
// TODO (P0): use?
const double score =
    levelStatus.scoreOverridden
        ? lua.readVariable<double>(levelStatus.scoreOverride)
        : status.getTimeSeconds();
#endif

void HexagonGame::goToMenu(bool mSendScores, bool mError)
{
    if (window == nullptr)
    {
        hg::lo("hg::HexagonGame::goToMenu") << "Attempted to go back to menu without a game window\n";

        return;
    }

    if (audio != nullptr)
    {
        audio->stopSounds();
    }

    ilcLuaTracked.clear();
    ilcLuaTrackedNames.clear();
    ilcLuaTrackedResults.clear();

    // Note: the menu's `init()` plays its own `select.ogg` to signal the
    // user has returned to the menu, so we don't play anything here for
    // the success case -- playing both sounds back-to-back was audible as
    // a "double beep" on ESC.

    calledDeprecatedFunctions.clear();

    if (mSendScores && !mError)
    {
        saveReplay();
    }

    // Stop infinite feedback from occurring if the error is happening on
    // onUnload.
    if (!mError)
    {
        runVoidLuaFunctionIfExists("onUnload");
    }

    if (fnGoToMenu)
    {
        fnGoToMenu(mError);
    }
}

void HexagonGame::raiseWarning(const sf::base::String& mFunctionName, const sf::base::String& mAdditionalInfo)
{
    // Only raise the warning once to avoid redundancy
    if (calledDeprecatedFunctions.contains(mFunctionName))
    {
        return;
    }

    calledDeprecatedFunctions.emplace(mFunctionName);

    // Raise warning to the console
    const sf::base::String errorMsg = Utils::concat("[Lua] WARNING: The function \"",
                                                    mFunctionName,
                                                    "\" (used in level \"",
                                                    levelData->name,
                                                    "\") is deprecated. ",
                                                    mAdditionalInfo);

    sf::cOut() << errorMsg << sf::endL;
    ilcCmdLog.emplaceBack(Utils::concat("[warning]: ", errorMsg, '\n'));
}

void HexagonGame::addMessage(sf::base::String mMessage, double mDuration, bool mSoundToggle)
{
    if (!Config::getShowMessages())
    {
        return;
    }

    Utils::uppercasify(mMessage);

    messageTimeline.append_do([this, mSoundToggle, mMessage]
    {
        if (mSoundToggle)
        {
            playSoundOverride(levelStatus.beepSound);
        }

        if (textUI.hasValue())
        {
            textUI->messageText.setString(mMessage);
        }
    });

    messageTimeline.append_wait_for_sixths(mDuration);
    messageTimeline.append_do([this]
    {
        if (textUI.hasValue())
        {
            textUI->messageText.setString("");
        }
    });
}

void HexagonGame::clearMessages()
{
    messageTimeline.clear();
}

void HexagonGame::setLevelData(const LevelData& mLevelData, bool mMusicFirstPlay)
{
    levelData = &mLevelData;

    levelStatus = LevelStatus{Config::getMusicSpeedDMSync(), Config::getSpawnDistance()};

    styleData = assets.getStyleData(levelData->packId, levelData->styleId);
    styleData.computeColors();

    musicData           = assets.getMusicData(levelData->packId, levelData->musicId);
    musicData.firstPlay = mMusicFirstPlay;
}

[[nodiscard]] const sf::base::String& HexagonGame::getPackId() const noexcept
{
    return levelData->packId;
}

[[nodiscard]] const PackData& HexagonGame::getPackData() const noexcept
{
    return assets.getPackData(getPackId());
}

[[nodiscard]] const sf::base::String& HexagonGame::getPackDisambiguator() const noexcept
{
    return getPackData().disambiguator;
}

[[nodiscard]] const sf::base::String& HexagonGame::getPackAuthor() const noexcept
{
    return getPackData().author;
}

[[nodiscard]] const sf::base::String& HexagonGame::getPackName() const noexcept
{
    return getPackData().name;
}

[[nodiscard]] int HexagonGame::getPackVersion() const noexcept
{
    return getPackData().version;
}

void HexagonGame::playLevelMusic()
{
    if (shouldPlayMusic())
    {
        const MusicData::Segment segment = musicData.playRandomSegment(getPackId(), *audio);

        // TODO (P0): can this actually de-sync replays? beatpulse affects
        // gameplay
        // TODO (P1): problems with addHash in headless mode:
        status.beatPulseDelay += segment.beatPulseDelayOffset;
    }
}

void HexagonGame::playLevelMusicAtTime(float mSeconds)
{
    if (shouldPlayMusic())
    {
        musicData.playSeconds(getPackId(), *audio, mSeconds);
    }
}

void HexagonGame::stopLevelMusic()
{
    if (shouldPlayMusic())
    {
        audio->stopMusic();
    }
}

void HexagonGame::invalidateScore(const sf::base::String& mReason)
{
    if (status.scoreInvalid)
    {
        return;
    }

    status.scoreInvalid  = true;
    status.invalidReason = mReason;

    hg::lo("HexagonGame::invalidateScore") << "Invalidating official game (" << mReason << ")\n";
}

auto HexagonGame::getColorMain() const -> sf::Color
{
    if (Config::getBlackAndWhite())
    {
        return sf::Color(255, 255, 255, styleData.getMainColor().a);
    }

    return styleData.getMainColor();
}

auto HexagonGame::getColorPlayer() const -> sf::Color
{
    if (Config::getBlackAndWhite())
    {
        return sf::Color(255, 255, 255, styleData.getPlayerColor().a);
    }

    return styleData.getPlayerColor();
}

auto HexagonGame::getColorPlayerAdjustedForSwap() const -> sf::Color
{
    if (Config::getBlackAndWhite())
    {
        return sf::Color(255, 255, 255, styleData.getPlayerColor().a);
    }

    if (!Config::getShowSwapBlinkingEffect())
    {
        return getColorPlayer();
    }

    return player.getColorAdjustedForSwap(getColorPlayer());
}

auto HexagonGame::getColorPlayerTrail() const -> sf::Color
{
    return Config::getPlayerTrailHasSwapColor() ? getColorPlayerAdjustedForSwap() : getColorPlayer();
}

auto HexagonGame::getColorText() const -> sf::Color
{
    if (Config::getBlackAndWhite())
    {
        return sf::Color(255, 255, 255, styleData.getTextColor().a);
    }

    return styleData.getTextColor();
}

auto HexagonGame::getColorCap() const -> sf::Color
{
    if (Config::getBlackAndWhite())
    {
        return sf::Color::Black;
    }

    return styleData.getCapColorResult();
}

auto HexagonGame::getColorWall() const -> sf::Color
{
    if (Config::getBlackAndWhite())
    {
        return sf::Color(255, 255, 255, styleData.getWallColor().a);
    }

    return styleData.getWallColor();
}

[[nodiscard]] float HexagonGame::getMusicDMSyncFactor() const
{
    return std::pow(difficultyMult, 0.12f);
}

[[nodiscard]] float HexagonGame::getOptionalMusicDMSyncFactor() const
{
    return levelStatus.syncMusicToDM ? getMusicDMSyncFactor() : 1.f;
}

void HexagonGame::refreshMusicPitch()
{
    if (audio != nullptr)
    {
        audio->setCurrentMusicPitch(getOptionalMusicDMSyncFactor() * Config::getMusicSpeedMult() * levelStatus.musicPitch);
    }
}

void HexagonGame::setSides(unsigned int mSides)
{
    playSoundOverride(levelStatus.beepSound);

    if (mSides < 3)
    {
        mSides = 3;
    }

    levelStatus.sides = mSides;
}

[[nodiscard]] ssvs::GameState& HexagonGame::getGame() noexcept
{
    return game;
}

[[nodiscard]] float HexagonGame::getRadius() const noexcept
{
    return status.radius;
}

[[nodiscard]] const sf::Color& HexagonGame::getColor(int mIdx) const noexcept
{
    return styleData.getColor(mIdx);
}

[[nodiscard]] float HexagonGame::getSpeedMultDM() const noexcept
{
    const auto res = levelStatus.speedMult * (std::pow(difficultyMult, 0.65f));

    if (!levelStatus.hasSpeedMaxLimit())
    {
        return res;
    }

    return (res < levelStatus.speedMax) ? res : levelStatus.speedMax;
}

[[nodiscard]] float HexagonGame::getDelayMultDM() const noexcept
{
    const auto res = levelStatus.delayMult / (std::pow(difficultyMult, 0.10f));

    if (!levelStatus.hasDelayMaxLimit())
    {
        return res;
    }

    return (res < levelStatus.delayMax) ? res : levelStatus.delayMax;
}

[[nodiscard]] float HexagonGame::getRotationSpeed() const noexcept
{
    return levelStatus.rotationSpeed;
}

[[nodiscard]] unsigned int HexagonGame::getSides() const noexcept
{
    return levelStatus.sides;
}

[[nodiscard]] float HexagonGame::getWallSkewLeft() const noexcept
{
    return levelStatus.wallSkewLeft;
}

[[nodiscard]] float HexagonGame::getWallSkewRight() const noexcept
{
    return levelStatus.wallSkewRight;
}

[[nodiscard]] float HexagonGame::getWallAngleLeft() const noexcept
{
    return levelStatus.wallAngleLeft;
}

[[nodiscard]] float HexagonGame::getWallAngleRight() const noexcept
{
    return levelStatus.wallAngleRight;
}

[[nodiscard]] HexagonGameStatus& HexagonGame::getStatus() noexcept
{
    return status;
}

[[nodiscard]] const HexagonGameStatus& HexagonGame::getStatus() const noexcept
{
    return status;
}

[[nodiscard]] LevelStatus& HexagonGame::getLevelStatus()
{
    return levelStatus;
}

[[nodiscard]] HGAssets& HexagonGame::getAssets()
{
    return assets;
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
    return activeReplay.hasValue();
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
    runVoidLuaFunctionIfExists("onCursorSwap");

    if (mPlaySound)
    {
        playSoundOverride(getLevelStatus().swapSound);
    }
}

void HexagonGame::performPlayerKill()
{
    // Menu/preview HG instances visualize a level in the background -- they
    // must never kill the player or trip the death cascade, regardless of
    // what walls collide or what Lua scripts request.
    if (previewMode)
    {
        return;
    }

    const bool fatal = !Config::getInvincible() && !getLevelStatus().tutorialMode;

    player.kill(fatal);
    death();
}

} // namespace hg
