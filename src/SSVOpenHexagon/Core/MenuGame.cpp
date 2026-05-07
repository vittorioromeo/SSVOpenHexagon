// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/HexagonClient.hpp"
#include "SSVOpenHexagon/Core/HexagonDialogBox.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/LeaderboardCache.hpp"
#include "SSVOpenHexagon/Core/LuaScripting.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/RandomNumberGenerator.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/GameSystem/GameState.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Input/Bind.hpp"
#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/InputState.hpp"
#include "SSVOpenHexagon/Input/Manager.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/FontHeight.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Priv/GlslFwd.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Exchange.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <SSVUtils/Core/String/ToStr.hpp>
#include <tuple>

#include <cstdio>
#include <cstdlib>


namespace hg
{

void MenuGame::MenuFont::updateHeight()
{
    height = hg::Utils::getFontHeight(font);
}


void MenuGame::initOnlineIcons()
{
    assets.getTexture("onlineIcon.png").setSmooth(true);
    assets.getTexture("onlineIconFail.png").setSmooth(true);
}

//*****************************************************
//
// INITIALIZATION
//
//*****************************************************

inline constexpr float maxOffset{100.f};

MenuGame::MenuGame(Steam::steam_manager&     mSteamManager,
                   Discord::discord_manager& mDiscordManager,
                   HGAssets&                 mAssets,
                   Audio&                    mAudio,
                   ssvs::GameWindow&         mGameWindow,
                   HexagonClient&            mHexagonClient) :
    steamManager(mSteamManager),
    discordManager(mDiscordManager),
    assets(mAssets),
    openSquare(mAssets.getFont("OpenSquare-Regular.ttf")),
    openSquareBold(mAssets.getFont("OpenSquare-Bold.ttf")),
    audio(mAudio),
    window(mGameWindow),
    hexagonClient{mHexagonClient},
    dialogBox(openSquare, mGameWindow),
    leaderboardCache{sf::base::makeUnique<LeaderboardCache>()},
    lua{},
    execScriptPackPathContext{},
    currentPack{nullptr},
    txTitleBar{assets.getTexture("titleBar.png")},
    txEpilepsyWarning{assets.getTexture("epilepsyWarning.png")},
    titleBar{.textureRect = txTitleBar.getRect()},
    epilepsyWarning{.textureRect = txEpilepsyWarning.getRect()},
    txSOnline{&assets.getTexture("onlineIconFail.png")},
    sOnline{.textureRect = txSOnline->getRect()},
    rsOnlineStatus{{.size = {128.f, 32.f}}},
    txtOnlineStatus{openSquare, {.string = "", .characterSize = 24}},
    backgroundCamera{
        sf::View{.center = sf::Vec2f{0.f, 0.f},
                 .size = {Config::getSizeX() * Config::getZoomFactor(), Config::getSizeY() * Config::getZoomFactor()}}},
    overlayCamera{
        sf::View{.center = {Config::getWidth() / 2.f, Config::getHeight() * Config::getZoomFactor() / 2.f},
                 .size = {Config::getWidth() * Config::getZoomFactor(), Config::getHeight() * Config::getZoomFactor()}}},
    mustRefresh{false},
    state{States::EpilepsyWarning},
    levelStatus{Config::getMusicSpeedDMSync(), Config::getSpawnDistance()},
    ignoreInputs{0},
    w{0.f},
    h{0.f},
    fourByThree{false},
    levelData{},
    styleData{},
    txtProf{{openSquare, {.string = "", .characterSize = 18}}},
    txtSelectionSmall{{openSquare, {.string = "", .characterSize = 14}}}
{
    // Set cursor visible by default, will be disabled when using keyboard and
    // re-enabled when moving the mouse.
    setMouseCursorVisible(true);

    if (Config::getFirstTimePlaying())
    {
        showFirstTimeTips = true;
        Config::setFirstTimePlaying(false);
    }

    initAssets();
    initOnlineIcons();
    refreshCamera();

    game.onUpdate += [this](float mFT) { update(mFT); };

    game.onDraw += [this] { draw(); };

    const auto checkCloseBootScreens = [this]
    {
        if ((--ignoreInputs) == 0)
        {
            // EpilepsyWarning → SMain on any key press.
            playLocally();
            setIgnoreAllInputs(0);
            playSoundOverride("select.ogg");
        }
    };

    const auto checkCloseDialogBox = [this]
    {
        const auto closeBox = [this]
        {
            playSoundOverride("select.ogg");
            dialogBox.clearDialogBox();
            setIgnoreAllInputs(0);
        };

        const auto transitionInputSequence = [this, closeBox](const DialogInputState newState)
        {
            dialogInputState = newState;
            SFML_BASE_SCOPE_GUARD({ closeBox(); });
            return dialogBox.getInput();
        };

        const auto endInputSequence = [this, closeBox]
        {
            dialogInputState = DialogInputState::Nothing;
            SFML_BASE_SCOPE_GUARD({ closeBox(); });
            return dialogBox.getInput();
        };

        if (ignoreInputs != 0)
        {
            return;
        }

        if (dialogInputState == DialogInputState::Nothing)
        {
            closeBox();
            return;
        }

        if (dialogInputState == DialogInputState::Registration_EnteringUsername)
        {
            registrationUsername = transitionInputSequence(DialogInputState::Registration_EnteringPassword);

            showInputDialogBoxNice("REGISTRATION", "PASSWORD");
            dialogBox.setInputBoxPassword(true);
            setIgnoreAllInputs(1);
            return;
        }

        if (dialogInputState == DialogInputState::Registration_EnteringPassword)
        {
            registrationPassword = transitionInputSequence(DialogInputState::Registration_EnteringPasswordConfirm);

            showInputDialogBoxNice("REGISTRATION", "CONFIRM PASSWORD");
            dialogBox.setInputBoxPassword(true);
            setIgnoreAllInputs(1);
            return;
        }

        if (dialogInputState == DialogInputState::Registration_EnteringPasswordConfirm)
        {
            registrationPasswordConfirm = endInputSequence();

            if (registrationPassword == registrationPasswordConfirm)
            {
                hexagonClient.tryRegister(registrationUsername, registrationPassword);
            }
            else
            {
                playSoundOverride("error.ogg");

                showDialogBox(
                    "PASSWORD MISMATCH\n\n"
                    "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");

                setIgnoreAllInputs(1);
            }

            return;
        }

        if (dialogInputState == DialogInputState::Login_EnteringUsername)
        {
            loginUsername = transitionInputSequence(DialogInputState::Login_EnteringPassword);

            showInputDialogBoxNice("LOGIN", "PASSWORD");
            dialogBox.setInputBoxPassword(true);
            setIgnoreAllInputs(1);
            return;
        }

        if (dialogInputState == DialogInputState::Login_EnteringPassword)
        {
            loginPassword = endInputSequence();
            hexagonClient.tryLogin(loginUsername, loginPassword);
            return;
        }

        if (dialogInputState == DialogInputState::DeleteAccount_EnteringPassword)
        {
            deleteAccountPassword = endInputSequence();
            hexagonClient.tryDeleteAccount(deleteAccountPassword);
            return;
        }
    };

    game.onAnyEvent += [this, checkCloseBootScreens, checkCloseDialogBox](const sf::Event& event)
    {
        if (const auto* e = event.getIf<sf::Event::Resized>())
        {
            changeResolutionTo(e->size.x, e->size.y);
        }
        else if (const auto* e = event.getIf<sf::Event::TextEntered>())
        {
            // Feed printable ASCII into the new UI's typedChars buffer so
            // text fields can read it. The buffer is NUL-terminated, max
            // 7 chars per frame; excess bytes are dropped.
            if (newUIActiveForCurrentState() && e->unicode >= 32 && e->unicode < 127)
            {
                hg::ui::Input&  uin = ui_pendingInput;
                sf::base::SizeT len = 0;
                while (len < 7 && uin.typedChars[len] != '\0')
                    ++len;
                if (len < 7)
                {
                    uin.typedChars[len]     = static_cast<char>(e->unicode);
                    uin.typedChars[len + 1] = '\0';
                }
            }

            // Mirror printable text into the dialog-box input field when
            // one is open (used by the online login / register flow).
            if (!dialogBox.empty() && dialogBox.isInputBox() && e->unicode >= 32 && e->unicode < 127)
            {
                sf::base::String& input = dialogBox.getInput();
                if (input.size() < 32)
                {
                    playSoundOverride("beep.ogg");
                    input.pushBack(static_cast<char>(e->unicode));
                }
            }
        }
        else if (const auto* e = event.getIf<sf::Event::KeyPressed>())
        {
            if (window.hasFocus())
            {
                setMouseCursorVisible(false);
            }

            // Clear the input lock that `returnToLevelSelection` (and
            // similar) sets on a *fresh* keypress. Without this the
            // user has to press the key twice to start a second level:
            // the first press is eaten while `ignoreAllInputs(true)` is
            // armed, then the release clears the lock, and only the
            // second press makes it through the trigger system.
            //
            // Held keys carrying over from gameplay don't produce a
            // KeyPressed event (only a release-then-press transition
            // does), so this path won't auto-fire e.g. PLAY just
            // because the user came back from a level still holding
            // Enter. Skipped during EpilepsyWarning (which deliberately
            // requires a release to dismiss the splash) and while a
            // dialog box is open (dialog has its own ignore-counter
            // accounting on release).
            if (ignoreInputs > 0 && state != States::EpilepsyWarning && dialogBox.empty())
            {
                setIgnoreAllInputs(0);
            }

            // Backspace inside the legacy dialog input box.
            if (!dialogBox.empty() && dialogBox.isInputBox() && e->code == sf::Keyboard::Key::Backspace)
            {
                sf::base::String& input = dialogBox.getInput();
                if (!input.empty())
                {
                    playSoundOverride("beep.ogg");
                    input.erase(input.size() - 1, 1);
                }
            }
        }
        else if (event.is<sf::Event::MouseMoved>())
        {
            if (window.hasFocus())
            {
                setMouseCursorVisible(true);
            }
        }
        else if (event.is<sf::Event::MouseButtonPressed>() || event.is<sf::Event::JoystickButtonPressed>())
        {
            // Same lock-clear as KeyPressed above, applied to the
            // fresh-mouse-click path. Without it the first click after
            // returning from gameplay does nothing because
            // `drawNewMainMenu` gates `mouseDown` on `ignoreInputs == 0`
            // (see the gate around `sf::Mouse::isButtonPressed`).
            if (ignoreInputs > 0 && state != States::EpilepsyWarning && dialogBox.empty())
            {
                setIgnoreAllInputs(0);
            }
        }
        else if (const auto* e = event.getIf<sf::Event::KeyReleased>())
        {
            if (ignoreInputs == 0)
            {
                return;
            }

            // Boot screen -- any key advances to the main menu.
            if (state == States::EpilepsyWarning)
            {
                checkCloseBootScreens();
                return;
            }

            // Dialog box (login / register flow). Any key (or the
            // configured close key) ticks down `ignoreInputs`; once it
            // reaches zero the dialog is dismissed.
            if (dialogBoxDelay > 0.f)
            {
                return;
            }
            if (!dialogBox.empty())
            {
                const sf::Keyboard::Key key{e->code};
                if (dialogBox.getKeyToClose() == sf::Keyboard::Key::Unknown || key == dialogBox.getKeyToClose())
                {
                    --ignoreInputs;
                }
                if (dialogBox.isInputBox() && key == sf::Keyboard::Key::Escape)
                {
                    setIgnoreAllInputs(0);
                    dialogInputState = DialogInputState::Nothing;
                    playSoundOverride("select.ogg");
                    dialogBox.clearDialogBox();
                    return;
                }
                checkCloseDialogBox();
                return;
            }

            // No menus left to dispatch into -- just clear the lock.
            setIgnoreAllInputs(0);
        }
        else if (event.is<sf::Event::MouseButtonReleased>() || event.is<sf::Event::JoystickButtonReleased>())
        {
            if (ignoreInputs == 0)
            {
                return;
            }
            if (state == States::EpilepsyWarning)
            {
                checkCloseBootScreens();
                return;
            }
            if (dialogBoxDelay > 0.f)
            {
                return;
            }
            if (!dialogBox.empty())
            {
                if (dialogBox.getKeyToClose() == sf::Keyboard::Key::Unknown)
                {
                    --ignoreInputs;
                    checkCloseDialogBox();
                }
                return;
            }
            setIgnoreAllInputs(0);
        }
    };

    // To close the load results with any key
    setIgnoreAllInputs(1);

    window.onRecreation += [this]
    {
        refreshCamera();
        mustRefresh = true;
    };

    initInput();
    initNewUIServices();
}

MenuGame::~MenuGame()
{
    hg::lo("MenuGame::~MenuGame") << "Cleaning up menu resources...\n";
}

void MenuGame::init(bool error)
{
    steamManager.set_rich_presence_in_menu();
    steamManager.update_hardcoded_achievements();

    discordManager.set_rich_presence_in_menu();

    audio.stopMusic();
    audio.stopSounds();

    if (error)
    {
        playSoundOverride("error.ogg");
    }
    else
    {
        playSoundOverride("select.ogg");
    }

    // Online::setForceLeaderboardRefresh(true);
}

void MenuGame::init(bool error, const sf::base::String& pack, const sf::base::String& level)
{
    init(error);
    loadCommandLineLevel(pack, level);
}

// ----------------------------------------------------------------------------
// New immediate-mode UI integration (see `docs/UI_REWRITE_DESIGN.md`).

bool MenuGame::newUIActiveForCurrentState() const noexcept
{
    // The new UI owns every reachable screen; the only legacy state still
    // around is `EpilepsyWarning`, which is the boot-time splash.
    return state == States::SMain;
}

void MenuGame::initNewUIServices()
{
    // Wire the new UI's action callbacks to existing MenuGame helpers.
    ui_services.onExit = [this] { window.stop(); };

    ui_services.onOnlineConnect    = [this] { hexagonClient.connect(); };
    ui_services.onOnlineDisconnect = [this] { hexagonClient.disconnect(); };
    ui_services.onOnlineLogout     = [this] { hexagonClient.tryLogoutFromServer(); };
    ui_services.onOnlineLogin      = [this]
    {
        // Reuse the legacy login dialog overlay -- it already runs on top
        // of the new UI's draw because dialogBox rendering happens in
        // `MenuGame::draw` after `drawNewMainMenu`.
        if (dialogInputState != DialogInputState::Nothing)
            return;
        openLoginDialogBoxAndStartLoginProcess();
        ignoreInputsAfterMenuExec();
    };
    ui_services.onOnlineRegister = [this]
    {
        if (dialogInputState != DialogInputState::Nothing)
            return;
        dialogInputState = DialogInputState::Registration_EnteringUsername;
        showInputDialogBoxNice("REGISTRATION", "USERNAME");
        ignoreInputsAfterMenuExec();
    };
    ui_services.onStartLevel = [this](const sf::base::String& levelId, float difficultyMult)
    {
        // `levelId` is the pack-prefixed asset key as stored in
        // `levelDataIdsByPack` -- `LevelData::id` alone wouldn't pass
        // `isValidLevelId`. Direct call to the gameplay-launch hook, no
        // legacy menu state to populate.
        if (!assets.isValidLevelId(levelId) || !hostCallbacks.newGame)
        {
            return;
        }
        const LevelData& ld = assets.getLevelData(levelId);
        setMouseCursorVisible(false);
        hostCallbacks.newGame(ld.packId, levelId, /*firstPlay=*/true, difficultyMult, /*executeLastReplay=*/false);
    };

    ui_services.onPreviewLevel = [this](const sf::base::String& levelId)
    {
        // Reload the preview HG with the newly-selected level so its
        // walls / style / 3D animate inside the LevelSelect right-side
        // preview texture. Skipped if we're already showing this level.
        if (!assets.isValidLevelId(levelId) || hgPreview == nullptr || previewLoadedLevelId == levelId)
        {
            return;
        }
        const LevelData& ld = assets.getLevelData(levelId);
        hgPreview->newGame(ld.packId,
                           levelId,
                           /*firstPlay=*/true,
                           /*difficultyMult=*/1.f,
                           /*executeLastReplay=*/false);
        hgPreview->setMustStart(true);
        previewLoadedLevelId = levelId;
    };

    ui_services.onRequestLeaderboard = [this](const sf::base::String& levelId, const float diffMult)
    {
        // Resolve `(levelId, diffMult)` to a validator on `this`, then
        // (if the level is actually trackable) try to send a request.
        // The per-frame refresh in `drawNewMainMenu` picks up the new
        // validator + any cache changes on the next tick -- accepting
        // a one-frame display lag in exchange for not paying for a
        // refresh on every poll (the screen calls this every frame the
        // leaderboard column is visible).
        if (tryUpdateLeaderboardValidator(levelId, diffMult))
        {
            maybeIssueLeaderboardTopScoresRequest();
        }
    };

    ui_services.playSound = [this](sf::base::StringView s)
    {
        // Best-effort; small stack buffer keeps us null-terminated.
        char       buf[64] = {};
        const auto n       = s.size() < (sizeof(buf) - 1) ? s.size() : (sizeof(buf) - 1);
        SFML_BASE_MEMCPY(buf, s.data(), n);
        playSoundOverride(buf);
    };

    ui_services.onWatchReplay = [this](const sf::base::U64 scoreTimestamp)
    {
        // The validator is whatever `onRequestLeaderboard` last set --
        // i.e. the (level, difficulty) the LevelSelect cursor is on.
        // The screen polls `onRequestLeaderboard` every frame the
        // leaderboard column is visible, so this is always fresh.
        if (currentLeaderboardValidator.empty())
        {
            return;
        }

        // Dedup: if the user mashes Enter on the same row, we already
        // have a request in flight (or just finished one). Re-sending
        // wastes wire bandwidth and the server has its own throttle
        // anyway -- but quieting the client side stops the request
        // from looking distinct enough to slip through (e.g. across
        // network jitter). Cleared on validator change below so a new
        // selection always gets a fresh shot.
        if (lastReplayRequestValidator == currentLeaderboardValidator && lastReplayRequestTimestamp == scoreTimestamp)
        {
            return;
        }

        if (!hexagonClient.tryRequestReplay(currentLeaderboardValidator, scoreTimestamp))
        {
            hg::lo("hg::MenuGame::onWatchReplay")
                << "tryRequestReplay refused for validator='" << currentLeaderboardValidator
                << "' (state=" << static_cast<int>(hexagonClient.getState()) << ")\n";
            return;
        }

        lastReplayRequestValidator = currentLeaderboardValidator;
        lastReplayRequestTimestamp = scoreTimestamp;

        hg::lo("hg::MenuGame::onWatchReplay")
            << "Requested replay for validator='" << currentLeaderboardValidator << "' ts=" << scoreTimestamp << '\n';
    };

    ui_services.assets       = &assets;
    ui_services.steamManager = &steamManager;
    // `currentProfile` is refreshed each frame in `drawNewMainMenu` because
    // no profile is selected at construction time (it's chosen later via
    // `SLPSelectBoot`/`SLPSelect`).
    ui_services.currentProfile = nullptr;
}

void MenuGame::setMenuPreviewGames(HexagonGame*     menuBackground,
                                   HexagonGame*     preview,
                                   sf::base::String menuBackgroundPackId,
                                   sf::base::String menuBackgroundLevelId)
{
    hgMenuBg  = menuBackground;
    hgPreview = preview;

    // Boot the menu-background level once. Skipped silently when the
    // configured pack/level isn't installed -- the menu still works, just
    // without an animated backdrop.
    if (hgMenuBg != nullptr && !menuBackgroundPackId.empty() && !menuBackgroundLevelId.empty() &&
        assets.isValidPackId(menuBackgroundPackId) && assets.isValidLevelId(menuBackgroundLevelId))
    {
        hgMenuBg->newGame(menuBackgroundPackId,
                          menuBackgroundLevelId,
                          /*firstPlay=*/true,
                          /*difficultyMult=*/1.f,
                          /*executeLastReplay=*/false);
        // Force the level to start on the first `update()` so walls begin
        // spawning immediately. `start()` is private -- flagging via
        // `setMustStart` is the public path used by replay tooling too.
        hgMenuBg->setMustStart(true);
    }

    // Allocate the off-screen target the preview HG will draw into. Fixed
    // 16:9 so the miniature preview keeps a consistent shape regardless of
    // the actual window resolution / aspect ratio. The LevelSelect screen
    // sizes the on-screen preview to the same ratio.
    if (hgPreview != nullptr)
    {
        constexpr sf::Vec2u previewSize{1280u, 720u};
        if (auto rt = sf::RenderTexture::create(previewSize); rt.hasValue())
        {
            previewTexture          = SFML_BASE_MOVE(rt);
            hgPreview->renderTarget = &*previewTexture;
        }
    }
}

[[nodiscard]] bool MenuGame::tryUpdateLeaderboardValidator(const sf::base::String& levelId, const float diffMult)
{
    // Invalid asset id: clear the validator + dedup. The per-frame
    // snapshot will then publish `Connecting` / `Offline` based on the
    // client state, since `haveValidator` is false.
    if (!assets.isValidLevelId(levelId))
    {
        if (!currentLeaderboardValidator.empty())
        {
            currentLeaderboardValidator.clear();
            lastReplayRequestValidator.clear();
            lastReplayRequestTimestamp = 0;
        }
        return false;
    }

    const LevelData& ld           = assets.getLevelData(levelId);
    const auto&      newValidator = ld.getValidator(diffMult);

    const bool validatorChanged = (currentLeaderboardValidator != newValidator);
    currentLeaderboardValidator = newValidator;

    if (validatorChanged)
    {
        // Reset the watch-replay dedup so navigating away/back lets the
        // user re-watch the same row.
        lastReplayRequestValidator.clear();
        lastReplayRequestTimestamp = 0;
    }

    // Skip the wire send entirely for levels the server doesn't track:
    // it would never reply and the UI would sit on "LOADING..." forever.
    // The server populates `_supportedLevelValidators` only after
    // `LoggedIn_Ready`; pre-ready frames fall through and the per-frame
    // snapshot still classifies them as `Connecting`.
    const auto hcState = hexagonClient.getState();
    if (hcState == HexagonClient::State::LoggedIn_Ready &&
        !hexagonClient.isLevelSupportedByServer(currentLeaderboardValidator))
    {
        if (validatorChanged)
        {
            hg::lo("hg::MenuGame::onRequestLeaderboard")
                << "validator='" << currentLeaderboardValidator << "' unsupported, skipping request\n";
        }
        return false;
    }

    return true;
}

void MenuGame::maybeIssueLeaderboardTopScoresRequest()
{
    // Cache rate-limit: refuses re-sends within 6s of the previous
    // marker for the same validator.
    if (!leaderboardCache->shouldRequestScores(currentLeaderboardValidator))
    {
        return;
    }

    const bool sent = hexagonClient.tryRequestTopScores(currentLeaderboardValidator);
    if (sent)
    {
        // Mark only on success. A failed send (e.g. wrong state) doesn't
        // poison the cache window -- otherwise a later retry from a
        // healthy state would be silently rate-limited away for 6s.
        leaderboardCache->requestedScores(currentLeaderboardValidator);
    }

    hg::lo("hg::MenuGame::onRequestLeaderboard")
        << "validator='" << currentLeaderboardValidator << "' sent=" << (sent ? "yes" : "no")
        << " state=" << static_cast<int>(hexagonClient.getState()) << '\n';
}

void MenuGame::refreshLeaderboardSnapshot()
{
    using S   = HexagonClient::State;
    using LBS = hg::ui::Services::LeaderboardStatus;

    const S hcSt = hexagonClient.getState();

    const bool haveValidator    = !currentLeaderboardValidator.empty();
    const bool haveReceived     = haveValidator && leaderboardCache->hasReceivedScores(currentLeaderboardValidator);
    const bool requestInFlight  = haveValidator && !haveReceived &&
                                  leaderboardCache->hasInformation(currentLeaderboardValidator);
    const bool readyToFetch     = (hcSt == S::LoggedIn_Ready);
    const bool stillHandshaking = (hcSt == S::Connected || hcSt == S::Connecting || hcSt == S::LoggedIn);
    const bool unsupportedHere  = readyToFetch && haveValidator &&
                                  !hexagonClient.isLevelSupportedByServer(currentLeaderboardValidator);

    if (haveReceived)
    {
        // Server replied. Vector may legitimately be empty; the screen
        // renders that as "NO SCORES". Highest priority -- once a real
        // reply has arrived, transient state changes shouldn't mask it.
        ui_services.leaderboardScores = &leaderboardCache->getScores(currentLeaderboardValidator);
        ui_services.leaderboardStatus = LBS::Ready;
    }
    else if (unsupportedHere)
    {
        // Connected, ready, level highlighted, but the server doesn't
        // have this validator on its whitelist -- it would simply not
        // reply if we asked. Surface that explicitly so the user
        // doesn't watch a permanent spinner.
        ui_services.leaderboardScores = nullptr;
        ui_services.leaderboardStatus = LBS::Unsupported;
    }
    else if (requestInFlight && readyToFetch)
    {
        // We sent a request and are waiting on the server.
        ui_services.leaderboardScores = nullptr;
        ui_services.leaderboardStatus = LBS::Loading;
    }
    else if (stillHandshaking || (readyToFetch && !haveValidator))
    {
        // Either the client is still negotiating with the server, or we
        // just haven't selected a level yet.
        ui_services.leaderboardScores = nullptr;
        ui_services.leaderboardStatus = LBS::Connecting;
    }
    else
    {
        // Disconnected / init error / connection error -- no path forward.
        ui_services.leaderboardScores = nullptr;
        ui_services.leaderboardStatus = LBS::Offline;
    }

    // Publish the per-validator "no replay" set. Always non-null (the
    // cache returns a static empty set when no entry exists), so the
    // screen can do unconditional `contains` lookups when annotating
    // rows.
    ui_services.leaderboardUnavailable = &leaderboardCache->getUnavailableTimestamps(currentLeaderboardValidator);
}

void MenuGame::pumpWorkshopEvents()
{
    using EK = hg::Steam::WorkshopEvent::Kind;

    // Helper: stash {id, title} pairs into the screen's name cache so the
    // dependency list can render readable titles instead of raw 64-bit ids.
    const auto mergeIntoNameCache = [&](const auto& items)
    {
        auto& cache = ui_app.workshop.nameCache;
        for (const auto& it : items)
        {
            bool present = false;
            for (auto& e : cache)
            {
                if (e.publishedFileId == it.publishedFileId)
                {
                    e.title = it.title;
                    present = true;
                    break;
                }
            }
            if (!present)
            {
                cache.emplaceBack(hg::ui::WorkshopBrowseScreenState::WorkshopNameEntry{it.publishedFileId, it.title});
            }
        }
    };

    while (auto evt = steamManager.poll_workshop_event())
    {
        switch (evt->kind)
        {
            case EK::QueryComplete:
                mergeIntoNameCache(evt->queryResults);
                ui_app.workshop.items         = SFML_BASE_MOVE(evt->queryResults);
                ui_app.workshop.queryInFlight = false;
                ui_app.workshop.totalMatching = evt->totalMatching;
                std::snprintf(ui_app.workshop.statusMessage,
                              sizeof(ui_app.workshop.statusMessage),
                              "%zu items received",
                              static_cast<std::size_t>(ui_app.workshop.items.size()));
                break;

            case EK::DetailsComplete:
                // On-demand lookup (e.g. dep titles). Don't touch `items` --
                // those drive the visible list. Only populate the name cache.
                mergeIntoNameCache(evt->queryResults);
                break;

            case EK::ItemInstalled:
                if (!evt->installFolder.empty())
                {
                    (void)assets.installPackAtRuntime(evt->installFolder);
                }
                // Reflect install state in the UI's cached list.
                for (auto& it : ui_app.workshop.items)
                {
                    if (it.publishedFileId == evt->publishedFileId)
                    {
                        it.isInstalled = true;
                        break;
                    }
                }
                break;

            case EK::ItemSubscribed:
            case EK::ItemUnsubscribed:
                for (auto& it : ui_app.workshop.items)
                {
                    if (it.publishedFileId == evt->publishedFileId)
                    {
                        it.isSubscribed = (evt->kind == EK::ItemSubscribed);
                        break;
                    }
                }
                break;

            case EK::DownloadProgress:
                // Future: surface a progress bar. For now we just note the
                // event and let `ItemInstalled` close the loop.
                break;

            case EK::PreviewDownloaded:
            {
                // Decode the HTTP response bytes into a `sf::Texture` and
                // park it in the per-item cache. Failures (corrupt image,
                // unsupported format) leave the optional empty so the UI
                // still shows the "(NO PREVIEW)" placeholder.
                auto& slot = ui_app.workshop.previewTextures[evt->publishedFileId];
                if (!evt->previewBytes.empty())
                {
                    if (auto tex = sf::Texture::loadFromMemory(evt->previewBytes.data(), evt->previewBytes.size());
                        tex.hasValue())
                    {
                        tex->setSmooth(true);
                        slot = SFML_BASE_MOVE(tex);
                    }
                }
                break;
            }
        }
    }

    // Drive in-flight HTTP preview downloads. Cheap when nothing is pending.
    steamManager.pump_workshop_http();
}

void MenuGame::drawNewMainMenu()
{
    // Refresh per-frame snapshots the screens read from. Cheap; runs only
    // when the new UI is active. Profile may be unset (no local profile
    // chosen yet) -- handle that case so the new UI doesn't crash on first
    // boot before the user picks one.
    {
        const bool hasProfile      = assets.pIsValidLocalProfile();
        ui_services.currentProfile = hasProfile ? &assets.getCurrentLocalProfile() : nullptr;

        if (hasProfile)
        {
            const ProfileData& prof = assets.getCurrentLocalProfile();

            const auto& name = assets.pGetName();
            std::snprintf(ui_app.profileSnapshot.name,
                          sizeof(ui_app.profileSnapshot.name),
                          "%.*s",
                          static_cast<int>(name.size()),
                          name.cStr());

            ui_app.profileSnapshot.totalScored    = static_cast<int>(prof.getScores().size());
            ui_app.profileSnapshot.totalFavorites = static_cast<int>(prof.getFavoriteLevelIds().size());
        }
        else
        {
            std::snprintf(ui_app.profileSnapshot.name, sizeof(ui_app.profileSnapshot.name), "(none)");
            ui_app.profileSnapshot.totalScored    = 0;
            ui_app.profileSnapshot.totalFavorites = 0;
        }

        // Map the legacy `HexagonClient::State` into both a display string
        // and the capability flags consumed by the new Online screen.
        using S               = HexagonClient::State;
        const S     hcState   = hexagonClient.getState();
        const char* statusStr = hcState == S::Disconnected      ? "OFFLINE"
                                : hcState == S::InitError       ? "INIT ERROR"
                                : hcState == S::Connecting      ? "CONNECTING..."
                                : hcState == S::ConnectionError ? "CONNECTION ERROR"
                                : hcState == S::Connected       ? "CONNECTED"
                                : hcState == S::LoggedIn        ? "LOGGED IN"
                                : hcState == S::LoggedIn_Ready  ? "LOGGED IN (READY)"
                                                                : "OFFLINE";
        std::snprintf(ui_app.profileSnapshot.onlineStatus, sizeof(ui_app.profileSnapshot.onlineStatus), "%s", statusStr);

        const bool loggedIn                  = (hcState == S::LoggedIn || hcState == S::LoggedIn_Ready);
        const bool connected                 = (hcState == S::Connected || loggedIn);
        ui_app.profileSnapshot.canConnect    = !connected && hcState != S::Connecting;
        ui_app.profileSnapshot.canDisconnect = connected || hcState == S::Connecting;
        ui_app.profileSnapshot.canLogIn      = (hcState == S::Connected);
        ui_app.profileSnapshot.canRegister   = (hcState == S::Connected);
        ui_app.profileSnapshot.canLogOut     = loggedIn;
    }

    // Drain any pending Steam Workshop events before drawing -- hot-installs
    // a newly-downloaded pack, mirrors subscribe state into the cached item
    // list, etc. Cheap when the queue is empty.
    pumpWorkshopEvents();

    // Lazy-allocate the off-screen target the new UI renders into. Its
    // pixel size mirrors the window so the overlay view + mouse mapping
    // already in place keep working unchanged. Recreated on resize.
    {
        const sf::Vec2u winSz = window.getRenderWindow().getSize();
        const bool      need  = !uiCompositeTexture.hasValue() || uiCompositeTexture->getSize() != winSz;
        if (need && winSz.x > 0 && winSz.y > 0)
        {
            if (auto rt = sf::RenderTexture::create(winSz); rt.hasValue())
            {
                uiCompositeTexture = SFML_BASE_MOVE(rt);
            }
        }
    }

    // Lazy-load the post-process shader once. If the file is missing we
    // still draw the UI -- the gradient pass just becomes a passthrough.
    if (!menuAccentShaderLoadAttempted)
    {
        menuAccentShaderLoadAttempted = true;
        if (auto sh = sf::Shader::loadFromFile({.fragmentPath = "Assets/menuAccentGradient.frag"}); sh.hasValue())
        {
            menuAccentShader = SFML_BASE_MOVE(sh);
        }
    }

    menuAccentShaderTime += ui_dt * 50.f;

    // Build a Context for this frame.
    hg::ui::Context ctx{};
    // Render the UI into the off-screen composite texture when available;
    // otherwise fall back to drawing straight on the window so the menu
    // never goes invisible if texture allocation failed.
    ctx.target = uiCompositeTexture.hasValue() ? static_cast<sf::RenderTarget*>(&*uiCompositeTexture)
                                               : static_cast<sf::RenderTarget*>(&window.getRenderWindow());
    ctx.font   = &openSquare;
    ctx.input  = ui_pendingInput;
    ctx.dt     = ui_dt;

    // Refresh per-frame text metrics now that font + fontSize are set so
    // every row widget (`button`, `label`, `slider`, …) gets accurate
    // vertical centering via `rowTextY`. Cheap -- measures one glyph.
    hg::ui::recomputeTextMetrics(ctx);

    // Mouse button state. Position is mapped *below*, after `renderStates`
    // has been set up -- so widget hit-testing matches the transformed
    // render.
    ctx.input.mouseDown    = (ignoreInputs == 0) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    ctx.input.mousePressed = ctx.input.mouseDown && !mouseWasPressed;

    // Render the new UI through the same overlay view used by the title
    // bar, credits, and dialog box. That view is built in `refreshCamera`
    // around a virtual ~1366×768 design space, so UI coordinates stay
    // resolution-independent and grow with the window the same way the
    // legacy decorations do.
    ctx.renderStates.view      = getOverlayView();
    ctx.renderStates.transform = sf::Transform{}.scaleBy({0.75f, 0.75f});

    // Pointer used by widgets to play sound feedback (button click,
    // toggle flip, slider tick…) without threading `Services&` through
    // every signature. Cleared per-frame, since the dispatcher doesn't
    // own `Services`.
    ctx.services = &ui_services;

    // Surface the per-frame preview render texture (filled in by
    // `MenuGame::draw` from `hgPreview`) so LevelSelect can paint it.
    ui_services.previewTexture = previewTexture.hasValue() && !previewLoadedLevelId.empty() ? &*previewTexture : nullptr;

    // Window-as-target for screens that need to draw outside the
    // accent-gradient shader pass (e.g. the LevelSelect preview, whose
    // magenta pixels would otherwise be remapped). Only meaningful when
    // we're actually compositing through the UI texture; otherwise the
    // shader isn't running and a null pointer tells screens to fall back
    // to `ctx.target`.
    ui_services.rawTarget = uiCompositeTexture.hasValue() ? static_cast<sf::RenderTarget*>(&window.getRenderWindow()) : nullptr;

    // Surface the latest cached leaderboard snapshot for whatever
    // (level, difficulty) the LevelSelect screen last requested. The
    // cache holds entries indefinitely, so reusing a stale pointer
    // across frames is safe -- a fresh `EReceivedTopScores` event just
    // overwrites the underlying vector in place.
    refreshLeaderboardSnapshot();

    // Map raw pixel mouse → UI layout space, accounting for the view +
    // transform set on `renderStates`.
    const sf::Vec2f mousePixelPos = sf::Mouse::getPosition(window.getRenderWindow()).to<sf::Vec2f>();
    ctx.input.mousePixelPos       = mousePixelPos;
    ctx.input.mousePos            = hg::ui::screenToUI(ctx, mousePixelPos);

    // Clear the off-screen UI texture to fully transparent black so the
    // background level (rendered earlier into the window) shows through
    // wherever the UI doesn't draw.
    if (uiCompositeTexture.hasValue() && ctx.target == &*uiCompositeTexture)
    {
        uiCompositeTexture->clear(sf::Color{0, 0, 0, 0});
    }

    hg::ui::drawCurrentScreen(ctx, ui_app, ui_services);

    // Draw the dialog box (login / register / first-time-tip flows) into
    // the same off-screen UI texture so its magenta-sentinel frame goes
    // through the accent gradient shader at composite time. Without this
    // detour the dialog renders straight to the window after the shader
    // pass, and the frame stays plain magenta. Uses the same overlay view
    // the new UI uses so positions align with the rest of the layout.
    if (!dialogBox.empty() && uiCompositeTexture.hasValue() && ctx.target == &*uiCompositeTexture)
    {
        dialogBox.setRenderTargetOverride(&*uiCompositeTexture);
        dialogBox.draw(getOverlayView(),
                       /*txtColor=*/sf::Color{255, 255, 255, 255},
                       /*frameColor=*/sf::Color{255, 0, 255, 255},
                       /*backdropColor=*/sf::Color{0, 0, 0, 255});
        dialogBox.setRenderTargetOverride(nullptr);
    }

    // Composite the UI back onto the window with the gradient shader. The
    // shader leaves white text and dark row backgrounds alone but maps
    // every magenta-saturated pixel (selection pills, text outlines) to
    // an animated noise gradient.
    if (uiCompositeTexture.hasValue() && ctx.target == &*uiCompositeTexture)
    {
        uiCompositeTexture->display();

        const sf::Texture& tex     = uiCompositeTexture->getTexture();
        const sf::Vec2u    texSize = tex.getSize();
        const sf::Vec2u    winSize = window.getRenderWindow().getSize();

        sf::RenderStates states{};
        states.texture = &tex;
        states.view    = sf::View::fromScreenSize(winSize.to<sf::Vec2f>());

        if (menuAccentShader.hasValue())
        {
            if (auto loc = menuAccentShader->getUniformLocation("u_resolution"); loc.hasValue())
            {
                menuAccentShader->setUniform(*loc,
                                             sf::Glsl::Vec2{static_cast<float>(winSize.x), static_cast<float>(winSize.y)});
            }
            if (auto loc = menuAccentShader->getUniformLocation("u_time"); loc.hasValue())
            {
                menuAccentShader->setUniform(*loc, menuAccentShaderTime);
            }
            states.shader = &*menuAccentShader;
        }

        window.getRenderWindow().draw(
            sf::Sprite{
                .position    = {0.f, 0.f},
                .textureRect = {{0.f, 0.f}, texSize.to<sf::Vec2f>()},
            },
            states);
    }

    // Reset edges so they don't carry to next frame. Mouse pos/down are not
    // edges and survive -- they're rebuilt next frame anyway.
    ui_pendingInput = {};

    if (ui_app.exitRequested)
    {
        window.stop();
    }
}

// ----------------------------------------------------------------------------

void MenuGame::initAssets()
{
    for (const auto& t : {"titleBar.png", "epilepsyWarning.png"})
    {
        assets.getTexture(t).setSmooth(true);
    }
}

void MenuGame::changeStateTo(const States mState)
{
    const States prevState = state;
    state                  = mState;

    if (prevState == state)
    {
        // Not a state transition.
        return;
    }

    if (state == States::SMain)
    {
        if (sf::base::exchange(mustShowLoginAtStartup, false) && Config::getShowLoginAtStartup())
        {
            openLoginDialogBoxAndStartLoginProcess();
            setIgnoreAllInputs(2);
        }
    }

    if (!showFirstTimeTips)
    {
        // Not the first time playing.
        return;
    }

    if (state == States::SMain && sf::base::exchange(mustShowFTTMainMenu, false))
    {
        playSoundOverride("select.ogg");
        showDialogBox(
            "WELCOME TO OPEN HEXAGON!\n\n"
            "YOU CAN NAVIGATE THE MAIN MENU WITH THE UP/DOWN ARROW KEYS\n"
            "OR THE DPAD/THUMBSTICK ON YOUR CONTROLLER\n\n"
            "REMEMBER TO CHECK OUT THE OPTIONS MENU\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
        setIgnoreAllInputs(1);

        // Prevent dialog box from being closed immediately:
        dialogBoxDelay = 64.f;
    }
}

void MenuGame::initInput()
{
    using k = sf::Keyboard::Key;
    using t = ssvs::Input::Type;

    // Each navigable key feeds the new UI's edge-triggered input snapshot
    // directly. `drawNewMainMenu` reads `ui_pendingInput` once per frame
    // and clears it; the immediate-mode screens dispatch from there.
    game.addInput({{k::Up}}, [this](float) { ui_pendingInput.up = true; }, t::Once);
    game.addInput({{k::Down}}, [this](float) { ui_pendingInput.down = true; }, t::Once);
    game.addInput({{k::Left}}, [this](float) { ui_pendingInput.left = true; }, t::Once);
    game.addInput({{k::Right}}, [this](float) { ui_pendingInput.right = true; }, t::Once);
    game.addInput({{k::Enter}}, [this](float) { ui_pendingInput.enter = true; }, t::Once);
    game.addInput({{k::Escape}}, [this](float) { ui_pendingInput.escape = true; }, t::Once);
    game.addInput({{k::Backspace}}, [this](float) { ui_pendingInput.backspace = true; }, t::Once);

    // Alt+Enter toggles fullscreen -- preserved through the legacy refactor
    // because it's a global shortcut, not a menu action.
    game.addInput({{k::LAlt, k::Enter}},
                  [this](float)
    {
        Config::setFullscreen(window, !window.getFullscreen());
        game.ignoreNextInputs();
    },
                  t::Once)
        .setPriorityUser(-1000);
}

void MenuGame::runLuaFile(const sf::base::String& mFileName)
try
{
    if (Config::getUseLuaFileCache())
    {
        Utils::runLuaFileCached(assets, lua, mFileName);
    }
    else
    {
        Utils::runLuaFile(lua, mFileName);
    }
} catch (...)
{
    playSoundOverride("error.ogg");
    hg::lo("hg::MenuGame::initLua") << "Fatal error in menu for Lua file '" << mFileName << '\'' << logEndl;
}

void MenuGame::changeResolutionTo(unsigned int mWidth, unsigned int mHeight)
{
    if (Config::getWidth() == mWidth && Config::getHeight() == mHeight)
    {
        return;
    }

    Config::setCurrentResolution(mWidth, mHeight);
    window.getRenderWindow().setSize(sf::Vec2u{mWidth, mHeight});

    refreshCamera();
}

void MenuGame::playSoundOverride(const sf::base::String& assetId)
{
    if (!Config::getNoSound())
    {
        audio.playSoundOverride(assetId);
    }
}

void MenuGame::setMouseCursorVisible(const bool x)
{
    window.setMouseCursorVisible(x);
}

void MenuGame::playLocally()
{
    assets.pSaveCurrent();

    // Single-profile model: auto-create a default profile on first boot,
    // otherwise just take whichever exists. The legacy "select a local
    // profile" UI is gone.
    if (assets.getLocalProfilesSize() == 0)
    {
        assets.pCreate(sf::base::String{"PLAYER"});
        assets.pSetCurrent(sf::base::String{"PLAYER"});
    }
    else if (!assets.pIsValidLocalProfile())
    {
        const auto names = assets.getLocalProfileNames();
        if (!names.empty())
            assets.pSetCurrent(names[0]);
    }

    changeStateTo(States::SMain);
}

void MenuGame::ignoreInputsAfterMenuExec()
{
    setIgnoreAllInputs(2);
}

bool MenuGame::loadCommandLineLevel(const sf::base::String& /*pack*/, const sf::base::String& /*level*/)
{
    // Legacy command-line level loading. The new UI doesn't expose it
    // and the implementation depended on legacy menu state -- leaving it
    // as a no-op so the `--level` flag silently does nothing rather
    // than crashing.
    return false;
}

void MenuGame::initLua()
{
    static CCustomWallManager      cwManager;
    static random_number_generator rng{0};
    static HexagonGameStatus       hexagonGameStatus;

    LuaScripting::init(lua,
                       rng,
                       true /* inMenu */,
                       cwManager,
                       levelStatus,
                       hexagonGameStatus,
                       styleData,
                       assets,
                       [this](const sf::base::String& filename) { runLuaFile(filename); },
                       execScriptPackPathContext,
                       [this]() -> const sf::base::String& { return levelData->packPath; },
                       [this]() -> const PackData& { return *currentPack; },
                       false /* headless */);

    lua.writeVariable("u_log", [](const sf::base::String& mLog) { hg::lo("lua-menu") << mLog << '\n'; });

    lua.writeVariable("u_getDifficultyMult", [] { return 1; });

    lua.writeVariable("u_getSpeedMultDM", [] { return 1; });

    lua.writeVariable("u_getDelayMultDM", [] { return 1; });

    lua.writeVariable("u_getPlayerAngle", [] { return 0; });

    // Unused functions
    for (const auto& un :
         {"u_isKeyPressed",
          "u_isMouseButtonPressed",
          "u_isFastSpinning",
          "u_setPlayerAngle",
          "u_forceIncrement",
          "u_haltTime",
          "u_timelineWait",
          "u_clearWalls",
          "u_setFlashEffect",

          "a_setMusic",
          "a_setMusicSegment",
          "a_setMusicSeconds",
          "a_playSound",
          "a_playPackSound",
          "a_syncMusicToDM",
          "a_setMusicPitch",
          "a_overrideBeepSound",
          "a_overrideIncrementSound",
          "a_overrideSwapSound",
          "a_overrideDeathSound",

          "t_eval",
          "t_kill",
          "t_clear",
          "t_wait",
          "t_waitS",
          "t_waitUntilS",

          "e_eval",
          "e_kill",
          "e_stopTime",
          "e_stopTimeS",
          "e_wait",
          "e_waitS",
          "e_waitUntilS",
          "e_messageAdd",
          "e_messageAddImportant",
          "e_messageAddImportantSilent",
          "e_clearMessages",

          "ct_create",
          "ct_eval",
          "ct_kill",
          "ct_stopTime",
          "ct_stopTimeS",
          "ct_wait",
          "ct_waitS",
          "ct_waitUntilS",

          "l_overrideScore",
          "l_setRotation",
          "l_getRotation",
          "l_getOfficial",

          "s_setStyle",

          "w_wall",
          "w_wallAdj",
          "w_wallAcc",
          "w_wallHModSpeedData",
          "w_wallHModCurveData",

          "steam_unlockAchievement",

          "u_kill",
          "u_eventKill",
          "u_playSound",
          "u_playPackSound",
          "u_setFlashEffect",
          "u_setFlashColor",

          "e_eventStopTime",
          "e_eventStopTimeS",
          "e_eventWait",
          "e_eventWaitS",
          "e_eventWaitUntilS",
          "m_messageAdd",
          "m_messageAddImportant",
          "m_messageAddImportantSilent",
          "m_clearMessages"})
    {
        lua.writeVariable(un, [] {});
    }
}


void MenuGame::update(float mFT)
{
    // Capture frame time for the new UI's animations. `mFT` is in
    // milliseconds (engine convention); convert to seconds.
    ui_dt = mFT / 1000.f;

    // Tick the menu-background level (always) and the in-LevelSelect
    // preview (only when it has a level loaded). `previewMode` keeps
    // both from doing anything they shouldn't (input, scoring, audio).
    if (hgMenuBg != nullptr)
        hgMenuBg->getGame().onUpdate(mFT);
    if (hgPreview != nullptr && !previewLoadedLevelId.empty())
        hgPreview->getGame().onUpdate(mFT);

    // ---- Online client events ---------------------------------------------
    hexagonClient.update();

    const auto showHCEventDialogBox =
        [this](const bool error, const sf::base::String& msg, const sf::base::String& err = "")
    {
        if (!dialogBox.empty())
            return;
        playSoundOverride(error ? "error.ogg" : "select.ogg");
        strBuf.clear();
        strBuf += msg;
        if (!err.empty())
        {
            strBuf += "\n\n";
            strBuf += err;
        }
        strBuf += "\n";
        dialogBoxDelay = 16.f;
        showDialogBox(strBuf);
        setIgnoreAllInputs(1);
    };

    sf::base::Optional<HexagonClient::Event> hcEvent;
    while ((hcEvent = hexagonClient.pollEvent()).hasValue())
    {
        hcEvent->linearMatch( //
            [&](const HexagonClient::EConnectionSuccess&) { showHCEventDialogBox(false, "CONNECTION SUCCESS"); },
            [&](const HexagonClient::EConnectionFailure& e)
        { showHCEventDialogBox(true, "CONNECTION FAILURE", e.error); },
            [&](const HexagonClient::EKicked&) { showHCEventDialogBox(true, "DISCONNECTED FROM SERVER"); },
            [&](const HexagonClient::ERegistrationSuccess&) { showHCEventDialogBox(false, "REGISTRATION SUCCESS"); },
            [&](const HexagonClient::ERegistrationFailure& e)
        { showHCEventDialogBox(true, "REGISTRATION FAILURE", e.error); },
            [&](const HexagonClient::ELoginSuccess&)
        {
            showHCEventDialogBox(false, "LOGIN SUCCESS");
            steamManager.unlock_achievement("a23_login");
        },
            [&](const HexagonClient::ELoginFailure& e) { showHCEventDialogBox(true, "LOGIN FAILURE", e.error); },
            [&](const HexagonClient::ELogoutSuccess&) { showHCEventDialogBox(false, "LOGOUT SUCCESS"); },
            [&](const HexagonClient::ELogoutFailure&) { showHCEventDialogBox(true, "LOGOUT FAILURE"); },
            [&](const HexagonClient::EDeleteAccountSuccess&) { showHCEventDialogBox(false, "DELETE ACCOUNT SUCCESS"); },
            [&](const HexagonClient::EDeleteAccountFailure& e)
        { showHCEventDialogBox(true, "DELETE ACCOUNT FAILURE", e.error); },
            [&](const HexagonClient::EReceivedTopScores& e)
        { leaderboardCache->receivedScores(e.levelValidator, e.scores); },
            [&](const HexagonClient::EReceivedOwnScore& e)
        { leaderboardCache->receivedOwnScore(e.levelValidator, e.score); },
            [&](const HexagonClient::EGameVersionMismatch&)
        { hg::lo("hg::MenuGame::update") << "Client/server game version mismatch, likely not a problem\n"; },
            [&](const HexagonClient::EProtocolVersionMismatch&)
        { showHCEventDialogBox(true, "CLIENT/SERVER PROTOCOL VERSION MISMATCH"); },
            [&](const HexagonClient::EReceivedReplay& e)
        {
            hg::lo("hg::MenuGame::update")
                << "Received replay for validator='" << e.levelValidator << "' ts=" << e.scoreTimestamp << '\n';

            if (!hostCallbacks.watchReplay)
            {
                hg::lo("hg::MenuGame::update") << "[ERROR] hostCallbacks.watchReplay not installed by host\n";
                return;
            }

            // Hand off to `main.cpp`'s wiring -- it knows how to install
            // the replay on the foreground gameplay HG and start playback.
            setMouseCursorVisible(false);
            hostCallbacks.watchReplay(e.replay);
        },
            [&](const HexagonClient::EReplayUnavailable& e)
        {
            hg::lo("hg::MenuGame::update")
                << "Replay unavailable for validator='" << e.levelValidator << "' ts=" << e.scoreTimestamp
                << " reason='" << e.reason << "'\n";
            // No modal dialog -- showed up too aggressively when
            // walking through pre-feature scores. Mark the row so
            // LevelSelect can render an inline "(NO REPLAY)" suffix
            // and update the published snapshot pointer.
            leaderboardCache->markReplayUnavailable(e.levelValidator, e.scoreTimestamp);
            refreshLeaderboardSnapshot();
        }
            //
        );
    }

    if (hostCallbacks.updateRichPresence)
    {
        hostCallbacks.updateRichPresence();
    }

    // ---- Misc per-frame ticks --------------------------------------------
    Joystick::update(Config::getJoystickDeadzone());
    if (dialogBoxDelay > 0.f)
        dialogBoxDelay -= mFT;
    if (Joystick::risingEdge(Joystick::Jid::Screenshot))
        mustTakeScreenshot = true;
}

void MenuGame::refreshCamera()
{
    const float fw{1024.f / getWindowWidth()};
    const float fh{768.f / getWindowHeight()};
    const float fmax{SFML_BASE_MAX(fw, fh)};

    w = getWindowWidth() * fmax;
    h = getWindowHeight() * fmax;

    backgroundCamera = {
        sf::View{.center = sf::Vec2f{0.f, 0.f},
                 .size = {Config::getSizeX() * Config::getZoomFactor(), Config::getSizeY() * Config::getZoomFactor()}}};

    overlayCamera = sf::View{.center = {w / 2.f, h / 2.f}, .size = {w, h}};

    const float scaleFactor{w / 1024.f};
    epilepsyWarning.origin   = epilepsyWarning.getLocalCenter();
    epilepsyWarning.position = {1024 / (2.f / scaleFactor), 768 / 2.f - 50};
    epilepsyWarning.scale    = {0.36f, 0.36f};

    // The new UI is rendered through the overlay view at virtual ~1366×768
    // -- `fourByThree` is still computed here because the dialog box
    // rendering reads it indirectly via `getOverlayView`.
    fourByThree = 10.f * getWindowWidth() / getWindowHeight() < 16;

    txtProf.updateHeight();
}
void MenuGame::renderText(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos)
{
    mText.setString(mStr);
    mText.position = mPos;
    drawOverlay(mText);
}

[[nodiscard]] float MenuGame::getWindowWidth() const noexcept
{
    return window.getRenderWindow().getSize().x;
}

[[nodiscard]] float MenuGame::getWindowHeight() const noexcept
{
    return window.getRenderWindow().getSize().y;
}

[[nodiscard]] ssvs::GameState& MenuGame::getGame() noexcept
{
    return game;
}

void MenuGame::returnToLevelSelection()
{
    setIgnoreAllInputs(1); // otherwise you go back to the main menu

    // The user pressed ESC inside gameplay; the trigger system can fire
    // the same hardcoded ESC binding once more on the menu side as the
    // GameState swaps. Clear `ui_pendingInput` so a stale `escape` edge
    // doesn't immediately pop LevelSelect back to Main on the very first
    // menu frame.
    ui_pendingInput = {};

    // Drop any in-flight replay events. The user just exited gameplay
    // -- if a `EReceivedReplay` arrives now (e.g. a request fired right
    // before they hit ESC) we'd yank them straight back into another
    // replay. Same for `EReplayUnavailable`: surfacing a "REPLAY
    // UNAVAILABLE" dialog right after returning to the menu is jarring
    // and usually for a request the user no longer cares about. The
    // helper preserves login/score events.
    hexagonClient.discardPendingReplayEvents();

    // Reset the watch-replay dedup so the user can ask for a replay
    // again after coming back to LevelSelect.
    lastReplayRequestValidator.clear();
    lastReplayRequestTimestamp = 0;

    // Steer the post-gameplay menu back to the LevelSelect screen.
    if (state != States::SMain)
    {
        changeStateTo(States::SMain);
    }
    ui_app.current = hg::ui::Screen::LevelSelect;
    ui_app.backStack.clear();
    ui_app.backStack.emplaceBack(hg::ui::Screen::Main);
}


void MenuGame::refreshBinds()
{
    // Keyboard-mouse
    for (sf::base::SizeT i{0u}; i < Config::triggerGetters.size(); ++i)
    {
        game.refreshTrigger(Config::triggerGetters[i](), i);

        if (hostCallbacks.triggerRefresh)
        {
            hostCallbacks.triggerRefresh(Config::triggerGetters[i](), i);
        }
    }

    // Joystick
    Config::loadAllJoystickBinds();
}

void MenuGame::setIgnoreAllInputs(const unsigned int presses)
{
    ignoreInputs = presses;

    if (ignoreInputs == 0)
    {
        game.ignoreAllInputs(false);
        Joystick::ignoreAllPresses(false);
        return;
    }

    game.ignoreAllInputs(true);
    Joystick::ignoreAllPresses(true);
}

//*****************************************************
//
// DRAWING
//
//*****************************************************


void MenuGame::draw()
{
    mouseWasPressed = mousePressed;
    mousePressed    = (ignoreInputs == 0) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    if (mustRefresh)
    {
        mustRefresh = false;
        refreshCamera();
    }

    window.clear(sf::Color{0, 0, 0, 255});

    const bool mainOrAbove{state >= States::SMain};

    // Render the menu-background level first so menus draw on top of it.
    // `previewMode` keeps it from clearing the window (we just did) and
    // from drawing a player or any HUD/text overlay.
    //
    // The HG paints into `menuBgTexture`; we then blit that texture to
    // the window through `menuBgBlurShader`, which gaussian-blurs by
    // an amount driven by `ui_app.backDepthAnim` (0 on Main, 1 on a
    // sub-screen). This gives a smooth in/out blur transition without
    // touching the rest of the rendering.
    if (mainOrAbove && hgMenuBg != nullptr)
    {
        // Lazy-allocate / resize the off-screen targets to match the window.
        // Two textures: the source the HG renders into, and a ping-pong
        // intermediate for the horizontal pass of the separable blur.
        const sf::Vec2u winSz = window.getRenderWindow().getSize();
        // Bilinear filtering is critical for the separable blur shader: it
        // samples between texels at fractional offsets and relies on the
        // hardware interpolation to return a weighted blend of two texels.
        // Without `setSmooth(true)` (GL_NEAREST default) those fetches pick
        // a single texel, which makes the kernel resemble several
        // ghost-copies of the image instead of a Gaussian.
        if (winSz.x > 0 && winSz.y > 0 && (!menuBgTexture.hasValue() || menuBgTexture->getSize() != winSz))
        {
            if (auto rt = sf::RenderTexture::create(winSz); rt.hasValue())
            {
                menuBgTexture = SFML_BASE_MOVE(rt);
                menuBgTexture->setSmooth(true);
                hgMenuBg->renderTarget = &*menuBgTexture;
            }
        }
        if (winSz.x > 0 && winSz.y > 0 && (!menuBgBlurTextureH.hasValue() || menuBgBlurTextureH->getSize() != winSz))
        {
            if (auto rt = sf::RenderTexture::create(winSz); rt.hasValue())
            {
                menuBgBlurTextureH = SFML_BASE_MOVE(rt);
                menuBgBlurTextureH->setSmooth(true);
            }
        }

        // Lazy-load the blur shader once. If the file's missing we fall
        // back to a passthrough copy below.
        if (!menuBgBlurShaderLoadAttempted)
        {
            menuBgBlurShaderLoadAttempted = true;
            if (auto sh = sf::Shader::loadFromFile({.fragmentPath = "Assets/menuBackgroundBlur.frag"}); sh.hasValue())
            {
                menuBgBlurShader = SFML_BASE_MOVE(sh);
            }
        }

        if (menuBgTexture.hasValue() && hgMenuBg->renderTarget == &*menuBgTexture)
        {
            menuBgTexture->clear(sf::Color::Black);
            hgMenuBg->getGame().onDraw();
            menuBgTexture->display();

            // Helper for one separable pass: blits `srcTex` onto `dstTarget`
            // running the shader with the chosen direction. When the shader
            // is missing this becomes a plain copy.
            const auto runPass = [&](const sf::Texture& srcTex, sf::RenderTarget& dstTarget, sf::Glsl::Vec2 direction)
            {
                const sf::Vec2u  sz = srcTex.getSize();
                sf::RenderStates rs{};
                rs.texture = &srcTex;
                rs.view    = sf::View::fromScreenSize(sz.to<sf::Vec2f>());

                if (menuBgBlurShader.hasValue())
                {
                    if (auto loc = menuBgBlurShader->getUniformLocation("u_resolution"); loc.hasValue())
                    {
                        menuBgBlurShader->setUniform(*loc,
                                                     sf::Glsl::Vec2{static_cast<float>(sz.x), static_cast<float>(sz.y)});
                    }
                    if (auto loc = menuBgBlurShader->getUniformLocation("u_blur"); loc.hasValue())
                    {
                        menuBgBlurShader->setUniform(*loc, ui_app.backDepthAnim);
                    }
                    if (auto loc = menuBgBlurShader->getUniformLocation("u_direction"); loc.hasValue())
                    {
                        menuBgBlurShader->setUniform(*loc, direction);
                    }
                    rs.shader = &*menuBgBlurShader;
                }

                dstTarget.draw(
                    sf::Sprite{
                        .position    = {0.f, 0.f},
                        .textureRect = {{0.f, 0.f}, sz.to<sf::Vec2f>()},
                    },
                    rs);
            };

            if (menuBgBlurTextureH.hasValue())
            {
                // Pass 1: horizontal blur into the intermediate.
                menuBgBlurTextureH->clear(sf::Color::Black);
                runPass(menuBgTexture->getTexture(), *menuBgBlurTextureH, {1.f, 0.f});
                menuBgBlurTextureH->display();

                // Pass 2: vertical blur, into the window.
                runPass(menuBgBlurTextureH->getTexture(), window.getRenderWindow(), {0.f, 1.f});
            }
            else
            {
                // Intermediate allocation failed -- fall back to a single
                // pass so the menu still draws (just no blur).
                runPass(menuBgTexture->getTexture(), window.getRenderWindow(), {0.f, 0.f});
            }
        }
        else
        {
            // Texture allocation failed -- fall back to direct render so
            // the menu still has a backdrop, just without the blur.
            hgMenuBg->getGame().onDraw();
        }

        window.draw(sf::RectangleShapeData{
            .fillColor = sf::Color{0, 0, 0, 25},
            .size      = window.getRenderWindow().getSize().toVec2f(),
        });
    }

    // Refresh the level-preview render texture: hgPreview drew into its
    // own `sf::RenderTexture` so the menu can later paint it as a sprite.
    if (previewTexture.hasValue() && hgPreview != nullptr && !previewLoadedLevelId.empty())
    {
        previewTexture->clear(sf::Color::Black);
        hgPreview->getGame().onDraw();

        previewTexture->display();
    }

    // The legacy "CURRENT PROFILE: <name>" line has been removed (single-
    // profile model -- see `playLocally`). Missing-dependency warnings
    // still surface if any packs need attention.
    if (mainOrAbove)
    {
        const auto& pwmd = assets.getPackIdsWithMissingDependencies();
        if (!pwmd.empty())
        {
            strBuf.clear();
            strBuf += "WARNING - PACKS WITH MISSING DEPENDENCIES:";
            for (const auto& p : pwmd)
            {
                strBuf += "\n    ";
                strBuf += p;
            }
            strBuf += "\nFORGOT TO DOWNLOAD THEM FROM THE STEAM WORKSHOP?";
            renderText(strBuf, txtSelectionSmall.font, sf::Vec2f{20.f, titleBar.getGlobalBottom() + 8});
        }
    }

    switch (state)
    {
        case States::EpilepsyWarning:
            drawOverlay(epilepsyWarning, sf::RenderStates{.texture = &txEpilepsyWarning});
            renderText("PRESS ANY KEY OR BUTTON TO CONTINUE", txtProf.font, {txtProf.height, h - txtProf.height * 2.7f + 5.f});
            return;

        case States::SMain:
            // The new immediate-mode UI owns the main screen. Sub-screens
            // (LevelSelect, Options, Workshop, Online) are pushed via
            // `app.current` without a state transition, so they all end
            // up here too. Only the online-status bar still ships from
            // the host outside the new UI.
            drawNewMainMenu();
            drawOnlineStatus();
            break;

        default:
            break;
    }

    if (mustTakeScreenshot)
    {
        window.saveScreenshot("screenshot.png");
        mustTakeScreenshot = false;
    }

    // The dialog box is now drawn inside `drawNewMainMenu` -- routed
    // through the UI composite texture so its magenta-sentinel frame
    // gets the same accent gradient as the rest of the new UI. The
    // direct-to-window draw that used to live here has been removed.
}

void MenuGame::drawOnlineStatus()
{
    const float onlineStatusScaling = 1.5f;
    const float scaling             = onlineStatusScaling / Config::getZoomFactor();
    const float padding             = 3.f * onlineStatusScaling;

    txtOnlineStatus.scale = {(10.f * scaling) / static_cast<float>(txtOnlineStatus.getCharacterSize()),
                             (10.f * scaling) / static_cast<float>(txtOnlineStatus.getCharacterSize())};
    txtOnlineStatus.setFillColor(sf::Color::White);

    const HexagonClient::State state = hexagonClient.getState();

    const auto [stateGood, stateString] = [&]() -> std::tuple<bool, sf::base::String>
    {
        switch (state)
        {
            case HexagonClient::State::Disconnected:
            {
                return {false, "DISCONNECTED"};
            }

            case HexagonClient::State::InitError:
            {
                return {false, "CLIENT ERROR"};
            }

            case HexagonClient::State::Connecting:
            {
                return {false, "CONNECTING"};
            }

            case HexagonClient::State::ConnectionError:
            {
                return {false, "CONNECTION ERROR"};
            }

            case HexagonClient::State::Connected:
            {
                if (hexagonClient.hasRTKeys())
                {
                    return {true, "CONNECTED, PLEASE LOG IN"};
                }
                else
                {
                    return {false, "CONNECTED, BUT KEY EXCHANGE FAILED"};
                }
            }

            case HexagonClient::State::LoggedIn:
                [[fallthrough]];
            case HexagonClient::State::LoggedIn_Ready:
            {
                if (Config::getSaveLastLoginUsername() && hexagonClient.getLoginName().hasValue())
                {
                    // Save last login username for quicker login next time.

                    Config::setLastLoginUsername(hexagonClient.getLoginName().value());
                }

                return {true, "LOGGED IN AS " + hexagonClient.getLoginName().valueOr("UNKNOWN")};
            }
        }

        return {false, "UNKNOWN"};
    }();

    txtOnlineStatus.setString("ABC,:::'");
    const auto  txtHeight   = txtOnlineStatus.getGlobalHeight();
    const float spriteScale = (txtHeight + padding * 2.f) / 64.f;

    txtOnlineStatus.setString(stateString);

    if (stateGood)
    {
        txSOnline = &assets.getTexture("onlineIcon.png");
    }
    else
    {
        txSOnline = &assets.getTexture("onlineIconFail.png");
    }

    sOnline.textureRect = txSOnline->getRect();
    sOnline.scale       = {spriteScale, spriteScale};
    sOnline.origin      = sOnline.getLocalBottomLeft();
    sOnline.position    = {0.f + padding, getWindowHeight() - padding};

    rsOnlineStatus.setSize({txtOnlineStatus.getGlobalWidth() + padding * 4.f, txtHeight + padding * 2.f});
    rsOnlineStatus.setFillColor(sf::Color::Black);
    rsOnlineStatus.origin   = rsOnlineStatus.getLocalBottomLeft();
    rsOnlineStatus.position = {sOnline.getGlobalRight() + padding, sOnline.position.y};

    txtOnlineStatus.origin   = txtOnlineStatus.getLocalCenterLeft();
    txtOnlineStatus.position = {rsOnlineStatus.getGlobalLeft() + padding * 2.f, rsOnlineStatus.getGlobalCenter().y};

    drawScreen(sOnline, sf::RenderStates{.texture = txSOnline});
    drawScreen(rsOnlineStatus);
    drawScreen(txtOnlineStatus);
}

void MenuGame::showDialogBox(const sf::base::String& msg)
{
    dialogBox.create(msg, 22 /* charSize */, 12.f /* frameSize */, DBoxDraw::center);
}

void MenuGame::showInputDialogBox(const sf::base::String& msg)
{
    dialogBox.createInput(msg, 22 /* charSize */, 12.f /* frameSize */, DBoxDraw::center);
}

void MenuGame::showInputDialogBoxNice(const sf::base::String& title,
                                      const sf::base::String& inputType,
                                      const sf::base::String& extra)
{
    showInputDialogBoxNiceWithDefault(title, inputType, "" /* default */, extra);
}

void MenuGame::showInputDialogBoxNiceWithDefault(const sf::base::String& title,
                                                 const sf::base::String& inputType,
                                                 const sf::base::String& def,
                                                 const sf::base::String& extra)
{
    strBuf.clear();

    if (extra.empty())
    {
        strBuf += Utils::concat(title,
                                "\n\nPLEASE INSERT ",
                                inputType,
                                "\n\nCONFIRM WITH [ENTER]\nCANCEL WITH [ESCAPE]\n");
    }
    else
    {
        strBuf += Utils::concat(title,
                                "\n\nPLEASE INSERT ",
                                inputType,
                                "\n\n",
                                extra,
                                "\n\nCONFIRM WITH [ENTER]\nCANCEL WITH [ESCAPE]\n");
    }

    showInputDialogBox(strBuf);
    dialogBox.getInput() = def;
}

void MenuGame::openLoginDialogBoxAndStartLoginProcess()
{
    SSVOH_ASSERT(dialogInputState == DialogInputState::Nothing);

    dialogInputState = DialogInputState::Login_EnteringUsername;

    const sf::base::String defaultLoginUsername = Config::getSaveLastLoginUsername() ? Config::getLastLoginUsername() : "";

    showInputDialogBoxNiceWithDefault("LOGIN", "USERNAME", defaultLoginUsername);
}

} // namespace hg
