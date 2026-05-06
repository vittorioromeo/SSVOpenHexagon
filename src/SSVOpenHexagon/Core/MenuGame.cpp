// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"
#include "SSVOpenHexagon/Core/BindControl.hpp"
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
#include "SSVOpenHexagon/Data/LoadInfo.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/PackInfo.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/GameSystem/GameState.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Input/Bind.hpp"
#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/InputState.hpp"
#include "SSVOpenHexagon/Input/Manager.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"
#include "SSVOpenHexagon/Input/Utils.hpp"
#include "SSVOpenHexagon/MenuSystem/Items/GoBack.hpp"
#include "SSVOpenHexagon/MenuSystem/Items/Goto.hpp"
#include "SSVOpenHexagon/MenuSystem/Items/Single.hpp"
#include "SSVOpenHexagon/MenuSystem/Items/Slider.hpp"
#include "SSVOpenHexagon/MenuSystem/Items/Toggle.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/Category.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/Menu.hpp"
#include "SSVOpenHexagon/MenuSystem/SSVMenuSystem.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Io.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/Utils/Casts.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/FontHeight.hpp"
#include "SSVOpenHexagon/Utils/Geometry.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/Math.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"
#include "SSVOpenHexagon/Utils/Timestamp.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVUtils/Core/Common/Casts.hpp"
#include "SSVUtils/Core/String/Utils.hpp"
#include "SSVUtils/Core/Utils/Math.hpp"
#include "SSVUtils/Core/Utils/Rnd.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"

#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/Array.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringStreamOp.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <SSVUtils/Core/String/ToStr.hpp>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include <cstdio>
#include <cstdlib>


namespace hg
{

[[nodiscard]] static bool anyItemEnabled(const ssvms::Menu& menu)
{
    for (const auto& i : menu.getItems())
    {
        if (i->isEnabled())
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] static bool scrollToEnabledMenuItem(ssvms::Menu* menu)
{
    if (menu == nullptr)
    {
        return false;
    }

    // Scroll to a menu item that is enabled
    menu->update();

    if (!anyItemEnabled(*menu))
    {
        return false;
    }

    while (!menu->getItem().isEnabled())
    {
        menu->next();
    }

    return true;
}

[[nodiscard]] static float getTextScaleForCharacterSize(const sf::Text& text, const unsigned int characterSize)
{
    return static_cast<float>(characterSize) / static_cast<float>(text.getCharacterSize());
}

static void setVisualCharacterSize(sf::Text& text, const unsigned int characterSize)
{
    const float scale = getTextScaleForCharacterSize(text, characterSize);
    text.scale        = {scale, scale};
}

void MenuGame::MenuFont::updateHeight()
{
    height = hg::Utils::getFontHeight(font);
}

[[nodiscard]] bool MenuGame::isEnteringText() const noexcept
{
    return state <= States::ETLPNewBoot || state == States::ETLPNew;
}


[[nodiscard]] ssvms::Menu* MenuGame::getCurrentMenu() noexcept
{
    switch (state)
    {
        case States::SMain:
            return &mainMenu;
        case States::MOpts:
            return &optionsMenu;
        case States::MOnline:
            return &onlineMenu;
        case States::SLPSelectBoot:
        case States::SLPSelect:
            return &profileSelectionMenu;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
        default:
            return nullptr;
#pragma GCC diagnostic pop
    }
}

[[nodiscard]] bool MenuGame::isInMenu() noexcept
{
    return getCurrentMenu() != nullptr;
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
    txCreditsBar1{assets.getTexture("creditsBar1.png")},
    txCreditsBar2{&assets.getTexture("creditsBar2.png")},
    txEpilepsyWarning{assets.getTexture("epilepsyWarning.png")},
    titleBar{.textureRect = txTitleBar.getRect()},
    creditsBar1{.textureRect = txCreditsBar1.getRect()},
    creditsBar2{.textureRect = txCreditsBar2->getRect()},
    epilepsyWarning{.textureRect = txEpilepsyWarning.getRect()},
    txSOnline{&assets.getTexture("onlineIconFail.png")},
    sOnline{.textureRect = txSOnline->getRect()},
    rsOnlineStatus{{.size = {128.f, 32.f}}},
    txtOnlineStatus{openSquare, {.string = "", .characterSize = 24}},
    enteredChars{},
    backgroundCamera{
        sf::View{.center = sf::Vec2f{0.f, 0.f},
                 .size = {Config::getSizeX() * Config::getZoomFactor(), Config::getSizeY() * Config::getZoomFactor()}}},
    overlayCamera{
        sf::View{.center = {Config::getWidth() / 2.f, Config::getHeight() * Config::getZoomFactor() / 2.f},
                 .size = {Config::getWidth() * Config::getZoomFactor(), Config::getHeight() * Config::getZoomFactor()}}},
    mustRefresh{false},
    wasFocusHeld{false},
    focusHeld{false},
    wheelProgress{0.f},
    touchDelay{0.f},
    state{States::EpilepsyWarning},
    packChangeDirection{0},
    levelStatus{Config::getMusicSpeedDMSync(), Config::getSpawnDistance()},
    ignoreInputs{0},
    w{0.f},
    h{0.f},
    scrollbarOffset{0},
    fourByThree{false},
    welcomeMenu{},
    mainMenu{},
    optionsMenu{},
    onlineMenu{},
    profileSelectionMenu{},
    levelData{},
    styleData{},
    txtVersion{{openSquare, {.string = "", .characterSize = 40}}},
    txtProf{{openSquare, {.string = "", .characterSize = 18}}},
    // For the loading screen
    txtLoadBig{{openSquare, {.string = "", .characterSize = 70}}},
    txtLoadSmall{{openSquareBold, {.string = "", .characterSize = 24}}},
    txtRandomTip{{openSquare, {.string = "", .characterSize = 32}}},
    // For the Main Menu
    txtMenuBig{{openSquare, {.string = "", .characterSize = 36}}},
    txtMenuSmall{{openSquare, {.string = "", .characterSize = 24}}},
    txtMenuTiny{{openSquare, {.string = "", .characterSize = 14}}},
    txtProfile{{openSquare, {.string = "", .characterSize = 32}}},
    txtInstructionsBig{{openSquare, {.string = "", .characterSize = 46}}},
    txtInstructionsMedium{{openSquare, {.string = ""}}},
    txtInstructionsSmall{{openSquare, {.string = "", .characterSize = 20}}},
    // Manual Input
    txtEnteringText{{openSquare, {.string = "", .characterSize = 54}}},
    // For the Level Selection Screen
    txtSelectionBig{{openSquareBold, {.string = "", .characterSize = 28}}},
    txtSelectionMedium{{openSquareBold, {.string = "", .characterSize = 19}}},
    txtSelectionSmall{{openSquare, {.string = "", .characterSize = 14}}},
    txtSelectionScore{{openSquare, {.string = "", .characterSize = 28}}},
    txtSelectionRanked{{openSquareBold, {.string = "", .characterSize = 10}}},
    menuTextColor{},
    menuQuadColor{},
    menuSelectionColor{},
    dialogBoxTextColor{},
    menuBackgroundTris{},
    menuQuads{},
    loadInfo(mAssets.getLoadResults()),
    randomTip{},
    hexagonRotation{0.f},
    menuHalfHeight{0.f},
    enteringTextOffset{0.f},
    isLevelFavorite{false},
    favoriteLevelDataIds{},
    lvlSlct{},
    favSlct{.levelDataIds = &favoriteLevelDataIds, .isFavorites = true},
    lvlDrawer{&lvlSlct}
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
            // The legacy LoadingScreen state has been removed — the app
            // now boots straight into EpilepsyWarning, and any key press
            // here goes directly to the main menu.
            mainMenu.getItems()[0]->getOffset() = maxOffset;
            mainMenu.getCategory().getOffset()  = fourByThree ? 280.f : 400.f;

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
            if (e->unicode < 128)
            {
                enteredChars.emplaceBack(ssvu::toNum<char>(e->unicode));
            }

            // Feed printable ASCII into the new UI's typedChars buffer so
            // text fields can read it. typedChars is a NUL-terminated buffer
            // of size 8 (max 7 chars/frame); excess bytes in a single frame
            // are dropped.
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

            if (!dialogBox.empty() && dialogBox.isInputBox())
            {
                sf::base::String& input = dialogBox.getInput();

                if (e->unicode >= 32 && e->unicode < 127)
                {
                    if (input.size() < 32)
                    {
                        playSoundOverride("beep.ogg");
                        input.pushBack(static_cast<char>(e->unicode));
                    }
                }
            }
        }
        else if (const auto* e = event.getIf<sf::Event::KeyPressed>())
        {
            if (window.hasFocus())
            {
                setMouseCursorVisible(false);
            }

            // Feed backspace edges into the new UI so text fields can erase.
            if (newUIActiveForCurrentState() && e->code == sf::Keyboard::Key::Backspace)
            {
                ui_pendingInput.backspace = true;
            }

            if (!dialogBox.empty() && dialogBox.isInputBox())
            {
                sf::base::String& input = dialogBox.getInput();

                if (e->code == sf::Keyboard::Key::Backspace)
                {
                    if (!input.empty())
                    {
                        playSoundOverride("beep.ogg");
                        input.erase(input.size() - 1, 1);
                    }
                }
            }
        }
        else if (const auto* e = event.getIf<sf::Event::MouseMoved>())
        {
            const sf::Vec2i mouseMoveVec   = {e->position.x, e->position.y};
            const sf::Vec2i mouseMoveDelta = lastMouseMovedPosition - mouseMoveVec;

            lastMouseMovedPosition = mouseMoveVec;

            const bool actuallyMoved = (mouseMoveDelta.x != 0) || (mouseMoveDelta.y != 0);

            if (window.hasFocus() && actuallyMoved)
            {
                setMouseCursorVisible(true);
            }
        }
        else if (const auto* e = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (window.hasFocus())
            {
                setMouseCursorVisible(true);
            }

            // Disable scroll while assigning a bind
            if (state == States::MOpts)
            {
                const auto* const bc{dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem())};
                if (bc != nullptr && bc->isWaitingForBind())
                {
                    return;
                }
            }

            if (state == States::LevelSelection)
            {
                if (focusHeld)
                {
                    changePackQuick(e->delta > 0 ? -1 : 1);
                }
                else if (lvlDrawer != nullptr)
                {
                    lvlDrawer->YScrollTo += e->delta * 48.f;

                    if (lvlDrawer->YScrollTo > 0)
                    {
                        lvlDrawer->YScrollTo = 0;
                    }
                    else if (lvlDrawer->YScrollTo < -4000)
                    {
                        // Why...
                        steamManager.unlock_achievement("a35_eagerformore");
                        playSoundOverride("error.ogg");
                        lvlDrawer->YScrollTo = 0;
                    }
                }

                return;
            }

            wheelProgress += e->delta;
            if (wheelProgress > 1.f)
            {
                wheelProgress = 0.f;
                upAction();
            }
            else if (wheelProgress < -1.f)
            {
                wheelProgress = 0.f;
                downAction();
            }
        }
        else if (const auto* e = event.getIf<sf::Event::KeyReleased>())
        {
            // don't do anything if inputs are being processed as usual
            if (ignoreInputs == 0)
            {
                return;
            }

            // Scenario one: epilepsy warning is being drawn and user
            // must close it with any key press
            if (state == States::EpilepsyWarning || state == States::LoadingScreen)
            {
                checkCloseBootScreens();
                return;
            }

            // Scenario two: actions are blocked cause a dialog box is open
            if (dialogBoxDelay > 0.f)
            {
                return;
            }

            const sf::Keyboard::Key key{e->code};
            if (!dialogBox.empty())
            {
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

            // Scenario three: actions are blocked cause we are using a
            // BindControl menu item
            if (getCurrentMenu() != nullptr && key == sf::Keyboard::Key::Escape)
            {
                getCurrentMenu()->getItem().exec(); // turn off bind inputting
                setIgnoreAllInputs(0);
                playSoundOverride("beep.ogg");
                return;
            }

            if ((--ignoreInputs) == 0)
            {
                if (getCurrentMenu() == nullptr || state != States::MOpts)
                {
                    setIgnoreAllInputs(0);
                    return;
                }

                auto* const bc{dynamic_cast<KeyboardBindControl*>(&getCurrentMenu()->getItem())};

                // don't try assigning a keyboard key to a controller bind
                if (bc == nullptr)
                {
                    playSoundOverride("error.ogg");
                    ignoreInputs = 1;
                    return;
                }

                // If user tries to bind a key that is already hardcoded ignore
                // the input and notify it of what has happened.
                if (!bc->newKeyboardBind(key))
                {
                    playSoundOverride("error.ogg");
                    setIgnoreAllInputs(1);
                    showDialogBox(
                        "THE KEY YOU ARE TRYING TO ASSIGN TO THIS ACTION\n"
                        "IS ALREADY BOUND TO IT BY DEFAULT,\n"
                        "YOUR LAST INPUT HAS BEEN IGNORED\n\n"
                        "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
                    return;
                }

                playSoundOverride("select.ogg");
                setIgnoreAllInputs(0);
                touchDelay = 10.f;
            }
        }
        else if (const auto* e = event.getIf<sf::Event::MouseButtonReleased>())
        {
            if (ignoreInputs == 0)
            {
                return;
            }

            if (state == States::EpilepsyWarning || state == States::LoadingScreen)
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
                if (dialogBox.getKeyToClose() != sf::Keyboard::Key::Unknown)
                {
                    return;
                }
                --ignoreInputs;
                checkCloseDialogBox();
                return;
            }

            if ((--ignoreInputs) == 0)
            {
                if (getCurrentMenu() == nullptr || state != States::MOpts)
                {
                    setIgnoreAllInputs(0);
                    return;
                }

                auto* const bc{dynamic_cast<KeyboardBindControl*>(&getCurrentMenu()->getItem())};

                // don't try assigning a keyboard key to a controller bind
                if (bc == nullptr)
                {
                    playSoundOverride("error.ogg");
                    ignoreInputs = 1;
                    return;
                }

                bc->newKeyboardBind(e->button);
                playSoundOverride("select.ogg");
                setIgnoreAllInputs(0);
                touchDelay = 10.f;
            }
        }
        else if (const auto* e = event.getIf<sf::Event::JoystickButtonReleased>())
        {
            if (ignoreInputs == 0)
            {
                return;
            }

            if (state == States::EpilepsyWarning || state == States::LoadingScreen)
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
                if (dialogBox.getKeyToClose() != sf::Keyboard::Key::Unknown)
                {
                    return;
                }
                --ignoreInputs;
                checkCloseDialogBox();
                return;
            }

            if ((--ignoreInputs) == 0)
            {
                if (getCurrentMenu() == nullptr || state != States::MOpts)
                {
                    setIgnoreAllInputs(0);
                    return;
                }

                auto* const bc{dynamic_cast<JoystickBindControl*>(&getCurrentMenu()->getItem())};

                // don't try assigning a controller button to a keyboard bind
                if (bc == nullptr)
                {
                    playSoundOverride("error.ogg");
                    ignoreInputs = 1;
                    return;
                }

                bc->newJoystickBind(e->button);
                setIgnoreAllInputs(0);
                playSoundOverride("select.ogg");
                touchDelay = 10.f;
            }
        }
    };

    // To close the load results with any key
    setIgnoreAllInputs(1);

    window.onRecreation += [this]
    {
        refreshCamera();
        adjustLevelsOffset();
        adjustMenuOffset(true);
        resetNamesScrolls();
        mustRefresh = true;
    };

    initMenus();
    initInput();
    initLua();
    initNewUIServices();

    //--------------------------------
    // Main menu background

    {
        const auto [randomPack, randomLevel] = pickRandomMainMenuBackgroundStyle();

        const sf::base::String& randomPackId = getNthSelectablePackInfo(randomPack).id;

        lvlSlct.levelDataIds = &assets.getLevelIdsByPack(randomPackId);
        setIndex(randomLevel);
    }

    // Setup for the loading menu
    static constexpr sf::base::Array<sf::base::Array<std::string_view, 2>, 4> tips{
        {{"HOLDING FOCUS WHILE CHANGING PACK", "SKIPS THE SWITCH ANIMATION"},
         {"REMEMBER TO TAKE BREAKS", "OPEN HEXAGON IS AN INTENSE GAME"},
         {"EXPERIMENT USING SWAP", "IT MAY SAVE YOUR LIFE"},
         {"IF A LEVEL IS TOO CHALLENGING", "PRACTICE IT AT A LOWER DIFFICULTY"}}};
    randomTip = tips[ssvu::getRndI(0, tips.size())];

    // Set size of the level offsets vector to the minimum required
    unsigned int maxSize{0}, packSize;
    for (sf::base::SizeT i{0}; i < getSelectablePackInfosSize(); ++i)
    {
        const sf::base::String& packId = getNthSelectablePackInfo(i).id;

        if (!assets.packHasLevels(packId))
        {
            continue;
        }

        packSize = assets.getLevelIdsByPack(packId).size();
        if (packSize > maxSize)
        {
            maxSize = packSize;
        }
    }
    lvlSlct.lvlOffsets.resize(maxSize);
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
    if (!useNewUI)
    {
        return false;
    }
    // Phase 0: only the main menu is migrated.
    return state == States::SMain;
}

void MenuGame::initNewUIServices()
{
    // Wire the new UI's action callbacks to existing MenuGame helpers. Once
    // every screen is migrated, these can be replaced with direct calls and
    // `MenuGame` itself can be deleted.
    ui_services.onExit          = [this] { window.stop(); };
    ui_services.onPlayRequested = [this]
    {
        // PLAY is handled by the new Level Select screen; this fallback only
        // exists for transition periods or if the new path is bypassed.
        if (firstLevelSelection)
        {
            lvlSlct.packIdx      = 0;
            diffMultIdx          = 0;
            lvlSlct.levelDataIds = &assets.getLevelIdsByPack(getNthSelectablePackInfo(0).id);
            setIndex(0);
        }
        changeStateTo(States::LevelSelection);
        playSoundOverride("select.ogg");
    };
    ui_services.onOptionsRequested = [this] { changeStateTo(States::MOpts); };
    ui_services.onOnlineRequested  = [this] { changeStateTo(States::MOnline); };

    ui_services.onOnlineConnect    = [this] { hexagonClient.connect(); };
    ui_services.onOnlineDisconnect = [this] { hexagonClient.disconnect(); };
    ui_services.onOnlineLogout     = [this] { hexagonClient.tryLogoutFromServer(); };
    ui_services.onOnlineLogin      = [this]
    {
        // Reuse the legacy login dialog overlay — it already runs on top
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
    ui_services.onWorkshopRequested = [this]
    {
        // Phase 3 will replace this with the Workshop browse screen. Until
        // then, surface a dialog so the entry point exists in the UI.
        showDialogBox("WORKSHOP BROWSING IS COMING IN A FUTURE UPDATE.\nPRESS ANY KEY TO CONTINUE.");
    };

    ui_services.onStartLevel = [this](const sf::base::String& levelId, float difficultyMult)
    {
        // Drive the legacy gameplay-launch path with the level + difficulty
        // selected from the new UI. We populate the legacy menu's selection
        // state (`lvlSlct`/`lvlDrawer`/`levelData`/`diffMultIdx`) then call
        // `playSelectedLevel()`, which is the same call site the legacy
        // LevelSelection screen uses on OK. `levelId` is the pack-prefixed
        // asset key as stored in `levelDataIdsByPack`, NOT the bare
        // `LevelData::id` (the latter would not pass `isValidLevelId`).
        if (!assets.isValidLevelId(levelId))
        {
            return;
        }
        const LevelData& ld = assets.getLevelData(levelId);

        // The legacy code reads through `lvlDrawer`. Force it to the regular
        // (non-favorites) drawer so we drive the pack we just resolved.
        lvlDrawer = &lvlSlct;

        // Find the pack index in the selectable pack list.
        int packIdx = 0;
        for (sf::base::SizeT i = 0; i < getSelectablePackInfosSize(); ++i)
        {
            if (getNthSelectablePackInfo(static_cast<int>(i)).id == ld.packId)
            {
                packIdx = static_cast<int>(i);
                break;
            }
        }

        lvlSlct.packIdx      = packIdx;
        lvlSlct.levelDataIds = &assets.getLevelIdsByPack(ld.packId);

        // Find the level index within the pack. `setIndex` populates
        // `levelData`, `currentPack`, and `styleData` from the resolved id.
        for (sf::base::SizeT i = 0; i < lvlSlct.levelDataIds->size(); ++i)
        {
            if ((*lvlSlct.levelDataIds)[i] == levelId)
            {
                setIndex(static_cast<int>(i));
                break;
            }
        }

        // Find the difficulty index.
        diffMultIdx = 0;
        for (sf::base::SizeT i = 0; i < ld.difficultyMults.size(); ++i)
        {
            if (ld.difficultyMults[i] == difficultyMult)
            {
                diffMultIdx = static_cast<int>(i);
                break;
            }
        }

        // Same call site the legacy menu uses on OK from level selection.
        resetNamesScrolls();
        playSelectedLevel();
    };

    ui_services.onPreviewLevel = [this](const sf::base::String& levelId)
    {
        // Drive the legacy backdrop (style colors, pack info) when the new
        // UI's LevelSelect cursor moves. `setIndex` populates `levelData`,
        // `currentPack`, and `styleData`, which the legacy `drawGraphics`
        // reads each frame to render the menu background.
        if (!assets.isValidLevelId(levelId))
            return;
        const LevelData& ld = assets.getLevelData(levelId);

        lvlDrawer = &lvlSlct;

        int packIdx = 0;
        for (sf::base::SizeT i = 0; i < getSelectablePackInfosSize(); ++i)
        {
            if (getNthSelectablePackInfo(static_cast<int>(i)).id == ld.packId)
            {
                packIdx = static_cast<int>(i);
                break;
            }
        }
        lvlSlct.packIdx      = packIdx;
        lvlSlct.levelDataIds = &assets.getLevelIdsByPack(ld.packId);
        for (sf::base::SizeT i = 0; i < lvlSlct.levelDataIds->size(); ++i)
        {
            if ((*lvlSlct.levelDataIds)[i] == levelId)
            {
                setIndex(static_cast<int>(i));
                break;
            }
        }

        // Reload the preview HG with the newly-selected level so its
        // walls/style/3D animate inside the LevelSelect right-side preview
        // texture. Skipped if we're already showing this level.
        if (hgPreview != nullptr && previewLoadedLevelId != levelId)
        {
            hgPreview->newGame(ld.packId,
                               levelId,
                               /*firstPlay=*/true,
                               /*difficultyMult=*/1.f,
                               /*executeLastReplay=*/false);
            hgPreview->setMustStart(true);
            previewLoadedLevelId = levelId;
        }
    };

    ui_services.playSound = [this](sf::base::StringView s)
    {
        // Best-effort; small stack buffer keeps us null-terminated.
        char       buf[64] = {};
        const auto n       = s.size() < (sizeof(buf) - 1) ? s.size() : (sizeof(buf) - 1);
        for (decltype(s.size()) i = 0; i < n; ++i)
        {
            buf[i] = s.data()[i];
        }
        playSoundOverride(buf);
    };

    ui_services.assets       = &assets;
    ui_services.steamManager = &steamManager;
    // `currentProfile` is refreshed each frame in `drawNewMainMenu` because
    // no profile is selected at construction time (it's chosen later via
    // `SLPSelectBoot`/`SLPSelect`).
    ui_services.currentProfile = nullptr;
}

void MenuGame::applyLevelThemeToContext(hg::ui::Context& /*ctx*/) const
{
    // Intentionally empty. The new UI uses a fixed white/black/magenta
    // palette (see `Context` defaults) — the magenta accent is a sentinel
    // replaced by an animated gradient in the post-process shader pass.
    // Per-level theming would fight the gradient and isn't wanted here.
}

void MenuGame::setMenuPreviewGames(HexagonGame*     menuBackground,
                                   HexagonGame*     preview,
                                   sf::base::String menuBackgroundPackId,
                                   sf::base::String menuBackgroundLevelId)
{
    hgMenuBg  = menuBackground;
    hgPreview = preview;

    // Boot the menu-background level once. Skipped silently when the
    // configured pack/level isn't installed — the menu still works, just
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
        // spawning immediately. `start()` is private — flagging via
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
            previewTexture          = SSVOH_MOVE(rt);
            hgPreview->renderTarget = &*previewTexture;
        }
    }
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
                ui_app.workshop.items         = std::move(evt->queryResults);
                ui_app.workshop.queryInFlight = false;
                ui_app.workshop.totalMatching = evt->totalMatching;
                std::snprintf(ui_app.workshop.statusMessage,
                              sizeof(ui_app.workshop.statusMessage),
                              "%zu items received",
                              static_cast<std::size_t>(ui_app.workshop.items.size()));
                break;

            case EK::DetailsComplete:
                // On-demand lookup (e.g. dep titles). Don't touch `items` —
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
                    if (auto tex = sf::Texture::loadFromMemory(evt->previewBytes.data(),
                                                               evt->previewBytes.size());
                        tex.hasValue())
                    {
                        tex->setSmooth(true);
                        slot = SSVOH_MOVE(tex);
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
    // chosen yet) — handle that case so the new UI doesn't crash on first
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

    // Drain any pending Steam Workshop events before drawing — hot-installs
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
                uiCompositeTexture = SSVOH_MOVE(rt);
            }
        }
    }

    // Lazy-load the post-process shader once. If the file is missing we
    // still draw the UI — the gradient pass just becomes a passthrough.
    if (!menuAccentShaderLoadAttempted)
    {
        menuAccentShaderLoadAttempted = true;
        if (auto sh = sf::Shader::loadFromFile({.fragmentPath = "Assets/menuAccentGradient.frag"}); sh.hasValue())
        {
            menuAccentShader = SSVOH_MOVE(sh);
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
    // vertical centering via `rowTextY`. Cheap — measures one glyph.
    hg::ui::recomputeTextMetrics(ctx);

    // Mouse button state. Position is mapped *below*, after `renderStates`
    // has been set up — so widget hit-testing matches the transformed
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

    // Map raw pixel mouse → UI layout space, accounting for the view +
    // transform set on `renderStates`.
    const sf::Vec2f mousePixelPos = sf::Mouse::getPosition(window.getRenderWindow()).to<sf::Vec2f>();
    ctx.input.mousePos            = hg::ui::screenToUI(ctx, mousePixelPos);

    applyLevelThemeToContext(ctx);

    // Clear the off-screen UI texture to fully transparent black so the
    // background level (rendered earlier into the window) shows through
    // wherever the UI doesn't draw.
    if (uiCompositeTexture.hasValue() && ctx.target == &*uiCompositeTexture)
    {
        uiCompositeTexture->clear(sf::Color{0, 0, 0, 0});
    }

    hg::ui::drawCurrentScreen(ctx, ui_app, ui_services);

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
    // edges and survive — they're rebuilt next frame anyway.
    ui_pendingInput = {};

    if (ui_app.exitRequested)
    {
        window.stop();
    }
}

// ----------------------------------------------------------------------------

void MenuGame::initAssets()
{
    for (const auto& t :
         {"titleBar.png",
          "creditsBar1.png",
          "creditsBar2.png",
          "creditsBar2b.png",
          "creditsBar2c.png",
          "creditsBar2d.png",
          "bottomBar.png",
          "epilepsyWarning.png"})
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
        if (std::exchange(mustShowLoginAtStartup, false) && Config::getShowLoginAtStartup())
        {
            openLoginDialogBoxAndStartLoginProcess();
            setIgnoreAllInputs(2);
        }
    }

    if (state == States::LevelSelection)
    {
        firstLevelSelection = false;
    }

    if (!showFirstTimeTips)
    {
        // Not the first time playing.
        return;
    }

    const auto mustShowTip = [&](const States s, bool& flag) { return state == s && std::exchange(flag, false); };

    const auto showTip = [&](const char* str)
    {
        playSoundOverride("select.ogg");

        showDialogBox(str);
        setIgnoreAllInputs(1);

        // Prevent dialog box from being closed immediately:
        dialogBoxDelay = 64.f;
    };

    if (mustShowTip(States::SMain, mustShowFTTMainMenu))
    {
        showTip(
            "WELCOME TO OPEN HEXAGON!\n\n"
            "YOU CAN NAVIGATE THE MAIN MENU WITH THE UP/DOWN ARROW KEYS\n"
            "OR THE DPAD/THUMBSTICK ON YOUR CONTROLLER\n\n"
            "REMEMBER TO CHECK OUT THE OPTIONS MENU\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
    }
    else if (mustShowTip(States::LevelSelection, mustShowFTTLevelSelect))
    {
        showTip(
            "THIS IS WHERE YOU CAN PICK A LEVEL TO PLAY\n\n"
            "LEVELS ARE ORGANIZED IN 'LEVEL PACKS'\n"
            "TO BROWSE LEVELS AND LEVEL PACKS, GO UP/DOWN\n"
            "HOLD SHIFT (FOCUS) TO QUICKLY JUMP BETWEEN PACKS\n"
            "TO CHANGE THE DIFFICULTY OF A LEVEL, GO LEFT/RIGHT\n\n"
            "AS A FIRST TIMER, PLAY THE 'CUBE' LEVELS IN ORDER\n"
            "THEN YOU CAN GET NEW LEVELS ON THE STEAM WORKSHOP\n\n"
            "HAVE FUN!\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
    }
    else if (mustShowTip(States::LevelSelection, mustShowFTTDeathTips))
    {
        showTip(
            "IF YOU FEEL LIKE THE GAME IS TOO HARD, TRY THESE TIPS\n\n"
            "- DECREMENT THE DIFFICULTY BY PRESSING LEFT\n"
            "- PLAY AN EASIER LEVEL TO BUILD UP MUSCLE MEMORY\n"
            "- TURN ON 'INVINCIBILITY' IN 'GAMEPLAY' OPTIONS TO PRACTICE\n\n"
            "REMEMBER THAT PRACTICE IS THE SECRET TO SUCCESS!\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
    }
}

void MenuGame::initInput()
{
    using k   = sf::Keyboard::Key;
    using t   = ssvs::Input::Type;
    using Tid = Config::Tid;

    const auto addTidInput = [&](const Tid tid, const t type, auto action)
    { game.addInput(Config::getTrigger(tid), action, type, static_cast<int>(tid)); };

    addTidInput(Tid::RotateCCW,
                t::Once,
                [this](float)
    {
        if (!mouseHovering)
        {
            leftAction();
        }
    });

    addTidInput(Tid::RotateCW,
                t::Once,
                [this](float)
    {
        if (!mouseHovering)
        {
            rightAction();
        }
    });

    game.addInput( // hardcoded
        {{k::Up}},
        [this](float) { upAction(); },
        t::Once);

    addTidInput(Tid::Up, t::Once, [this](float) { upAction(); });

    game.addInput( // hardcoded
        {{k::Down}},
        [this](float) { downAction(); },
        t::Once);

    addTidInput(Tid::Down, t::Once, [this](float) { downAction(); });

    addTidInput(Tid::NextPack, t::Once, [this](float) { changePackAction(1); });

    addTidInput(Tid::PreviousPack, t::Once, [this](float) { changePackAction(-1); });

    add2StateInput(game, Config::getTrigger(Tid::Focus), focusHeld, static_cast<int>(Tid::Focus));

    game.addInput( // hardcoded
        {{k::Enter}},
        [this](float /*unused*/) { okAction(); },
        t::Once);

    game.addInput( // hardcoded
        {{k::Escape}},
        [this](float mFT)
    {
        if (state != States::MOpts)
        {
            exitTimer += mFT;
        }
    },
        [this](float /*unused*/) { exitTimer = 0; },
        t::Always);

    game.addInput( // hardcoded
        {{k::Escape}},
        [this](float /*unused*/) { exitAction(); },
        t::Once);

    addTidInput(Tid::Exit,
                t::Once,
                [this](float /*unused*/)
    {
        if (isEnteringText())
        {
            return;
        }
        // The new UI's text fields are always-focused (e.g. the LevelSelect
        // search bar), so a rebound exit key like 'T' would otherwise both
        // type into the search and pop the screen on the same frame. Only
        // the hardcoded Escape binding (line 1438) reaches the new UI now.
        if (newUIActiveForCurrentState())
        {
            return;
        }
        exitAction();
    }); // editable

    addTidInput(Tid::Screenshot, t::Once, [this](float /*unused*/) { mustTakeScreenshot = true; });

    game.addInput({{k::LAlt, k::Enter}},
                  [this](float /*unused*/)
    {
        Config::setFullscreen(window, !window.getFullscreen());
        game.ignoreNextInputs();
    },
                  t::Once)
        .setPriorityUser(-1000);

    game.addInput( // hardcoded
        {{k::Backspace}},
        [this](float /*unused*/) { eraseAction(); },
        t::Once);

    game.addInput( // hardcoded
        {{k::F1}},
        [this](float /*unused*/) { addRemoveFavoriteLevel(); },
        t::Once);

    game.addInput( // hardcoded
        {{k::F2}},
        [this](float /*unused*/) { switchToFromFavoriteLevels(); },
        t::Once);

    game.addInput( // hardcoded
        {{k::F3}},
        [this](float /*unused*/) { reloadAssets(false); },
        t::Once);

    game.addInput( // hardcoded
        {{k::F4}},
        [this](float /*unused*/) { reloadAssets(true); },
        t::Once);
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
    adjustLevelsOffset();
    adjustMenuOffset(true);
    resetNamesScrolls();
}

void MenuGame::playSoundOverride(const sf::base::String& assetId)
{
    if (!Config::getNoSound())
    {
        audio.playSoundOverride(assetId);
    }
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

void MenuGame::ignoreInputsAfterMenuExec()
{
    // We only want to ignore a single input when using the left mouse button,
    // otherwise the user would have to press enter twice to accept in a dialog
    // box.

    setIgnoreAllInputs(mustUseMenuItem.hasValue() ? 1 : 2);
}

void MenuGame::initMenus()
{
    namespace i = ssvms::Items;

    auto whenNotOfficial = [] { return !Config::getOfficial(); };

    // Welcome menu
    auto& wlcm(welcomeMenu.createCategory("welcome"));
    wlcm.create<i::Single>("play locally", [this] { playLocally(); });
    wlcm.create<i::Single>("exit game", [this] { window.stop(); });

    //--------------------------------
    // OPTIONS MENU
    //--------------------------------

    auto& options(optionsMenu.createCategory("options"));
    auto& play(optionsMenu.createCategory("gameplay"));
    auto& controls(optionsMenu.createCategory("controls"));
    auto& keyboard(optionsMenu.createCategory("keyboard"));
    auto& joystick(optionsMenu.createCategory("joystick"));
    auto& resolution(optionsMenu.createCategory("resolution"));
    auto& gfx(optionsMenu.createCategory("graphics"));
    auto& sfx(optionsMenu.createCategory("audio"));
    auto& advanced(optionsMenu.createCategory("advanced"));

    options.create<i::Goto>("GAMEPLAY", play);
    options.create<i::Goto>("CONTROLS", controls);
    options.create<i::Goto>("RESOLUTION", resolution);
    options.create<i::Goto>("GRAPHICS", gfx);
    options.create<i::Goto>("AUDIO", sfx);
    options.create<i::Goto>("ADVANCED", advanced);
    options.create<i::Single>("RESET CONFIG",
                              [this]
    {
        Config::resetConfigToDefaults();
        refreshBinds();
    });

    //--------------------------------
    // Gameplay

    play.create<i::Toggle>("autorestart", &Config::getAutoRestart, &Config::setAutoRestart);
    play.create<i::Toggle>("rotate to start", &Config::getRotateToStart, &Config::setRotateToStart);
    play.create<i::Toggle>("OFFICIAL MODE", &Config::getOfficial, &Config::setOfficial);
    play.create<i::Toggle>("debug mode", &Config::getDebug, &Config::setDebug) | whenNotOfficial;
    play.create<i::Toggle>("invincible", &Config::getInvincible, &Config::setInvincible) | whenNotOfficial;
    play.create<i::Slider>("timescale", &Config::getTimescale, &Config::setTimescale, 0.1f, 2.f, 0.05f) | whenNotOfficial;
    play.create<i::Toggle>("save last username", &Config::getSaveLastLoginUsername, &Config::setSaveLastLoginUsername);
    play.create<i::Toggle>("show login at startup", &Config::getShowLoginAtStartup, &Config::setShowLoginAtStartup);
    play.create<i::GoBack>("back");

    //--------------------------------
    // Controls

    controls.create<i::Goto>("keyboard", keyboard);
    controls.create<i::Goto>("joystick", joystick);
    controls.create<i::Slider>("joystick deadzone", &Config::getJoystickDeadzone, &Config::setJoystickDeadzone, 0.f, 100.f, 1.f);
    controls.create<i::Single>("reset binds",
                               [this]
    {
        Config::resetBindsToDefaults();
        refreshBinds();
    });
    controls.create<i::Single>("hardcoded keys reference",
                               [this]
    {
        showDialogBox(
            "UP ARROW - UP\n"
            "DOWN ARROW - DOWN\n"
            "RETURN - ENTER\n"
            "BACKSPACE - REMOVE BIND\n"
            "F1 - ADD LEVEL TO FAVORITES\n"
            "F2 - SWITCH TO/FROM FAVORITE LEVELS\n"
            "F3 - RELOAD LEVEL ASSETS (DEBUG MODE ONLY)\n"
            "F4 - RELOAD PACK ASSETS (DEBUG MODE ONLY)\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
        ignoreInputsAfterMenuExec();
    });
    controls.create<i::GoBack>("back");

    // Keyboard binds
    const auto callBack = [this](const ssvs::Input::Trigger& trig, const int bindID)
    {
        game.refreshTrigger(trig, bindID);

        if (fnHGTriggerRefresh)
        {
            fnHGTriggerRefresh(trig, bindID);
        }
    };

    using Tid = Config::Tid;

    const auto mkAddBindFn = [](ssvs::Input::Trigger& trig)
    {
        return [&trig](const sf::Keyboard::Key key, const sf::Mouse::Button btn, const int index)
        { Config::rebindTrigger(trig, key, btn, index); };
    };

    const auto mkClearBindFn = [](ssvs::Input::Trigger& trig)
    { return [&trig](const int index) { Config::clearTriggerBind(trig, index); }; };

    const auto createKeyboardBindControl =
        [&](const char* name, const Tid tid, const sf::Keyboard::Key hardcodedKey = sf::Keyboard::Key::Unknown)
    {
        const auto trigGetter = Config::triggerGetters[SSVOH_TO_SIZET(tid)];

        ssvs::Input::Trigger& trig = trigGetter();

        keyboard.create<KeyboardBindControl>(name,
                                             trigGetter,
                                             mkAddBindFn(trig),
                                             mkClearBindFn(trig),
                                             callBack,
                                             static_cast<int>(tid),
                                             hardcodedKey);
    };

    createKeyboardBindControl("rotate ccw", Tid::RotateCCW);
    createKeyboardBindControl("rotate cw", Tid::RotateCW);
    createKeyboardBindControl("focus", Tid::Focus);
    createKeyboardBindControl("exit", Tid::Exit, sf::Keyboard::Key::Escape);
    createKeyboardBindControl("force restart", Tid::ForceRestart);
    createKeyboardBindControl("restart", Tid::Restart);
    createKeyboardBindControl("replay", Tid::Replay);
    createKeyboardBindControl("screenshot", Tid::Screenshot);
    createKeyboardBindControl("swap", Tid::Swap);
    createKeyboardBindControl("up", Tid::Up, sf::Keyboard::Key::Up);
    createKeyboardBindControl("down", Tid::Down, sf::Keyboard::Key::Down);
    createKeyboardBindControl("next pack", Tid::NextPack);
    createKeyboardBindControl("previous pack", Tid::PreviousPack);
    createKeyboardBindControl("lua console (debug only)", Tid::LuaConsole);
    createKeyboardBindControl("pause game (debug only)", Tid::Pause);
    keyboard.create<i::GoBack>("back");

    // Joystick binds
    using Jid = Joystick::Jid;

    const auto joystickCallBack = [](const unsigned int button, const int buttonID)
    { Joystick::setJoystickBind(button, buttonID); };

    const auto createJoystickBindControl = [&](const char* name, const Jid jid)
    {
        const auto btnGetter = Config::joystickTriggerGetters[SSVOH_TO_SIZET(jid)];

        const auto btnSetter = Config::joystickTriggerSetters[SSVOH_TO_SIZET(jid)];

        joystick.create<JoystickBindControl>(name, btnGetter, btnSetter, joystickCallBack, static_cast<int>(jid));
    };

    createJoystickBindControl("select", Jid::Select);
    createJoystickBindControl("exit", Jid::Exit);
    createJoystickBindControl("focus", Jid::Focus);
    createJoystickBindControl("swap", Jid::Swap);
    createJoystickBindControl("force restart", Jid::ForceRestart);
    createJoystickBindControl("restart", Jid::Restart);
    createJoystickBindControl("replay", Jid::Replay);
    createJoystickBindControl("screenshot", Jid::Screenshot);
    createJoystickBindControl("next pack", Jid::NextPack);
    createJoystickBindControl("previous pack", Jid::PreviousPack);
    createJoystickBindControl("(un)favorite level", Jid::AddToFavorites);
    createJoystickBindControl("favorites menu", Jid::FavoritesMenu);
    joystick.create<i::GoBack>("back");

    //--------------------------------
    // Resolution

    resolution.create<i::Single>("automatically set resolution", [this] { Config::setCurrentResolutionAuto(window); });

    auto& sixByNine(optionsMenu.createCategory("16x9 resolutions"));
    auto& fourByThree(optionsMenu.createCategory("4x3 resolutions"));
    auto& sixByTen(optionsMenu.createCategory("16x10 resolutions"));

    resolution.create<i::Goto>("16x9 resolutions", sixByNine);
    resolution.create<i::Goto>("4x3 resolutions", fourByThree);
    resolution.create<i::Goto>("16x10 - uncommon resolutions", sixByTen);

    // Organize available resolutions based on their aspect ratio.
    int ratio;
    for (const auto& vm : sf::VideoModeUtils::getFullscreenModes())
    {
        if (vm.bitsPerPixel == 32)
        {
            ratio = 10.f * vm.size.x / vm.size.y;

            switch (ratio)
            {
                case 17: // 16:9
                    sixByNine.create<i::Single>(sf::base::String(ssvu::toStr(vm.size.x) + "x" + ssvu::toStr(vm.size.y)),
                                                [this, &vm] { changeResolutionTo(vm.size.x, vm.size.y); });
                    break;

                case 13: // 4:3
                    fourByThree.create<i::Single>(sf::base::String(ssvu::toStr(vm.size.x) + "x" + ssvu::toStr(vm.size.y)),
                                                  [this, &vm] { changeResolutionTo(vm.size.x, vm.size.y); });
                    break;

                default: // 16:10 and uncommon
                    sixByTen.create<i::Single>(sf::base::String(ssvu::toStr(vm.size.x) + "x" + ssvu::toStr(vm.size.y)),
                                               [this, &vm] { changeResolutionTo(vm.size.x, vm.size.y); });
                    break;
            }
        }
    }

    sixByNine.create<i::GoBack>("back");
    fourByThree.create<i::GoBack>("back");
    sixByTen.create<i::GoBack>("back");

    resolution.create<i::Single>("go windowed", [this] { Config::setFullscreen(window, false); });
    resolution.create<i::Single>("go fullscreen", [this] { Config::setFullscreen(window, true); });
    resolution.create<i::GoBack>("back");

    //--------------------------------
    // Graphics

    auto& visfx(optionsMenu.createCategory("visual fxs"));
    gfx.create<i::Goto>("visual fxs", visfx);
    visfx.create<i::Toggle>("3D effects", &Config::get3D, &Config::set3D);
    visfx.create<i::Toggle>("shader effects", &Config::getShaders, &Config::setShaders);
    visfx.create<i::Toggle>("no pulse", &Config::getNoPulse, &Config::setNoPulse) | whenNotOfficial;
    visfx.create<i::Toggle>("no rotation", &Config::getNoRotation, &Config::setNoRotation) | whenNotOfficial;
    visfx.create<i::Toggle>("no background", &Config::getNoBackground, &Config::setNoBackground) | whenNotOfficial;
    visfx.create<i::Toggle>("b&w colors", &Config::getBlackAndWhite, &Config::setBlackAndWhite) | whenNotOfficial;

    visfx.create<i::Toggle>("flash", &Config::getFlash, &Config::setFlash);
    visfx.create<i::Slider>("shake mult.", &Config::getCameraShakeMultiplier, &Config::setCameraShakeMultiplier, 0.f, 5.f, 0.1f);

    auto& playervisfx(optionsMenu.createCategory("player visual fxs"));
    gfx.create<i::Goto>("player visual fxs", playervisfx);
    playervisfx.create<i::Slider>("angle tilt mult.", &Config::getAngleTiltIntensity, &Config::setAngleTiltIntensity, 0.f, 5.f, 0.1f);
    playervisfx.create<i::Toggle>("show trail", &Config::getShowPlayerTrail, &Config::setShowPlayerTrail);
    playervisfx.create<i::Slider>("trail alpha", &Config::getPlayerTrailAlpha, &Config::setPlayerTrailAlpha, 0, 255, 5);
    playervisfx.create<i::Slider>("trail scale", &Config::getPlayerTrailScale, &Config::setPlayerTrailScale, 0.05f, 1.f, 0.05f);
    playervisfx.create<i::Slider>("trail decay", &Config::getPlayerTrailDecay, &Config::setPlayerTrailDecay, 0.5f, 50.f, 2.5f);
    playervisfx.create<i::Toggle>("trail has swap color",
                                  &Config::getPlayerTrailHasSwapColor,
                                  &Config::setPlayerTrailHasSwapColor);
    playervisfx.create<i::Toggle>("show swap particles", &Config::getShowSwapParticles, &Config::setShowSwapParticles);
    playervisfx.create<i::Toggle>("swap blinking effect", &Config::getShowSwapBlinkingEffect, &Config::setShowSwapBlinkingEffect);
    playervisfx.create<i::GoBack>("back");

    auto& fps(optionsMenu.createCategory("fps settings"));
    gfx.create<i::Goto>("fps settings", fps);
    fps.create<i::Toggle>("vsync", &Config::getVsync, [this](bool mValue) { Config::setVsync(window, mValue); });
    fps.create<i::Toggle>("limit fps", &Config::getLimitFPS, [this](bool mValue) { Config::setLimitFPS(window, mValue); });
    fps.create<i::Slider>("max fps", &Config::getMaxFPS, [this](unsigned int mValue) {
        Config::setMaxFPS(window, mValue);
    }, 30u, 1000u, 5u);
    fps.create<i::Toggle>("show fps", &Config::getShowFPS, &Config::setShowFPS);
    fps.create<i::GoBack>("back");

    gfx.create<i::Toggle>("text outlines", &Config::getDrawTextOutlines, &Config::setDrawTextOutlines);
    gfx.create<i::Slider>("text padding", &Config::getTextPadding, [](float mValue) {
        Config::setTextPadding(mValue);
    }, 0.f, 64.f, 1.f);
    gfx.create<i::Slider>("text scaling", &Config::getTextScaling, [](float mValue) {
        Config::setTextScaling(mValue);
    }, 0.1f, 4.f, 0.05f);

    gfx.create<i::Slider>("antialiasing",
                          [] { return Utils::concat(Config::getAntialiasingLevel(), 'x'); },
                          [this]
    {
        if (Config::getAntialiasingLevel() == 0)
        {
            Config::setAntialiasingLevel(window, 1);
            return;
        }

        Config::setAntialiasingLevel(window, ssvu::getClamped(Config::getAntialiasingLevel() << 1u, 0u, 16u));
    },
                          [this]
    { Config::setAntialiasingLevel(window, ssvu::getClamped(Config::getAntialiasingLevel() >> 1u, 0u, 16u)); });

    gfx.create<i::Toggle>("darken background chunk",
                          &Config::getDarkenUnevenBackgroundChunk,
                          &Config::setDarkenUnevenBackgroundChunk);
    gfx.create<i::Toggle>("show key icons", &Config::getShowKeyIcons, &Config::setShowKeyIcons);
    gfx.create<i::Slider>("key icons scaling", &Config::getKeyIconsScale, [](float mValue) {
        Config::setKeyIconsScale(mValue);
    }, 0.1f, 4.f, 0.05f);
    gfx.create<i::Toggle>("show level info", &Config::getShowLevelInfo, &Config::setShowLevelInfo);
    gfx.create<i::Toggle>("show timer", &Config::getShowTimer, &Config::setShowTimer) | whenNotOfficial;
    gfx.create<i::Toggle>("show status text", &Config::getShowStatusText, &Config::setShowStatusText) | whenNotOfficial;

    gfx.create<i::GoBack>("back");

    //--------------------------------
    // Sound

    sfx.create<i::Toggle>("no sound", &Config::getNoSound, &Config::setNoSound);
    sfx.create<i::Toggle>("no music", &Config::getNoMusic, &Config::setNoMusic);
    sfx.create<i::Slider>("sound volume",
                          &Config::getSoundVolume,
                          [this](unsigned int mValue)
    {
        Config::setSoundVolume(mValue);
        audio.setSoundVolume(mValue);
    },
                          0u,
                          100u,
                          5u);
    sfx.create<i::Slider>("music volume",
                          &Config::getMusicVolume,
                          [this](unsigned int mValue)
    {
        Config::setMusicVolume(mValue);
        audio.setMusicVolume(mValue);
    },
                          0u,
                          100u,
                          5u);
    sfx.create<i::Toggle>("sync music with difficulty", &Config::getMusicSpeedDMSync, &Config::setMusicSpeedDMSync);
    sfx.create<i::Slider>("music speed multiplier", &Config::getMusicSpeedMult, [](float mValue) {
        Config::setMusicSpeedMult(mValue);
    }, 0.7f, 1.3f, 0.05f);
    sfx.create<i::Toggle>("play swap ready blip sound", &Config::getPlaySwapReadySound, &Config::setPlaySwapReadySound);
    sfx.create<i::GoBack>("back");

    //--------------------------------
    // Advanced

    advanced.create<i::Toggle>("cache lua files", &Config::getUseLuaFileCache, &Config::setUseLuaFileCache);

    advanced.create<i::Single>("clear lua file cache", [this] { assets.getLuaFileCache().clear(); });

    advanced.create<i::Toggle>("disable game rendering", &Config::getDisableGameRendering, &Config::setDisableGameRendering);

    //--------------------------------
    // MAIN MENU
    //--------------------------------

    auto& main{mainMenu.createCategory("main")};
    auto& localProfiles{mainMenu.createCategory("local profiles")};
    main.create<i::Single>("LEVEL SELECT",
                           [this]
    {
        if (firstLevelSelection)
        {
            lvlSlct.packIdx = diffMultIdx = 0;
            lvlSlct.levelDataIds          = &assets.getLevelIdsByPack(getNthSelectablePackInfo(0).id);
            setIndex(0);
        }
        changeStateTo(States::LevelSelection);
        playSoundOverride("select.ogg");
    });
    main.create<i::Goto>("LOCAL PROFILES", localProfiles);
    main.create<i::Single>("ONLINE", [this] { changeStateTo(States::MOnline); });
    main.create<i::Single>("OPTIONS", [this] { changeStateTo(States::MOpts); });
    main.create<i::Single>("EXIT", [this] { window.stop(); });

    //--------------------------------
    // ONLINE MENU
    //--------------------------------

    auto whenMustConnect = [this]
    {
        return hexagonClient.getState() == HexagonClient::State::Disconnected ||
               hexagonClient.getState() == HexagonClient::State::ConnectionError;
    };

    auto whenMustLogin = [this]
    { return hexagonClient.getState() == HexagonClient::State::Connected && hexagonClient.hasRTKeys(); };

    auto whenMustRegister = [this]
    { return hexagonClient.getState() == HexagonClient::State::Connected && hexagonClient.hasRTKeys(); };

    auto whenConnected = [this] { return hexagonClient.getState() == HexagonClient::State::Connected; };

    auto whenLoggedIn = [this]
    {
        return (hexagonClient.getState() == HexagonClient::State::LoggedIn ||
                hexagonClient.getState() == HexagonClient::State::LoggedIn_Ready) &&
               hexagonClient.hasRTKeys();
    };

    auto whenMustDeleteAccount = [this]
    { return hexagonClient.getState() == HexagonClient::State::Connected && hexagonClient.hasRTKeys(); };


    auto& online(onlineMenu.createCategory("options"));

    online.create<i::Single>("CONNECT", [this] { hexagonClient.connect(); }) | whenMustConnect;

    online.create<i::Single>("LOGIN",
                             [this]
    {
        if (dialogInputState != DialogInputState::Nothing)
        {
            return;
        }

        openLoginDialogBoxAndStartLoginProcess();
        ignoreInputsAfterMenuExec();
    }) | whenMustLogin;

    online.create<i::Single>("REGISTER",
                             [this]
    {
        if (dialogInputState != DialogInputState::Nothing)
        {
            return;
        }

        dialogInputState = DialogInputState::Registration_EnteringUsername;

        showInputDialogBoxNice("REGISTRATION", "USERNAME");
        ignoreInputsAfterMenuExec();
    }) | whenMustRegister;

    online.create<i::Single>("LOGOUT", [this] { hexagonClient.tryLogoutFromServer(); }) | whenLoggedIn;

    online.create<i::Single>("DISCONNECT", [this] { hexagonClient.disconnect(); }) | whenConnected;

    online.create<i::Single>("DELETE ACCOUNT",
                             [this]
    {
        if (dialogInputState != DialogInputState::Nothing)
        {
            return;
        }

        dialogInputState = DialogInputState::DeleteAccount_EnteringPassword;

        showInputDialogBoxNice("DELETE ACCOUNT", "PASSWORD", "WARNING: THIS WILL DELETE ALL YOUR SCORES");
        dialogBox.setInputBoxPassword(true);
        ignoreInputsAfterMenuExec();
    }) | whenMustDeleteAccount;

    //--------------------------------
    // PROFILES MENU
    //--------------------------------

    localProfiles.create<i::Single>("CHOOSE PROFILE", [this] { changeStateTo(States::SLPSelect); });
    localProfiles.create<i::Single>("NEW PROFILE",
                                    [this]
    {
        changeStateTo(States::ETLPNew);
        enteredStr = "";
        playSoundOverride("select.ogg");
    });
    localProfiles.create<i::GoBack>("BACK");

    //--------------------------------
    // Profiles selection

    auto& profileSelection{profileSelectionMenu.createCategory("profile selection")};

    for (auto& p : assets.getLocalProfileNames())
    {
        profileSelection.create<i::Single>(p,
                                           [this, p]
        {
            assets.pSetCurrent(p);
            changeFavoriteLevelsToProfile();
        });
    }

    profileSelection.sortByName();
}

bool MenuGame::loadCommandLineLevel(const sf::base::String& pack, const sf::base::String& level)
{
    // First find the ID of the pack with name matching the one typed by the
    // user. `packDatas` is the only vector in assets with a data type
    // containing the name of the pack (without it being part of the id).
    sf::base::String packID;
    for (auto& d : assets.getPackDatas())
    {
        if (d.second.name == pack)
        {
            packID = d.second.id;
            break;
        }
    }

    if (packID.empty())
    {
        hg::lo("hg::Menugame::MenuGame()")
            << "Invalid pack name '" << pack << "' command line parameter, aborting boot level load\n";

        return false;
    }

    if (!assets.packHasLevels(packID))
    {
        hg::lo("hg::Menugame::MenuGame()") << "Pack '" << pack << "' has no levels, aborting boot level load\n";

        return false;
    }

    // Iterate through packInfos to find the menu pack index and the index
    // of the level.
    const sf::base::String levelID{packID + "_" + level};
    const auto&            p{assets.getSelectablePackInfos()};
    const auto&            levelsList{assets.getLevelIdsByPack(packID)};

    for (int i{0}; i < static_cast<int>(p.size()); ++i)
    {
        // once you find the pack index search if it contains the level
        if (packID != p[i].id)
        {
            continue;
        }

        auto it{std::find(levelsList.begin(), levelsList.end(), levelID)};
        if (it == levelsList.end())
        {

            hg::lo("hg::Menugame::MenuGame()")
                << "Invalid level name '" << level << "' command line parameter, aborting boot level load\n";

            return false;
        }

        // Level found, initialize parameters
        lvlSlct.packIdx      = i;
        lvlSlct.levelDataIds = &levelsList;
        setIndex(it - levelsList.begin());

        break;
    }

    // Do the sequence of actions user would have
    // to do manually to get to the desired level

    playLocally(); // go to profile selection screen

    if (state == States::ETLPNew)
    {
        hg::lo("hg::Menugame::MenuGame()") << "No player profiles exist, aborting boot level load\n";

        return false;
    }

    // Go to main menu
    enteredStr = assets.getLocalProfileNames()[0];
    assets.pSetCurrent(enteredStr);
    changeStateTo(States::SMain);

    // Go to level selection
    resetNamesScrolls();
    changeStateTo(States::LevelSelection);

    // Start game
    playSelectedLevel();

    return true;
}

void MenuGame::playLocally()
{
    assets.pSaveCurrent();
    enteredStr = "";

    // Single-profile model: no more profile-selection screen. If no
    // profile exists yet, auto-create a default one; otherwise pick the
    // first existing profile when none is currently selected. Goes
    // straight to the main menu.
    if (assets.getLocalProfilesSize() == 0)
    {
        const sf::base::String defaultName{"PLAYER"};
        assets.pCreate(defaultName);
        assets.pSetCurrent(defaultName);
        changeFavoriteLevelsToProfile();
    }
    else if (!assets.pIsValidLocalProfile())
    {
        const auto names = assets.getLocalProfileNames();
        if (!names.empty())
        {
            assets.pSetCurrent(names[0]);
            changeFavoriteLevelsToProfile();
        }
    }

    changeStateTo(States::SMain);
}

[[nodiscard]] std::pair<const unsigned int, const unsigned int> MenuGame::pickRandomMainMenuBackgroundStyle()
{
    // If there is no `menubackgrounds.json` abort
    if (!ssvufs::Path{"Assets/menubackgrounds.json"}.isFile())
    {
        hg::lo("MenuGame::$") << "File 'Assets/menubackgrounds.json' does not exist" << logEndl;

        return {0, 0};
    }

    sf::base::Vector<sf::base::String> levelIDs;
    ssvuj::Obj                         object = ssvuj::getFromFile("Assets/menubackgrounds.json");
    for (const auto& f : ssvuj::getExtr<sf::base::Vector<sf::base::String>>(object, "ids"))
    {
        levelIDs.emplaceBack(f);
    }

    // pick one of those at random
    const sf::base::String pickedLevel{levelIDs[ssvu::getRndI(0, levelIDs.size())]};

    // retrieve the level index location
    const auto&                               p(assets.getSelectablePackInfos());
    const sf::base::Vector<sf::base::String>* levelsIDs;

    // store info main menu requires to set the color theme
    for (int i{0}; i < static_cast<int>(p.size()); ++i)
    {
        const sf::base::String& packId = p[i].id;

        if (!assets.packHasLevels(packId))
        {
            continue;
        }

        levelsIDs = &assets.getLevelIdsByPack(packId);
        auto it   = std::find(levelsIDs->begin(), levelsIDs->end(), pickedLevel);
        if (it != levelsIDs->end())
        {
            return {i, it - levelsIDs->begin()};
        }
    }

    return {0, 0};
}

//*****************************************************
//
// NAVIGATION
//
//*****************************************************

void MenuGame::leftRightActionImpl(bool left)
{
    if (state == States::SLPSelectBoot)
    {
        okAction();
        return;
    }

    // Change difficulty in the level selection menu.
    if (state == States::LevelSelection)
    {
        if (left)
        {
            --diffMultIdx;
            playSoundOverride("difficultyMultDown.ogg");
        }
        else
        {
            ++diffMultIdx;
            playSoundOverride("difficultyMultUp.ogg");
        }

        difficultyBumpEffect = difficultyBumpEffectMax;
        touchDelay           = 50.f;
        return;
    }

    // If there is no valid action abort.
    if (!isInMenu() || !getCurrentMenu()->getItem().canIncrease())
    {
        return;
    }

    const bool modifier = (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift) || focusHeld || wasFocusHeld);

    for (int i{0}; i < (modifier ? 2 : 1); ++i)
    {
        if (left)
        {
            getCurrentMenu()->decrease();
        }
        else
        {
            getCurrentMenu()->increase();
        }
    }

    playSoundOverride("beep.ogg");
    touchDelay = 50.f;
}

void MenuGame::leftAction()
{
    if (newUIActiveForCurrentState())
    {
        ui_pendingInput.left = true;
        return;
    }
    leftRightActionImpl(true /* left */);
}

void MenuGame::rightAction()
{
    if (newUIActiveForCurrentState())
    {
        ui_pendingInput.right = true;
        return;
    }
    leftRightActionImpl(false /* left */);
}

inline constexpr int maxProfilesOnScreen{6};

void MenuGame::upAction()
{
    if (newUIActiveForCurrentState())
    {
        ui_pendingInput.up = true;
        return;
    }

    if (state == States::LevelSelection)
    {
        // Do not do anything until the pack change animation is over.
        if (packChangeState != PackChange::Rest)
        {
            return;
        }

        // When focus is held do an instant pack change.
        if (focusHeld)
        {
            changePackQuick(-1);
            return;
        }

        const int prevIdx{lvlDrawer->currentIndex - 1};

        // If there is only one pack behave differently.
        if (getSelectablePackInfosSize() == 1)
        {
            // If we are at the top of the pack go to its end instead
            // and scroll the menu to show it.
            if (prevIdx < 0)
            {
                setIndex(lvlDrawer->levelDataIds->size() - 1);
                calcScrollSpeed();

                const float scroll{packLabelHeight + levelLabelHeight * (lvlDrawer->currentIndex + 1) + 2.f * slctFrameSize};
                if (scroll > h - lvlDrawer->YOffset)
                {
                    lvlDrawer->YScrollTo = h - scroll;
                }
            }
            else
            {
                setIndex(prevIdx);
                calcLevelChangeScroll(-2);
            }
        }
        else if (prevIdx < 0)
        {
            // -2 means "go to previous pack and
            // skip to the last level of the list"
            changePackAction(-2);
        }
        else
        {
            setIndex(prevIdx);
            calcLevelChangeScroll(-2);
        }

        // Reset the scroll values of the text items that are related
        // to level specific fields.
        resetLevelNamesScrolls();
        playSoundOverride("beep.ogg");
        touchDelay = 50.f;
        return;
    }

    // In the loading screen scroll through the error messages.
    if (state == States::LoadingScreen)
    {
        if (scrollbarOffset != 0)
        {
            --scrollbarOffset;
            playSoundOverride("beep.ogg");
            touchDelay = 50.f;
        }
        return;
    }

    if (!isInMenu())
    {
        return;
    }

    // Scroll the profiles drawn on screen
    if ((state == States::SLPSelect || state == States::SLPSelectBoot) && getCurrentMenu()->getIdx() - 1 < scrollbarOffset)
    {
        const int index = ssvu::getMod(getCurrentMenu()->getIdx() - 1,
                                       0,
                                       static_cast<int>(getCurrentMenu()->getItems().size()));
        scrollbarOffset = std::max(index - (maxProfilesOnScreen - 1), 0);
    }

    if (getCurrentMenu() != nullptr && !anyItemEnabled(*getCurrentMenu()))
    {
        return;
    }

    // Go to the first enabled menu item.
    do
    {
        getCurrentMenu()->previous();
    } while (!getCurrentMenu()->getItem().isEnabled());

    playSoundOverride("beep.ogg");
    touchDelay = 50.f;
}

inline constexpr int maxErrorsOnScreen{7};

void MenuGame::downAction()
{
    if (newUIActiveForCurrentState())
    {
        ui_pendingInput.down = true;
        return;
    }
    if (state == States::LevelSelection)
    {
        if (packChangeState != PackChange::Rest)
        {
            return;
        }

        if (focusHeld)
        {
            changePackQuick(1);
            return;
        }

        const int nextIdx{lvlDrawer->currentIndex + 1};
        if (getSelectablePackInfosSize() == 1)
        {
            if (nextIdx > static_cast<int>(lvlDrawer->levelDataIds->size() - 1))
            {
                // Skip to the first level label.
                setIndex(0);
                calcScrollSpeed();
                lvlDrawer->YScrollTo = 0.f;
            }
            else
            {
                setIndex(nextIdx);
                calcLevelChangeScroll(2);
            }
        }
        else if (nextIdx > static_cast<int>(lvlDrawer->levelDataIds->size() - 1))
        {
            // Go to the next pack.
            changePackAction(1);
        }
        else
        {
            setIndex(nextIdx);
            calcLevelChangeScroll(2);
        }

        resetLevelNamesScrolls();
        playSoundOverride("beep.ogg");
        touchDelay = 50.f;
        return;
    }

    if (state == States::LoadingScreen)
    {
        if (scrollbarOffset < static_cast<int>(loadInfo.errorMessages.size()) - maxErrorsOnScreen)
        {
            ++scrollbarOffset;
            playSoundOverride("beep.ogg");
            touchDelay = 50.f;
        }
        return;
    }

    if (!isInMenu())
    {
        return;
    }

    if ((state == States::SLPSelect || state == States::SLPSelectBoot) &&
        getCurrentMenu()->getIdx() + 1 > maxProfilesOnScreen - 1 + scrollbarOffset)
    {
        const int index = ssvu::getMod(getCurrentMenu()->getIdx() + 1,
                                       0,
                                       static_cast<int>(getCurrentMenu()->getItems().size()));
        scrollbarOffset = std::max(index - (maxProfilesOnScreen - 1), 0);
    }

    if (getCurrentMenu() != nullptr && !anyItemEnabled(*getCurrentMenu()))
    {
        return;
    }

    do
    {
        getCurrentMenu()->next();
    } while (!getCurrentMenu()->getItem().isEnabled());

    playSoundOverride("beep.ogg");
    touchDelay = 50.f;
}

void MenuGame::changePackTo(const int idx)
{
    const auto& p{assets.getSelectablePackInfos()};

    // Deduce the new packIdx.
    lvlSlct.packIdx = ssvu::getMod(idx, 0, static_cast<int>(p.size()));

    // Load level ids relative to the new pack
    lvlSlct.levelDataIds = &assets.getLevelIdsByPack(p[lvlDrawer->packIdx].id);

    // Set the correct level index.
    setIndex(0);

    // Reset all text scrolling.
    resetNamesScrolls();
}

void MenuGame::changePack()
{
    const auto& p{assets.getSelectablePackInfos()};

    // Deduce the new packIdx.
    lvlSlct.packIdx = ssvu::getMod(lvlDrawer->packIdx + (packChangeDirection > 0 ? 1 : -1), 0, static_cast<int>(p.size()));

    // Load level ids relative to the new pack
    lvlSlct.levelDataIds = &assets.getLevelIdsByPack(p[lvlDrawer->packIdx].id);

    // Set the correct level index.
    setIndex(packChangeDirection == -2 ? lvlSlct.levelDataIds->size() - 1 : 0);

    // Reset all text scrolling.
    resetNamesScrolls();
}

void MenuGame::changePackQuick(const int direction)
{
    if (isFavoriteLevels())
    {
        return;
    }

    packChangeDirection = direction;
    playSoundOverride("beep.ogg");
    changePack();
    adjustLevelsOffset();

    // YOffset is 0 when the first pack is shown and gets lower
    // the further down we have to scroll.

    // Height of the top of the pack label that is one index before the current
    // one.
    float scroll{packLabelHeight * (lvlDrawer->packIdx - 1)};

    sf::base::FixedFunction<void(const float), 64> action{[this](const float target)
    { lvlDrawer->YScrollTo = lvlDrawer->YOffset = target; }};

    // If the height is lower than the offset of the level selection
    // the level list must be scrolled to show the labels before the current
    // one. The top must prevail.
    if (!checkWindowTopScrollWithResult(scroll, action))
    {
        // Height of the bottom of the pack label that is one index after the
        // current one.
        scroll = packLabelHeight * std::min(lvlDrawer->packIdx + 2, static_cast<int>(getSelectablePackInfosSize())) +
                 levelLabelHeight + 3.f * slctFrameSize;

        // If the bottom is outside the boundaries of the screen adjust offset
        // to show it.
        checkWindowBottomScroll(scroll, action);
    }
}

void MenuGame::changePackAction(const int direction)
{
    if (state != States::LevelSelection || getSelectablePackInfosSize() == 1 || packChangeState != PackChange::Rest)
    {
        return;
    }

    lvlDrawer->YScrollTo = lvlDrawer->YOffset; // stop scrolling for safety
    // Initiate the pack change animation.
    packChangeState     = PackChange::Folding;
    packChangeDirection = direction;
    calcScrollSpeed();

    touchDelay = 50.f;
    playSoundOverride("beep.ogg");
}

void MenuGame::okAction()
{
    if (newUIActiveForCurrentState())
    {
        ui_pendingInput.enter = true;
        return;
    }

    touchDelay = 50.f;

    switch (state)
    {
        case States::ETLPNewBoot:
            [[fallthrough]];
        case States::ETLPNew:
        {
            if (!enteredStr.empty())
            {
                ssvms::Category& profiles(profileSelectionMenu.getCategoryByName("profile selection"));

                // Abort if user is trying to create a profile
                // with a name already in use
                for (auto& i : profiles.getItems())
                {
                    if (enteredStr == i->getName())
                    {
                        playSoundOverride("error.ogg");
                        showDialogBox(
                            "A PROFILE WITH THE SAME NAME ALREADY EXISTS\n"
                            "PLEASE ENTER ANOTHER NAME\n\n"
                            "PRESS ANY KEY OR BUTTON TO CLOSE THIS "
                            "MESSAGE\n");
                        setIgnoreAllInputs(2);
                        return;
                    }
                }

                // All good
                assets.pCreate(enteredStr);
                assets.pSetCurrent(enteredStr);
                changeFavoriteLevelsToProfile();

                // Create new menu item
                profiles.create<ssvms::Items::Single>(enteredStr,
                                                      [this, name = enteredStr]
                {
                    assets.pSetCurrent(name);
                    changeFavoriteLevelsToProfile();
                });

                profiles.sortByName();

                enteredStr = "";
                if (state == States::ETLPNewBoot)
                {
                    playSoundOverride("openHexagon.ogg");
                    changeStateTo(States::SMain);
                    return;
                }
                changeStateTo(States::SMain);
            }

            break;
        }

        case States::SLPSelectBoot:
        {
            playSoundOverride("openHexagon.ogg");
            getCurrentMenu()->exec();
            changeStateTo(States::SMain);
            return;
        }

        case States::SMain:
        {
            const sf::base::String& category{getCurrentMenu()->getCategory().getName()};
            getCurrentMenu()->exec();

            // Going into the level selection set the selected level
            // label offset to the maximum value and set the other
            // offsets to 0.
            if (state == States::LevelSelection)
            {
                setIndex(lvlDrawer->currentIndex);
                adjustLevelsOffset();
                return;
            }

            if (getCurrentMenu() == nullptr)
            {
                return;
            }

            // Scroll to a menu item that is enabled
            if (scrollToEnabledMenuItem(getCurrentMenu()) == false)
            {
                return;
            }

            // Adjust the indents if we moved to a new submenu
            if (getCurrentMenu()->getCategory().getName() != category)
            {
                adjustMenuOffset(true);
            }

            break;
        }

        case States::MOpts:
        {
            getCurrentMenu()->exec();

            // There are two Bind controllers: KeyboardBindControl and
            // JoystickBindControl. So we cast to the common base class to not
            // check for one and the other.
            const auto* const bc{dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem())};
            if (bc != nullptr && bc->isWaitingForBind())
            {
                setIgnoreAllInputs(2);
                touchDelay = 10.f;
                playSoundOverride("beep.ogg");
                return;
            }

            // Scroll to a menu item that is enabled
            if (scrollToEnabledMenuItem(getCurrentMenu()) == false)
            {
                return;
            }

            break;
        }

        case States::MOnline:
        {
            getCurrentMenu()->exec();

            // Scroll to a menu item that is enabled
            if (scrollToEnabledMenuItem(getCurrentMenu()) == false)
            {
                return;
            }

            break;
        }

        case States::LevelSelection:
        {
            // Reset the scroll of the text fields so that
            // they will be 0 when user exit the level.
            resetNamesScrolls();
            playSelectedLevel();

            break;
        }

        default:
        {
            if (isInMenu())
            {
                getCurrentMenu()->exec();
            }

            break;
        }
    }

    playSoundOverride("select.ogg");
}

void MenuGame::playSelectedLevel()
{
    if (fnHGNewGame)
    {
        setMouseCursorVisible(false);

        fnHGNewGame(                                             //
            getNthSelectablePackInfo(lvlDrawer->packIdx).id,     //
            (*lvlDrawer->levelDataIds)[lvlDrawer->currentIndex], //
            true /* firstPlay */,                                //
            levelData->getNthDiffMult(diffMultIdx),              //
            false /* executeLastReplay */                        //
        );
    }
}

void MenuGame::eraseAction()
{
    if (isEnteringText() && !enteredStr.empty())
    {
        enteredStr.erase(static_cast<sf::base::SizeT>(enteredStr.size() - 1), 1);
        playSoundOverride("beep.ogg");
    }
    else if (state == States::SLPSelect)
    {
        const sf::base::String name{profileSelectionMenu.getCategory().getItem().getName()};

        // There must be at least one profile, don't erase profile
        // currently in use.
        if (profileSelectionMenu.getCategory().getItems().size() <= 1)
        {
            playSoundOverride("error.ogg");
            showDialogBox(
                "YOU CANNOT ERASE THE ONLY REMAINING PROFILE\n\n"
                "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
            setIgnoreAllInputs(2);
            return;
        }
        if (assets.pGetName() == name)
        {
            playSoundOverride("error.ogg");
            showDialogBox(
                "YOU CANNOT ERASE THE CURRENTLY IN USE PROFILE\n\n"
                "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
            setIgnoreAllInputs(2);
            return;
        }

        // Remove the profile .json
        if (const sf::base::String fileName{"Profiles/" + name + ".json"}; std::remove(fileName.cStr()) != 0)
        {
            hg::lo("eraseAction()") << "Error: file " << fileName << " does not exist\n";

            return;
        }
        else
        {
            // Remove profile from memory as well
            assets.pRemove(name);
        }

        // Remove the item from the menu
        profileSelectionMenu.getCategory().remove();
        playSoundOverride("beep.ogg");
    }
    else if (state == States::MOpts && isInMenu())
    {
        // Do not do anything if it's not a bind setter menu item.
        auto* const bc{dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem())};

        if (bc == nullptr)
        {
            return;
        }

        if (bc->erase())
        {
            playSoundOverride("beep.ogg");
        }

        touchDelay = 10.f;
    }
}

void MenuGame::exitAction()
{
    if (newUIActiveForCurrentState())
    {
        ui_pendingInput.escape = true;
        return;
    }

    if (isInMenu() && getCurrentMenu()->getCategory().getName() == "main")
    {
        return;
    }

    playSoundOverride("beep.ogg");

    if (state == States::SLPSelectBoot)
    {
        changeStateTo(States::ETLPNewBoot);
        return;
    }

    if (state == States::LevelSelection)
    {
        changeStateTo(States::SMain);
        resetNamesScrolls();
        adjustMenuOffset(false);
        return;
    }

    if (assets.pIsValidLocalProfile())
    {
        if (isInMenu())
        {
            if (getCurrentMenu()->canGoBack())
            {
                getCurrentMenu()->goBack();
            }
            else
            {
                // If the current menu does not have a
                // super menu it means we must go back to
                // the main menu.
                changeStateTo(States::SMain);
            }
            adjustMenuOffset(false);
        }
        else if (isEnteringText())
        {
            changeStateTo(States::SMain);
        }
    }
}

//*****************************************************
//
// UPDATE
//
//*****************************************************

void MenuGame::update(float mFT)
{
    // Capture frame time for the new UI's animations. `mFT` is in
    // milliseconds (per the engine's convention); convert to seconds.
    ui_dt = mFT / 1000.f;

    // Tick the menu-background level (always) and the in-LevelSelect
    // preview (only when it has a level loaded). `previewMode` keeps
    // both from doing anything they shouldn't (input, scoring, audio).
    // HG's `update`/`draw` are private; trigger them through the public
    // `onUpdate`/`onDraw` delegates that HG's constructor wired up.
    if (hgMenuBg != nullptr)
        hgMenuBg->getGame().onUpdate(mFT);
    if (hgPreview != nullptr && !previewLoadedLevelId.empty())
    {
        hgPreview->getGame().onUpdate(mFT);
    }

    hexagonClient.update();

    const auto showHCEventDialogBox =
        [this](const bool error, const sf::base::String& msg, const sf::base::String& err = "")
    {
        if (!dialogBox.empty())
        {
            return;
        }

        if (state != States::SMain && state != States::MOnline && state != States::MOpts &&
            state != States::SLPSelect && state != States::LevelSelection)
        {
            return;
        }

        if (error)
        {
            playSoundOverride("error.ogg");
        }
        else
        {
            playSoundOverride("select.ogg");
        }

        strBuf.clear();

        strBuf += msg;
        if (!err.empty())
        {
            strBuf += "\n\n";
            strBuf += err;
        }

        strBuf += "\n";

        // Prevent dialog box from being closed immediately:
        dialogBoxDelay = 16.f;

        showDialogBox(strBuf);
        setIgnoreAllInputs(1);

        SSVOH_ASSERT(!dialogBox.empty());
        SSVOH_ASSERT(ignoreInputs == 1);
    };

    sf::base::Optional<HexagonClient::Event> hcEvent;
    while ((hcEvent = hexagonClient.pollEvent()).hasValue())
    {
        hcEvent->linearMatch( //

            [&](const HexagonClient::EConnectionSuccess&)
        { showHCEventDialogBox(false /* error */, "CONNECTION SUCCESS"); },

            [&](const HexagonClient::EConnectionFailure& e)
        { showHCEventDialogBox(true /* error */, "CONNECTION FAILURE", e.error); },

            [&](const HexagonClient::EKicked&) { showHCEventDialogBox(true /* error */, "DISCONNECTED FROM SERVER"); },

            [&](const HexagonClient::ERegistrationSuccess&)
        { showHCEventDialogBox(false /* error */, "REGISTRATION SUCCESS"); },

            [&](const HexagonClient::ERegistrationFailure& e)
        { showHCEventDialogBox(true /* error */, "REGISTRATION FAILURE", e.error); },

            [&](const HexagonClient::ELoginSuccess&)
        {
            showHCEventDialogBox(false /* error */, "LOGIN SUCCESS");
            steamManager.unlock_achievement("a23_login");
        },

            [&](const HexagonClient::ELoginFailure& e)
        { showHCEventDialogBox(true /* error */, "LOGIN FAILURE", e.error); },

            [&](const HexagonClient::ELogoutSuccess&) { showHCEventDialogBox(false /* error */, "LOGOUT SUCCESS"); },

            [&](const HexagonClient::ELogoutFailure&) { showHCEventDialogBox(true /* error */, "LOGOUT FAILURE"); },

            [&](const HexagonClient::EDeleteAccountSuccess&)
        { showHCEventDialogBox(false /* error */, "DELETE ACCOUNT SUCCESS"); },

            [&](const HexagonClient::EDeleteAccountFailure& e)
        { showHCEventDialogBox(true /* error */, "DELETE ACCOUNT FAILURE", e.error); },

            [&](const HexagonClient::EReceivedTopScores& e)
        { leaderboardCache->receivedScores(e.levelValidator, e.scores); },

            [&](const HexagonClient::EReceivedOwnScore& e)
        { leaderboardCache->receivedOwnScore(e.levelValidator, e.score); },

            [&](const HexagonClient::EGameVersionMismatch&)
        {
            hg::lo("hg::MenuGame::update")
                << "Client/server game version mismatch, likely not a "
                   "problem\n";
        },

            [&](const HexagonClient::EProtocolVersionMismatch&)
        { showHCEventDialogBox(true /* error */, "CLIENT/SERVER PROTOCOL VERSION MISMATCH"); }

            //
        );
    }

    if (fnHGUpdateRichPresenceCallbacks)
    {
        fnHGUpdateRichPresenceCallbacks();
    }

    Joystick::update(Config::getJoystickDeadzone());

    // Focus should have no effect if we are in the favorites menu
    // or a pack change animation is in progress.
    if (state == States::LevelSelection && !isFavoriteLevels() && packChangeState == PackChange::Rest)
    {
        if (!focusHeld)
        {
            focusHeld = Joystick::pressed(Joystick::Jid::Focus);
        }

        // If focus was not pressed it means we have initiated a quick
        // pack switch.
        if (focusHeld && !wasFocusHeld)
        {
            if (lvlDrawer->currentIndex != 0)
            {
                setIndex(0);
            }
            quickPackFoldStretch();
        }
        else if (!focusHeld && wasFocusHeld)
        {
            // focus is now released we have to stretch the level list of the
            // current menu.
            quickPackFoldStretch();
        }
        wasFocusHeld = focusHeld;
    }
    else
    {
        wasFocusHeld = focusHeld = false;
    }

    // TODO (P2): cleanup mouse control
    if ((state == States::SMain || state == States::MOpts || state == States::MOnline || state == States::SLPSelect) &&
        mustUseMenuItem.hasValue())
    {
        if (getCurrentMenu() != nullptr)
        {
            auto& items = getCurrentMenu()->getItems();
            if (static_cast<int>(items.size()) > *mustUseMenuItem)
            {
                ssvms::ItemBase& item = *items[*mustUseMenuItem];

                if (item.isEnabled())
                {
                    playSoundOverride("beep.ogg");
                    item.exec();
                }
            }
        }

        mustUseMenuItem.reset();
    }

    // TODO (P2): cleanup mouse control
    if (state == States::LevelSelection && packChangeState == PackChange::Rest)
    {
        if (mustFavorite)
        {
            addRemoveFavoriteLevel();
            mustFavorite = false;
        }
        else if (mustPlay)
        {
            playSelectedLevel();
            mustPlay = false;
        }
        else if (mustChangeIndexTo.hasValue())
        {
            if (lvlDrawer != nullptr && lvlDrawer->currentIndex != *mustChangeIndexTo)
            {
                playSoundOverride("beep.ogg");
                setIndex(*mustChangeIndexTo);
            }

            mustChangeIndexTo.reset();
        }
        else if (mustChangePackIndexTo.hasValue())
        {
            if (lvlSlct.packIdx != *mustChangePackIndexTo)
            {
                playSoundOverride("beep.ogg");
                changePackTo(*mustChangePackIndexTo);
            }

            mustChangePackIndexTo.reset();
        }
    }

    if (Joystick::risingEdge(Joystick::Jid::NextPack))
    {
        changePackAction(1);
    }
    else if (Joystick::risingEdge(Joystick::Jid::PreviousPack))
    {
        changePackAction(-1);
    }

    if (Joystick::risingEdge(Joystick::Jid::AddToFavorites))
    {
        addRemoveFavoriteLevel();
    }

    if (Joystick::risingEdge(Joystick::Jid::FavoritesMenu))
    {
        switchToFromFavoriteLevels();
    }

    if (Joystick::risingEdge(Joystick::Jdir::Left))
    {
        leftAction();
    }
    else if (Joystick::risingEdge(Joystick::Jdir::Right))
    {
        rightAction();
    }
    else if (Joystick::risingEdge(Joystick::Jdir::Up))
    {
        upAction();
    }
    else if (Joystick::risingEdge(Joystick::Jdir::Down))
    {
        downAction();
    }

    if (Joystick::risingEdge(Joystick::Jid::Select))
    {
        okAction();
    }
    else if (Joystick::risingEdge(Joystick::Jid::Exit))
    {
        exitAction();
    }

    if (Joystick::risingEdge(Joystick::Jid::Screenshot))
    {
        mustTakeScreenshot = true;
    }

    if (touchDelay > 0.f)
    {
        touchDelay -= mFT;
    }

    if (dialogBoxDelay > 0.f)
    {
        dialogBoxDelay -= mFT;
    }

    if (difficultyBumpEffect > 0.f)
    {
        difficultyBumpEffect -= mFT * 2.f;
    }

    if (window.getFingerDownCount() == 1)
    {
        auto wThird{getWindowWidth() / 3.f};
        auto wLT{getWindowWidth() - wThird};
        auto hThird{getWindowHeight() / 3.f};
        auto hLT{getWindowHeight() - hThird};

        for (const auto& p : window.getFingerDownPositions())
        {
            if (p.y > hThird && p.y < hLT)
            {
                if (p.x > 0.f && p.x < wThird)
                {
                    leftAction();
                }
                else if (p.x < ssvu::toNum<int>(getWindowWidth()) && p.x > wLT)
                {
                    rightAction();
                }
                else if (p.x > wThird && p.x < wLT)
                {
                    okAction();
                }
            }
            else
            {
                if (p.y < hThird)
                {
                    upAction();
                }
                else if (p.y > hLT)
                {
                    downAction();
                }
            }
        }
    }

    if (getCurrentMenu() != nullptr)
    {
        getCurrentMenu()->update();
    }

    currentCreditsId += mFT;
    txCreditsBar2 = &assets.getTexture(creditsIds[static_cast<int>(currentCreditsId / 100) % creditsIds.size()]);
    creditsBar2.textureRect = txCreditsBar2->getRect();

    if (exitTimer > 20)
    {
        window.stop();
    }

    styleData.update(mFT);
    backgroundCamera.rotation += sf::degrees(levelStatus.rotationSpeed * 10.f * mFT);

    if (isEnteringText())
    {
        constexpr unsigned int limit{18u};
        for (auto& c : enteredChars)
        {
            if (enteredStr.size() < limit && (ssvu::isAlphanumeric(c) || ssvu::isPunctuation(c)))
            {
                playSoundOverride("beep.ogg");
                enteredStr.append(ssvu::toStr(c));
            }
        }
    }
    enteredChars.clear();

    switch (state)
    {
        case States::LoadingScreen:
            hexagonRotation += mFT / 100.f;
            break;

        case States::LevelSelection:
        {
            // Folding animation of the level list when we change pack.
            switch (packChangeState)
            {
                case PackChange::Rest:
                    scrollLevelListToTargetY(mFT);
                    break;

                case PackChange::Folding:
                {
                    const float listHeight{getLevelListHeight()};
                    // Change the offset of the level list of the current pack
                    // to fold.
                    packChangeOffset = std::min(packChangeOffset + mFT * scrollSpeed, listHeight);
                    // If needed scroll the level list offset to show the
                    // current level.
                    calcPackChangeScrollFold(listHeight);
                    if (packChangeOffset < listHeight)
                    {
                        break;
                    }

                    // Change the pack
                    changePack();

                    // Set the stretch info
                    packChangeOffset = getLevelListHeight();
                    calcScrollSpeed();
                    packChangeState = PackChange::Stretching;
                }
                break;

                case PackChange::Stretching:
                    // Change the offset of the level list of the current pack
                    // to stretch.
                    packChangeOffset = std::max(packChangeOffset - mFT * scrollSpeed, 0.f);
                    // If needed scroll the level list offset to show the
                    // current level.
                    calcPackChangeScrollStretch(getLevelListHeight());
                    if (packChangeOffset > 0.f)
                    {
                        break;
                    }

                    packChangeOffset = 0.f;
                    adjustLevelsOffset();
                    packChangeState = PackChange::Rest;
                    break;
            }

            // Make sure there isn't empty space above the first
            // element of the level list.
            lvlDrawer->YOffset = std::min(lvlDrawer->YOffset, 0.f);
        }
        break;

        default:
            break;
    }
}

void MenuGame::setIndex(const int mIdx)
{
    lvlDrawer->currentIndex = mIdx;

    const sf::base::String levelID{(*lvlDrawer->levelDataIds)[lvlDrawer->currentIndex]};

    levelData   = &assets.getLevelData(levelID);
    currentPack = &assets.getPackData(levelData->packId);

    formatLevelDescription();

    styleData = assets.getStyleData(levelData->packId, levelData->styleId);
    styleData.computeColors();

    // If we are in the favorite menu we must find the packId relative
    // to the selected level.
    if (isFavoriteLevels())
    {
        isLevelFavorite = true;

        const auto& p{assets.getSelectablePackInfos()};

        for (int i{0}; i < static_cast<int>(p.size()); ++i)
        {
            if (levelData->packId == p[i].id)
            {
                lvlDrawer->packIdx = i;
                break;
            }
        }
    }
    else if (!firstLevelSelection)
    {
        // setIndex() is called for the first time at a point
        // of the assets loading process where this call
        // causes a crash, so it can only safely occur when
        // the level selection menu is opened.
        isLevelFavorite = assets.getCurrentLocalProfile().isLevelFavorite(levelID);
    }

    // Set the colors of the menus
    auto& colors{styleData.getColors()};
    menuQuadColor = Config::getBlackAndWhite() ? sf::Color(20, 20, 20, 255) : styleData.getTextColor();
    if (static_cast<int>(menuQuadColor.a) == 0 && !Config::getBlackAndWhite())
    {
        for (auto& c : colors)
        {
            if (static_cast<int>(c.a) != 0)
            {
                menuQuadColor = c;
                break;
            }
        }
    }
    menuTextColor = Config::getBlackAndWhite() ? sf::Color::White : colors[0];

    dialogBoxTextColor   = menuQuadColor;
    dialogBoxTextColor.a = 255;

    // If there is only one color set the remaining colors
    // to be alpha variants of the ones we have already set.
    if (colors.size() == 1)
    {
        menuTextColor.a      = 255;
        menuSelectionColor   = menuQuadColor;
        menuSelectionColor.a = 75;
    }
    else
    {
        // If the alpha is 0 or the color is the same find another one.
        if (static_cast<int>(menuTextColor.a) == 0 || menuTextColor == menuQuadColor)
        {
            for (auto& c : colors)
            {
                if (static_cast<int>(c.a) != 0 && c != menuQuadColor && !Config::getBlackAndWhite())
                {
                    menuTextColor = c;
                    break;
                }
            }
        }

        // Same as above.
        menuSelectionColor = Config::getBlackAndWhite() ? sf::Color::White : colors[1];
        if (static_cast<int>(menuSelectionColor.a) == 0 || menuSelectionColor == menuQuadColor ||
            menuSelectionColor == menuTextColor)
        {
            for (auto& c : colors)
            {
                if (static_cast<int>(c.a) != 0 && c != menuQuadColor && c != menuTextColor && !Config::getBlackAndWhite())
                {
                    menuSelectionColor = c;
                    break;
                }
            }
        }
        menuSelectionColor.a = 175;
    }

    // Set color of the fonts.
    txtSelectionBig.font.setFillColor(menuQuadColor);
    txtSelectionSmall.font.setFillColor(menuTextColor);
    txtSelectionRanked.font.setFillColor(menuTextColor);
    txtInstructionsSmall.font.setFillColor(menuTextColor);
    txtSelectionScore.font.setFillColor(menuTextColor);

    // Set gameplay values
    diffMultIdx = 0;
    for (; levelData->difficultyMults[diffMultIdx] != 1.f; ++diffMultIdx)
    {
    }

    try
    {
        runLuaFile(levelData->luaScriptPath);
        Utils::runVoidLuaFunctionIfExists(lua, "onInit");
        Utils::runVoidLuaFunctionIfExists(lua, "onLoad");
    } catch (std::runtime_error& mError)
    {
        std::cout << "[MenuGame::init] Runtime Lua error on menu "
                     "(loadFile/onInit/onLoad) with level \""
                  << levelData->name << "\": \n"
                  << mError.what() << '\n'
                  << std::endl;

        if (!Config::getDebug())
        {
            playSoundOverride("error.ogg");
        }
    } catch (...)
    {
        std::cout << "[MenuGame::init] Unknown runtime Lua error on menu "
                     "(loadFile/onInit/onLoad) with level \""
                  << levelData->name << "\"\n"
                  << std::endl;

        if (!Config::getDebug())
        {
            playSoundOverride("error.ogg");
        }
    }
}

void MenuGame::reloadAssets(const bool reloadEntirePack)
{
    if (state != States::LevelSelection || !dialogBox.empty() || !Config::getDebug())
    {
        return;
    }

    assets.reloadAllShaders();

    // Do the necessary asset reload operation and get the log
    // of the results.
    sf::base::String reloadOutput;
    if (reloadEntirePack)
    {
        reloadOutput = assets.reloadPack(levelData->packId, levelData->packPath);
    }
    else
    {
        reloadOutput = assets.reloadLevel(levelData->packId, levelData->packPath, levelData->id);
    }

    setIndex(lvlDrawer->currentIndex); // loads the new levelData

    reloadOutput += "\nPRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n";
    Utils::uppercasify(reloadOutput);

    // Needs to be two because the dialog box reacts to key releases.
    // First key release is the one of the key press that made the dialog
    // box pop up, the second one belongs to the key press that closes it
    playSoundOverride("select.ogg");
    showDialogBox(reloadOutput);
    setIgnoreAllInputs(2);
}

void MenuGame::refreshCamera()
{
    const float fw{1024.f / getWindowWidth()};
    const float fh{768.f / getWindowHeight()};
    const float fmax{std::max(fw, fh)};

    w = getWindowWidth() * fmax;
    h = getWindowHeight() * fmax;

    backgroundCamera = {
        sf::View{.center = sf::Vec2f{0.f, 0.f},
                 .size = {Config::getSizeX() * Config::getZoomFactor(), Config::getSizeY() * Config::getZoomFactor()}}};

    overlayCamera = sf::View{.center = {w / 2.f, h / 2.f}, .size = {w, h}};

    titleBar.origin   = sf::Vec2f{0.f, 0.f};
    titleBar.scale    = {0.5f, 0.5f};
    titleBar.position = {20.f, 20.f};

    txtVersion.font.setString(GAME_VERSION_STR);
    txtVersion.font.origin   = {txtVersion.font.getLocalRight(), 0.f};
    txtVersion.font.position = {titleBar.getGlobalRight() - 15.f, titleBar.getGlobalTop() + 15.f};

    creditsBar1.origin   = {creditsBar1.getLocalBounds().size.x, 0.f};
    creditsBar1.scale    = {0.373f, 0.373f};
    creditsBar1.position = {w - 20.f, 20.f};

    creditsBar2.origin   = {creditsBar2.getLocalBounds().size.x, 0};
    creditsBar2.scale    = {0.373f, 0.373f};
    creditsBar2.position = {w - 20.f, 17.f + creditsBar1.getGlobalBottom()};

    const float scaleFactor{w / 1024.f};
    epilepsyWarning.origin   = epilepsyWarning.getLocalCenter();
    epilepsyWarning.position = {1024 / (2.f / scaleFactor), 768 / 2.f - 50};
    epilepsyWarning.scale    = {0.36f, 0.36f};

    // Readjust the menu background skew and the indents
    fourByThree = 10.f * getWindowWidth() / getWindowHeight() < 16;

    if (fourByThree)
    {
        backgroundCameraTransform.skew = {1.f, 0.8f};
    }
    else
    {
        backgroundCameraTransform.skew = {1.f, 0.6f};
    }

    for (auto& c : mainMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }

    for (auto& c : welcomeMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }

    for (auto& c : optionsMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }

    for (auto& c : onlineMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }

    for (auto& c : profileSelectionMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }

    // Update the height infos of the fonts.
    const auto setMenuFontVisualSize = [](MenuFont& menuFont, const unsigned int characterSize)
    { setVisualCharacterSize(menuFont.font, characterSize); };

    if (fourByThree)
    {
        setMenuFontVisualSize(txtMenuBig, 26);
        setMenuFontVisualSize(txtMenuTiny, 9);
        setMenuFontVisualSize(txtMenuSmall, 16);

        setMenuFontVisualSize(txtSelectionBig, 24);
        setMenuFontVisualSize(txtSelectionSmall, 14);
        setMenuFontVisualSize(txtSelectionRanked, 10);

        setMenuFontVisualSize(txtLoadBig, 56);
        setMenuFontVisualSize(txtLoadSmall, 16);

        setMenuFontVisualSize(txtRandomTip, 24);
    }
    else
    {
        setMenuFontVisualSize(txtMenuBig, 36);
        setMenuFontVisualSize(txtMenuTiny, 14);
        setMenuFontVisualSize(txtMenuSmall, 24);

        setMenuFontVisualSize(txtSelectionBig, 28);
        setMenuFontVisualSize(txtSelectionSmall, 14);
        setMenuFontVisualSize(txtSelectionRanked, 10);

        setMenuFontVisualSize(txtLoadBig, 70);
        setMenuFontVisualSize(txtLoadSmall, 24);

        setMenuFontVisualSize(txtRandomTip, 32);
    }

    // txtVersion and txtProfile are not in here cause they do not need it.
    for (auto f :
         {&txtProf,
          &txtLoadBig,
          &txtLoadSmall,
          &txtMenuBig,
          &txtMenuTiny,
          &txtMenuSmall,
          &txtInstructionsBig,
          &txtRandomTip,
          &txtInstructionsMedium,
          &txtInstructionsSmall,
          &txtEnteringText,
          &txtSelectionBig,
          &txtSelectionMedium,
          &txtSelectionSmall,
          &txtSelectionScore,
          &txtSelectionRanked})
    {
        f->updateHeight();
    }

    // Readjust the level selection drawing parameters
    updateLevelSelectionDrawingParameters();

    // Reformat the level description, but not on boot.
    // Otherwise the game crashes.
    if (!firstLevelSelection)
    {
        formatLevelDescription();
    }
}
void MenuGame::renderText(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos)
{
    mText.setString(mStr);
    mText.position = mPos;
    drawOverlay(mText);
}

void MenuGame::renderText(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos, const sf::Color& mColor)
{
    const sf::Color prevColor = mText.getFillColor();
    mText.setFillColor(mColor);
    renderText(mStr, mText, mPos);
    mText.setFillColor(prevColor);
}

void MenuGame::renderText(const sf::base::String& mStr, sf::Text& mText, const unsigned int mSize, const sf::Vec2f mPos)
{
    const sf::Vec2f prevScale = mText.scale;
    setVisualCharacterSize(mText, mSize);
    renderText(mStr, mText, mPos);
    mText.scale = prevScale;
}

void MenuGame::renderText(const sf::base::String& mStr,
                          sf::Text&               mText,
                          const unsigned int      mSize,
                          const sf::Vec2f         mPos,
                          const sf::Color&        mColor)
{
    const sf::Vec2f prevScale = mText.scale;
    setVisualCharacterSize(mText, mSize);
    const sf::Color prevColor = mText.getFillColor();
    mText.setFillColor(mColor);
    renderText(mStr, mText, mPos);
    mText.setFillColor(prevColor);
    mText.scale = prevScale;
}

// Text rendering centered
void MenuGame::renderTextCentered(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos)
{
    mText.setString(mStr);
    mText.position = {mPos.x - mText.getGlobalWidth() / 2.f, mPos.y};
    drawOverlay(mText);
}

void MenuGame::renderTextCentered(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos, const sf::Color& mColor)
{
    const sf::Color prevColor = mText.getFillColor();
    mText.setFillColor(mColor);
    renderTextCentered(mStr, mText, mPos);
    mText.setFillColor(prevColor);
}

void MenuGame::renderTextCentered(const sf::base::String& mStr, sf::Text& mText, const unsigned int mSize, const sf::Vec2f mPos)
{
    const sf::Vec2f prevScale = mText.scale;
    setVisualCharacterSize(mText, mSize);
    renderTextCentered(mStr, mText, mPos);
    mText.scale = prevScale;
}

void MenuGame::renderTextCentered(const sf::base::String& mStr,
                                  sf::Text&               mText,
                                  const unsigned int      mSize,
                                  const sf::Vec2f         mPos,
                                  const sf::Color&        mColor)
{
    const sf::Vec2f prevScale = mText.scale;
    setVisualCharacterSize(mText, mSize);
    const sf::Color prevColor = mText.getFillColor();
    mText.setFillColor(mColor);
    renderTextCentered(mStr, mText, mPos);
    mText.setFillColor(prevColor);
    mText.scale = prevScale;
}

// Text rendering centered with an offset
void MenuGame::renderTextCenteredOffset(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos, const float xOffset)
{
    mText.setString(mStr);
    mText.position = {xOffset + mPos.x - mText.getGlobalWidth() / 2.f, mPos.y};
    drawOverlay(mText);
}

void MenuGame::renderTextCenteredOffset(const sf::base::String& mStr,
                                        sf::Text&               mText,
                                        const sf::Vec2f         mPos,
                                        const float             xOffset,
                                        const sf::Color&        mColor)
{
    const sf::Color prevColor = mText.getFillColor();
    mText.setFillColor(mColor);
    renderTextCenteredOffset(mStr, mText, mPos, xOffset);
    mText.setFillColor(prevColor);
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
    adjustLevelsOffset();
    lvlDrawer->XOffset = 0.f;
    setIgnoreAllInputs(1); // otherwise you go back to the main menu

    // Steer the post-gameplay menu back to the new UI's LevelSelect screen
    // when the new UI is active. Use `changeStateTo` (not a raw assignment)
    // so any state-transition side effects fire.
    if (useNewUI)
    {
        if (state != States::SMain)
        {
            changeStateTo(States::SMain);
        }
        ui_app.current = hg::ui::Screen::LevelSelect;
        ui_app.backStack.clear();
        ui_app.backStack.emplaceBack(hg::ui::Screen::Main);

        hg::lo("MenuGame::returnToLevelSelection") << "[newUI] state=SMain, app.current=LevelSelect" << logEndl;
    }
}


void MenuGame::refreshBinds()
{
    // Keyboard-mouse
    for (sf::base::SizeT i{0u}; i < Config::triggerGetters.size(); ++i)
    {
        game.refreshTrigger(Config::triggerGetters[i](), i);

        if (fnHGTriggerRefresh)
        {
            fnHGTriggerRefresh(Config::triggerGetters[i](), i);
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

void MenuGame::adjustMenuOffset(const bool resetMenuOffset)
{
    if (getCurrentMenu() == nullptr)
    {
        return;
    }

    // Set to 0 the offset of the whole submenu.
    if (resetMenuOffset)
    {
        getCurrentMenu()->getCategory().getOffset() = 0.f;
    }

    // Set to 0 the offset of all elements except the one that is
    // currently selected.
    const auto& items{getCurrentMenu()->getItems()};
    for (auto& i : items)
    {
        i->getOffset() = 0.f;
    }

    items[getCurrentMenu()->getIdx()]->getOffset() = maxOffset;
}

void MenuGame::adjustLevelsOffset()
{
    // Set all level offsets to 0 except the currently selected one.
    for (auto& offset : lvlSlct.lvlOffsets)
    {
        offset = 0.f;
    }

    lvlSlct.lvlOffsets[lvlSlct.currentIndex] = maxOffset;

    // Do the same for the favorites menu but only if there are levels in it.
    if (!favSlct.lvlOffsets.size())
    {
        return;
    }

    for (auto& offset : favSlct.lvlOffsets)
    {
        offset = 0.f;
    }

    favSlct.lvlOffsets[favSlct.currentIndex] = maxOffset;
}

inline constexpr float offsetSpeed{4.f};
inline constexpr float offsetSnap{0.25f};

[[nodiscard]] float MenuGame::calcMenuOffset(float& offset, const float maxOffset, const bool revertOffset, const bool speedUp)
{
    // Adjust the offset of the menu depending on whether it
    // is being opened or closed.

    float speed;
    if (revertOffset)
    {
        // FPS consistent offset speed
        speed = (speedUp ? 12.f : 8.f) * offsetSpeed * getFPSMult();

        offset = std::max(offset - speed, 0.f);
        if (offset <= offsetSnap)
        {
            offset = 0.f;
        }
    }
    else if (offset < maxOffset)
    {
        speed = (speedUp ? 12.f : 8.f) * offsetSpeed * getFPSMult();
        offset += speed * (1.f - offset / maxOffset);
        if (offset >= maxOffset - offsetSnap)
        {
            offset = maxOffset;
        }
    }

    return maxOffset - offset;
}

void MenuGame::calcMenuItemOffset(float& offset, bool selected)
{
    // Same as above but for menu items that are selected and deselected.
    float speed;
    if (selected)
    {
        if (offset < maxOffset)
        {
            speed = offsetSpeed * getFPSMult();
            offset += speed * (1.f - offset / maxOffset);
            if (offset >= maxOffset - offsetSnap)
            {
                offset = maxOffset;
            }
        }
    }
    else if (offset > 0.f)
    {
        speed = offsetSpeed * getFPSMult();
        offset -= 1.5f * speed * offset / maxOffset;
        if (offset <= offsetSnap)
        {
            offset = 0.f;
        }
    }
}

void MenuGame::createQuad(const sf::Color& color, const float x1, const float x2, const float y1, const float y2)
{
    sf::Vec2f nw{x1, y1}, ne{x2, y1}, se{x2, y2}, sw{x1, y2};
    menuQuads.batch_unsafe_emplace_back_quad(color, nw, sw, se, ne);
}

void MenuGame::createQuad(const sf::Color& color, const sf::Vec2f mins, const sf::Vec2f maxs)
{
    createQuad(color, mins.x, maxs.x, mins.y, maxs.y);
}

void MenuGame::createQuadTrapezoid(
    const sf::Color& color,
    const float      x1,
    const float      x2,
    const float      x3,
    const float      y1,
    const float      y2,
    const bool       left)
{
    sf::Vec2f nw, ne, se, sw;

    if (left)
    {
        nw = {x1, y1};
        ne = {x2, y1};
        se = {x3, y2};
        sw = {x1, y2};
    }
    else
    {
        nw = {x1, y1};
        ne = {x2, y1};
        se = {x2, y2};
        sw = {x3, y2};
    }

    menuQuads.batch_unsafe_emplace_back_quad(color, nw, sw, se, ne);
}

[[nodiscard]] std::pair<int, int> MenuGame::getScrollbarNotches(const int size, const int maxSize) const
{
    if (size > maxSize)
    {
        return {size - maxSize, maxSize};
    }

    return {0, size};
}

void MenuGame::drawScrollbar(const float      totalHeight,
                             const int        size,
                             const int        notches,
                             const float      x,
                             const float      y,
                             const sf::Color& color)
{
    // Draw a scrollbar depending on the total amount of elements
    // and the total height of the portion of screen where the scrollbar
    // is drawn.

    const float notchHeight{totalHeight / size}, barHeight{totalHeight - notches * notchHeight},
        startHeight{y + notchHeight * scrollbarOffset};

    menuQuads.clear();
    menuQuads.reserve_quad(1);
    createQuad(color, x, x + textToQuadBorder, startHeight, startHeight + barHeight);
    drawOverlay(menuQuads);
}

void MenuGame::drawMainSubmenus(const sf::base::Vector<sf::base::UniquePtr<ssvms::Category>>& subMenus, const float indent)
{
    bool currentlySelected, hasOffset;
    for (auto& c : subMenus)
    {
        currentlySelected = mainMenu.getCategory().getName() == c->getName();
        hasOffset         = c->getOffset() != 0.f;

        // this submenu has been fully folded so there is no need to draw it.
        if (!currentlySelected && !hasOffset)
        {
            continue;
        }

        drawMainMenu(*c, w - indent, !currentlySelected && hasOffset);
    }
}

void MenuGame::drawSubmenusSmall(const sf::base::Vector<sf::base::UniquePtr<ssvms::Category>>& subMenus, const float indent)
{
    bool currentlySelected, hasOffset;
    for (auto& c : subMenus)
    {
        // Already drawn before this loop
        if (c->getName() == "options")
        {
            continue;
        }

        currentlySelected = optionsMenu.getCategory().getName() == c->getName();
        hasOffset         = c->getOffset() != 0.f;

        // this submenu has been fully folded
        if (!currentlySelected && !hasOffset)
        {
            continue;
        }

        drawOptionsSubmenus(*c, indent, !currentlySelected && hasOffset);
    }
}

// What is this font * fontHeightOffset thing about?
// The font used has some empty space on top that causes text to be drawn
// below what you would expect according to the chosen coordinates.
// Therefore, if we assume the text has to be drawn at 'Y' height the
// actual value to be sent to the renderer is 'Y - fontHeight *
// fontHeightOffset.
inline constexpr float fontHeightOffset{0.55f};
inline constexpr float frameSizeMulti{0.6f};

void MenuGame::setMouseCursorVisible(const bool x)
{
    window.setMouseCursorVisible(x);
    mouseCursorVisible = x;
}

[[nodiscard]] bool MenuGame::isMouseCursorVisible() const
{
    return mouseCursorVisible;
}

[[nodiscard]] bool MenuGame::overlayMouseOverlap(const sf::Vec2f mins, const sf::Vec2f maxs) const
{
    constexpr float tolerance = 1.f;

    if (!isMouseCursorVisible() || !window.hasFocus())
    {
        return false;
    }

    const sf::Vec2f mp = overlayCamera.screenToWorld(sf::Mouse::getPosition(window.getRenderWindow()).to<sf::Vec2f>(),
                                                     window.getRenderWindow().getSize().to<sf::Vec2f>());

    return mp.x > mins.x - tolerance && mp.x < maxs.x + tolerance && mp.y > mins.y - tolerance && mp.y < maxs.y + tolerance;
}

[[nodiscard]] bool MenuGame::overlayMouseOverlapAndUpdateHover(const sf::Vec2f mins, const sf::Vec2f maxs)
{
    if (overlayMouseOverlap(mins, maxs))
    {
        mouseHovering = true;
        return true;
    }

    return false;
}

[[nodiscard]] sf::Color MenuGame::mouseOverlapColor(const bool mouseOverlap, const sf::Color& c) const
{
    if (!mouseOverlap)
    {
        return c;
    }

    return sf::Color{
        static_cast<sf::base::U8>(255 - c.r), //
        static_cast<sf::base::U8>(255 - c.g), //
        static_cast<sf::base::U8>(255 - c.b)  //
    };
}

[[nodiscard]] bool MenuGame::mouseLeftRisingEdge() const
{
    return !mouseWasPressed && mousePressed;
}

void MenuGame::drawMainMenu(ssvms::Category& mSubMenu, float baseIndent, const bool revertOffset)
{
    const auto& items(mSubMenu.getItems());
    const int   size = items.size();

    // Global menu offset
    const float panelOffset{calcMenuOffset(mSubMenu.getOffset(), -(baseIndent - w), revertOffset)};
    baseIndent += panelOffset;

    // Calculate quads coordinates.
    // We use font height as our reference parameter.
    const float interline{3.f * txtMenuBig.height}, quadBorder{txtMenuBig.height * frameSizeMulti},
        doubleBorder{2.f * quadBorder}, totalHeight{interline * (size - 1) + doubleBorder + txtMenuBig.height};
    float quadHeight{(h - totalHeight) / 2.f + interline - quadBorder},
        txtHeight{quadHeight - txtMenuBig.height * fontHeightOffset + quadBorder}, indent;

    // Store info needed to draw the submenus
    menuHalfHeight = quadHeight + totalHeight / 2.f;

    // Draw the quads that surround the text
    menuQuads.clear();
    menuQuads.reserve_quad(size);

    static sf::base::Vector<bool> mouseOverlaps;
    mouseOverlaps.resize(size);

    for (int i{0}; i < size; ++i)
    {
        calcMenuItemOffset(items[i]->getOffset(), i == mSubMenu.getIdx());
        indent = baseIndent - items[i]->getOffset();

        const sf::Vec2f bodyMins{indent - txtMenuBig.height * 2.5f, quadHeight};

        const sf::Vec2f bodyMaxs{w, quadHeight + doubleBorder + txtMenuBig.height};

        const bool mouseOverlap = overlayMouseOverlapAndUpdateHover(bodyMins, bodyMaxs);

        mouseOverlaps[i] = mouseOverlap;

        sf::Color c = !items[i]->isEnabled() ? sf::Color{110, 110, 110, 255} : menuQuadColor;

        createQuadTrapezoid(mouseOverlapColor(mouseOverlap, c),
                            indent - txtMenuBig.height * 2.5f,
                            w,
                            indent - txtMenuBig.height / 2.f,
                            quadHeight,
                            quadHeight + doubleBorder + txtMenuBig.height,
                            false);

        // TODO (P2): cleanup mouse control
        if (mouseOverlap && !mustUseMenuItem.hasValue() && mouseLeftRisingEdge() && items[i]->isEnabled())
        {
            mustUseMenuItem.emplace(i);
        }

        quadHeight += interline;
    }

    drawOverlay(menuQuads);

    // Draw the text on top of the quads
    for (int i{0}; i < size; ++i)
    {
        indent = baseIndent - items[i]->getOffset();

        const sf::Color c = mouseOverlapColor(mouseOverlaps[i],
                                              !items[i]->isEnabled() ? sf::Color{150, 150, 150, 255} : menuTextColor);

        renderText(items[i]->getName(), txtMenuBig.font, {indent, txtHeight}, c);

        txtHeight += interline;
    }
}

void MenuGame::drawOptionsSubmenus(ssvms::Category& mSubMenu, const float baseIndent, const bool revertOffset)
{
    const auto& items{mSubMenu.getItems()};
    const int   size(items.size());

    // Calculate quads coordinates
    float       quadBorder{txtMenuSmall.height * frameSizeMulti};
    const float doubleBorder{quadBorder * 2.f};
    const float interline{2.1f * txtMenuSmall.height};
    const float totalHeight{interline * (size - 1) + 2.f * doubleBorder + txtMenuSmall.height};
    const float quadHeight{std::max(menuHalfHeight - totalHeight / 2.f, creditsBar2.getGlobalBottom() + 10.f)};

    // Offset
    const float panelOffset{calcMenuOffset(mSubMenu.getOffset(), baseIndent, revertOffset, true)};
    const float indent{baseIndent - quadBorder - panelOffset};

    // Draw the quads that surround the text
    menuQuads.clear();
    menuQuads.reserve_quad(2);

    createQuad(menuTextColor, 0, indent + doubleBorder, quadHeight, quadHeight + totalHeight);
    createQuad(menuQuadColor, 0, indent + quadBorder, quadHeight + quadBorder, quadHeight + totalHeight - quadBorder);
    drawOverlay(menuQuads);

    // Draw the text on top of the quads
    quadBorder = quadBorder * 1.5f - panelOffset;
    sf::base::String itemName;
    float            txtHeight{quadHeight - txtMenuSmall.height * fontHeightOffset + doubleBorder};
    for (int i{0}; i < size; ++i)
    {
        SSVOH_ASSERT(i < static_cast<int>(items.size()));
        itemName = items[i]->getName();

        Utils::uppercasify(itemName);
        if (i == mSubMenu.getIdx())
        {
            itemName = "> " + itemName;
        }

        renderText(itemName,
                   txtMenuSmall.font,
                   {quadBorder, txtHeight},
                   !items[i]->isEnabled() ? sf::Color{150, 150, 150, 255} : menuTextColor);

        if (!items[i]->isEnabled())
        {
            renderText("[OFFICIAL MODE ENABLED]",
                       txtMenuTiny.font,
                       {txtMenuSmall.font.getGlobalRight() + 6.f, txtMenuSmall.font.getGlobalTop() - 2.f},
                       sf::Color{150, 150, 150, 255});
        }

        txtHeight += interline;
    }
}

sf::base::String MenuGame::formatSurvivalTime(ProfileData* data)
{
    int time{0};
    for (auto& s : data->getScores())
    {
        time += s.second;
    }

    std::stringstream stream;
    stream << "Total survival time ";
    if (time < 60)
    {
        stream << std::setfill('0') << std::setw(2) << time;
    }
    else if (time < 3600)
    {
        stream << std::setfill('0') << std::setw(2) << time / 60 << ":" << std::setfill('0') << std::setw(2) << time % 60;
    }
    else
    {
        stream << time / 3600 << ":";
        time %= 3600;
        stream << std::setfill('0') << std::setw(2) << time / 60 << ":" << std::setfill('0') << std::setw(2) << time % 60;
    }

    return sf::base::String(stream.str());
}

inline constexpr float        profFrameSize{10.f};
inline constexpr unsigned int profCharSize{30};
inline constexpr unsigned int profSelectedCharSize{30 + 12};

void MenuGame::drawProfileSelection(const float xOffset, const bool revertOffset)
{
    ssvms::Category& mSubmenu{profileSelectionMenu.getCategory()};
    const auto&      items{mSubmenu.getItems()};

    const int realSize(items.size());
    auto [scrollbarNotches, drawnSize] = getScrollbarNotches(realSize, maxProfilesOnScreen);

    // Calculate the height
    const float fontHeight{Utils::getFontHeight(txtProfile.font, profCharSize)},
        selectedFontHeight{Utils::getFontHeight(txtProfile.font, profSelectedCharSize)};

    // check if the width of the menu should be increased
    constexpr float  profMinWidth{400.f};
    float            textWidth{profMinWidth};
    sf::base::String itemName;
    for (auto& p : items)
    {
        itemName = p->getName();
        Utils::uppercasify(itemName);
        txtProfile.font.setString(itemName);
        textWidth = std::max(textWidth, txtProfile.font.getGlobalWidth());
    }

    // Calculate horizontal coordinates
    constexpr float profMinHeight{360.f};
    const float     interline{4.f * fontHeight}, doubleBorder{profFrameSize * 2.f},
        totalHeight{std::max(interline * (drawnSize - 1) + doubleBorder * 2.f + fontHeight * 3.f, profMinHeight)};

    // always account for the scrollbar space
    constexpr float scrollbarInterspace{3.f};
    textWidth += doubleBorder + scrollbarInterspace * 2.f;

    // Make sure the box does not go out of bounds
    float indent{((w + xOffset) * 0.5f - textWidth) / 2.f + profFrameSize};
    indent = std::max(indent, doubleBorder);

    // Make sure the instructions do not go out of bounds
    txtInstructionsSmall.font.setString(
        "Press backspace to delete the selected profile\n"
        "You cannot delete the profile currently in use");
    const float instructionsWidth{txtInstructionsSmall.font.getGlobalWidth()},
        resultIndent{indent + (textWidth - instructionsWidth) / 2.f};
    if (resultIndent < 0.f)
    {
        indent += -resultIndent + 10.f;
    }

    // Calculate vertical coordinates
    float quadHeight{std::max((h - totalHeight) / 2.f, titleBar.getGlobalBottom() + 60.f)},
        txtHeight{quadHeight - fontHeight * fontHeightOffset + doubleBorder + profFrameSize * 0.5f};

    // Submenu global offset
    const float panelOffset{calcMenuOffset(mSubmenu.getOffset(), h - quadHeight, revertOffset)};
    txtHeight += panelOffset;
    quadHeight += panelOffset;

    // Draw the quads that surround the text and the scroll bar if needed
    menuQuads.clear();
    menuQuads.reserve_quad(2);

    createQuad(menuTextColor, indent - doubleBorder, indent + doubleBorder + textWidth, quadHeight, quadHeight + totalHeight);

    createQuad(menuQuadColor,
               indent - profFrameSize,
               indent + profFrameSize + textWidth,
               quadHeight + profFrameSize,
               quadHeight + totalHeight - profFrameSize);

    drawOverlay(menuQuads);

    if (scrollbarNotches != 0)
    {
        drawScrollbar(totalHeight - 2.f * (profFrameSize + scrollbarInterspace),
                      realSize,
                      scrollbarNotches,
                      indent + textWidth - scrollbarInterspace,
                      quadHeight + profFrameSize + scrollbarInterspace,
                      menuTextColor);
    }

    // Draw the text on top of the quads
    float        yPos;
    bool         selected;
    ProfileData* data;
    txtHeight += profFrameSize / 2.f;
    for (int i{scrollbarOffset}; i < drawnSize + scrollbarOffset; ++i)
    {
        selected = i == mSubmenu.getIdx();

        // Draw profile name
        SSVOH_ASSERT(i < static_cast<int>(items.size()));
        itemName = items[i]->getName();

        yPos = txtHeight - (selected ? fontHeight * 0.75f : 0.f);
        renderTextCentered(Utils::toUppercase(itemName),
                           txtProfile.font,
                           selected ? profSelectedCharSize : profCharSize,
                           {indent + textWidth / 2.f, yPos},
                           menuTextColor);

        // Add total survival time for extra flavor
        data = assets.getLocalProfileByName(itemName);
        if (data != nullptr)
        {
            yPos += (selected ? selectedFontHeight : fontHeight) * 1.75f;
            renderTextCentered(formatSurvivalTime(data),
                               txtProfile.font,
                               (selected ? profSelectedCharSize : profCharSize) - 15u,
                               {indent + textWidth / 2.f, yPos});
        }

        txtHeight += interline;
    }

    // Add message about profile deletion
    txtInstructionsSmall.font.position = {indent + (textWidth - instructionsWidth) / 2.f, quadHeight + totalHeight};
    drawOverlay(txtInstructionsSmall.font);
}

void MenuGame::drawProfileSelectionBoot()
{
    ssvms::Category& mSubmenu{profileSelectionMenu.getCategory()};
    const auto&      items(mSubmenu.getItems());

    const int realSize(items.size());
    auto [scrollbarNotches, drawnSize] = getScrollbarNotches(realSize, maxProfilesOnScreen);

    const float fontHeight{Utils::getFontHeight(txtProfile.font, profCharSize)},
        selectedFontHeight{Utils::getFontHeight(txtProfile.font, profSelectedCharSize)};

    // Calculate coordinates
    const float interline{4.f * fontHeight}, totalHeight{interline * drawnSize};
    float       height{(h - totalHeight) / 2.f - selectedFontHeight * 1.5f};

    // Draw instructions
    const float instructionsHeight{1.5f * txtInstructionsBig.height};
    // Make sure the instructions do not overlap the title bar or the credits
    height = std::max(height - 2.f * instructionsHeight, titleBar.getGlobalBottom() + 40.f);

    const sf::base::String instructions[] = {"SELECT LOCAL PROFILE", "PRESS ESC TO CREATE A NEW PROFILE"};
    for (auto& s : instructions)
    {
        renderTextCentered(s, txtInstructionsBig.font, {w / 2.f, height});
        height += instructionsHeight;
    }
    height += selectedFontHeight;

    // Draw scrollbar if needed
    sf::base::String itemName;
    if (scrollbarNotches != 0)
    {
        float width = 0.f;
        for (auto& p : items)
        {
            itemName = p->getName();
            Utils::uppercasify(itemName);
            txtProfile.font.setString(itemName);
            width = std::max(width, txtProfile.font.getGlobalWidth());
        }

        txtProfile.font.setString("Total survival time 0000:00");
        width = std::max(width, txtProfile.font.getGlobalWidth());
        width += 10.f;

        drawScrollbar(totalHeight, realSize, scrollbarNotches, (w + width) / 2.f, height, sf::Color::White);
    }

    // Draw profile names and score
    bool         selected;
    float        yPos;
    ProfileData* data;
    for (int i{scrollbarOffset}; i < drawnSize + scrollbarOffset; ++i)
    {
        selected = i == mSubmenu.getIdx();

        // Draw profile name
        SSVOH_ASSERT(i < static_cast<int>(items.size()));
        itemName = items[i]->getName();

        yPos = height - (selected ? fontHeight * 0.75f : 0.f);
        renderTextCentered(Utils::toUppercase(itemName),
                           txtProfile.font,
                           selected ? profSelectedCharSize : profCharSize,
                           {w / 2.f, yPos});

        // Add total survival time for extra flavor
        data = assets.getLocalProfileByName(itemName);
        if (data != nullptr)
        {
            yPos += (selected ? selectedFontHeight : fontHeight) * 1.75f;
            renderTextCentered(formatSurvivalTime(data),
                               txtProfile.font,
                               (selected ? profSelectedCharSize : profCharSize) - 15u,
                               {w / 2.f, yPos});
        }

        height += interline;
    }
}

void MenuGame::drawEnteringText(const float xOffset, const bool revertOffset)
{
    // Set text parameters
    Utils::uppercasify(enteredStr);
    txtEnteringText.font.setString(enteredStr);
    constexpr float enteringTextMinWidth{200.f};
    const float     textWidth{std::max(enteringTextMinWidth, txtEnteringText.font.getGlobalWidth())};

    // Calculate coordinates
    const float doubleFrame{profFrameSize * 2.f}, indent{((w + xOffset) * 0.5f - textWidth) / 2.f + profFrameSize},
        txtBottom{txtEnteringText.height * 0.45f}, totalHeight{txtEnteringText.height + txtBottom + doubleFrame * 2.f};
    float quadHeight{menuHalfHeight - totalHeight / 2.f},
        txtHeight{quadHeight - txtEnteringText.height * fontHeightOffset + doubleFrame};

    // Offset
    const float panelOffset{calcMenuOffset(enteringTextOffset, h - quadHeight, revertOffset)};
    txtHeight += panelOffset;
    quadHeight += panelOffset;

    // Draw the quads that surround the text
    menuQuads.clear();
    menuQuads.reserve_quad(2);

    createQuad(menuTextColor, indent - doubleFrame, indent + doubleFrame + textWidth, quadHeight, quadHeight + totalHeight);

    createQuad(menuQuadColor,
               indent - profFrameSize,
               indent + profFrameSize + textWidth,
               quadHeight + profFrameSize,
               quadHeight + totalHeight - profFrameSize);

    drawOverlay(menuQuads);

    // Draw the text on top of the quads
    renderTextCenteredOffset(enteredStr, txtEnteringText.font, {textWidth / 2.f, txtHeight + profFrameSize / 2.f}, indent, menuTextColor);

    // Draw instructions text above the quads
    const sf::base::String instructions[] = {"INSERT TEXT", "PRESS ENTER WHEN DONE", "PRESS ESC TO ABORT"};
    const float            instructionsHeight{txtInstructionsMedium.height * 1.5f};
    txtHeight -= profFrameSize + instructionsHeight * 3.f;
    txtInstructionsMedium.font.setFillColor(menuQuadColor);
    for (auto& s : instructions)
    {
        renderTextCenteredOffset(s, txtInstructionsMedium.font, {textWidth / 2.f, txtHeight}, indent);
        txtHeight += instructionsHeight;
    }
}

void MenuGame::drawEnteringTextBoot()
{
    // Set parameters
    float height{(h - txtEnteringText.height) / 2};

    // Draw instructions text
    const float instructionsHeight{txtInstructionsBig.height * 1.5f};
    height -= instructionsHeight * 2.f;
    const sf::base::String instructions[] = {"PROFILE CREATION", "PLEASE TYPE A NAME AND PRESS ENTER"};
    for (auto& s : instructions)
    {
        renderTextCentered(s, txtInstructionsBig.font, {w / 2.f, height});
        height += instructionsHeight;
    }

    // Draw the entered name text
    height += instructionsHeight / 2.f;
    Utils::uppercasify(enteredStr);
    renderTextCentered(enteredStr, txtEnteringText.font, {w / 2.f, height}, sf::Color::White);
}

void MenuGame::drawLoadResults()
{
    //--------------------------------------
    // Hexagon
    const float     div{Utils::tau / 6 * 0.5f}, hexagonRadius{100.f};
    const sf::Vec2f centerPos = {w / 2.f, h / 5.f};

    menuQuads.clear();

    menuQuads.reserve_quad(6);
    for (int i{0}; i < 6; ++i)
    {
        const float sAngle{div * 2.f * (i + hexagonRotation)};

        const sf::Vec2f nw{centerPos.movedTowards(hexagonRadius, sf::radians(sAngle - div))};
        const sf::Vec2f ne{centerPos.movedTowards(hexagonRadius, sf::radians(sAngle + div))};
        const sf::Vec2f se{centerPos.movedTowards(hexagonRadius + 10.f, sf::radians(sAngle + div))};
        const sf::Vec2f sw{centerPos.movedTowards(hexagonRadius + 10.f, sf::radians(sAngle - div))};

        menuQuads.batch_unsafe_emplace_back_quad(sf::Color::White, nw, sw, se, ne);
    }

    //--------------------------------------
    // Vertical separators

    menuQuads.reserve_more_quad(3);
    const float xOffset{w / 4.f};
    float       topHeight{h / 2.f - h / 15.f}, bottomHeight{h / 2.f + h / 15.f};

    for (int i{-1}; i < 2; ++i)
    {
        createQuad(sf::Color::White, w / 2.f + i * xOffset - 5.f, w / 2.f + i * xOffset + 5.f, topHeight, bottomHeight);
    }

    drawOverlay(menuQuads);

    //--------------------------------------
    // Counters: text and numbers

    topHeight += 5.f - txtLoadSmall.height;
    const float numbersHeight = bottomHeight - txtLoadBig.height * 2.f;

    txtLoadSmall.font.setFillColor(sf::Color::White);
    txtLoadBig.font.setFillColor(sf::Color::White);

    // 1
    float textOffset{w - 3.f * xOffset};
    renderTextCentered("PACKS LOADED", txtLoadSmall.font, {textOffset / 2.f, topHeight});
    renderTextCentered(sf::base::String(ssvu::toStr(loadInfo.packs)), txtLoadBig.font, {textOffset / 2.f, numbersHeight});

    // 2
    textOffset = w - xOffset;
    renderTextCentered("LEVELS LOADED", txtLoadSmall.font, {textOffset / 2.f, topHeight});
    renderTextCentered(sf::base::String(ssvu::toStr(loadInfo.levels)), txtLoadBig.font, {textOffset / 2.f, numbersHeight});

    // 3
    textOffset = w + xOffset;
    renderTextCentered("ASSETS LOADED", txtLoadSmall.font, {textOffset / 2.f, topHeight});
    renderTextCentered(sf::base::String(ssvu::toStr(loadInfo.assets)), txtLoadBig.font, {textOffset / 2.f, numbersHeight});

    //--------------------------------------
    // Random tip

    const float tipInterline{txtRandomTip.height * 1.5f};
    float       height{h - tipInterline * 2.f};
    for (int i{1}; i >= 0; --i) // all tips are on two lines
    {
        renderTextCentered(sf::base::String(randomTip[i]), txtRandomTip.font, {w / 2.f, height - tipInterline});
        height -= tipInterline;
    }

    //--------------------------------------
    // Errors (if any)

    int size = loadInfo.errorMessages.size();

    textOffset = w + 3.f * xOffset;
    renderTextCentered("ERRORS", txtLoadSmall.font, {textOffset / 2.f, topHeight}, sf::Color::Red);
    renderTextCentered(sf::base::String(ssvu::toStr(size)), txtLoadBig.font, {textOffset / 2.f, numbersHeight}, sf::Color::Red);

    // No error messages
    if (!size)
    {
        bottomHeight += txtLoadSmall.height * 1.75f;
        renderTextCentered("NO LOAD ERRORS", txtLoadSmall.font, {w / 2.f, bottomHeight}, sf::Color::White);
        return;
    }

    // Print errors from last to first.
    bottomHeight += txtLoadSmall.height / 2.f;
    const float txtSpacing{txtLoadSmall.height * 2.f};

    // But first handle the scrollbar
    auto [scrollbarNotches, drawnSize] = getScrollbarNotches(size, maxErrorsOnScreen);
    if (scrollbarNotches != 0)
    {
        drawScrollbar(txtSpacing * drawnSize,
                      size,
                      scrollbarNotches,
                      w - 4.f - textToQuadBorder,
                      bottomHeight + txtSpacing / 4.f,
                      sf::Color::Red);
    }

    for (int i{drawnSize - 1 + scrollbarOffset}; i > -1 + scrollbarOffset; --i)
    {
        renderTextCentered(loadInfo.errorMessages[i], txtLoadSmall.font, {w / 2.f, bottomHeight});
        bottomHeight += txtSpacing;
    }
}

void MenuGame::updateLevelSelectionDrawingParameters()
{
    levelDetailsOffset = 0.f;
    lvlSlct.XOffset    = 0.f;
    favSlct.XOffset    = 0.f;

    textToQuadBorder = txtSelectionMedium.height * frameSizeMulti;
    slctFrameSize    = textToQuadBorder * 0.3f;

    packLabelHeight = txtSelectionMedium.height + 2.f * textToQuadBorder + slctFrameSize;

    levelLabelHeight = txtSelectionBig.height +   // level name
                       txtSelectionSmall.height + // level author
                       textToQuadBorder +         // interspace
                       2.f * textToQuadBorder -   // top and bottom space
                       slctFrameSize;             // the bottom border
}

float MenuGame::getLevelSelectionHeight() const
{
    return packLabelHeight * getSelectablePackInfosSize() + getLevelListHeight() - packChangeOffset +
           (lvlDrawer->packIdx != static_cast<int>(getSelectablePackInfosSize()) - 1 ? 2.f : 1.f) * slctFrameSize;
}

void MenuGame::scrollName(sf::base::String& text, float& scroller)
{
    // FPS consistent scrolling
    scroller += getFPSMult();
    text += "  ";

    const auto       modIdx = ssvu::getMod(static_cast<int>(scroller / 100.f), text.size());
    sf::base::String charsToMove(text.toStringView().substrByPosLen(0, static_cast<sf::base::SizeT>(modIdx)));
    text.erase(0, static_cast<sf::base::SizeT>(modIdx));
    text += charsToMove;
}

void MenuGame::scrollNameRightBorder(sf::base::String& text, const sf::base::String key, sf::Text& font, float& scroller, float border)
{
    // Store length of the key
    font.setString(key);
    const float keyWidth = font.getGlobalWidth();

    Utils::uppercasify(text);
    font.setString(text);

    // If the text is already within border format and return
    border -= keyWidth;
    if (font.getGlobalWidth() <= border)
    {
        text = key + text;
        return;
    }

    // Scroll the name and shrink it to the required length
    scrollName(text, scroller);
    font.setString(text);
    while (font.getGlobalWidth() > border && text.size() > 1)
    {
        text.erase(text.size() - 1, 1);
        font.setString(text);
    }
    text = key + text;
}

void MenuGame::scrollNameRightBorder(sf::base::String& text, sf::Text& font, float& scroller, const float border)
{
    Utils::uppercasify(text);
    font.setString(text);

    if (font.getGlobalWidth() <= border)
    {
        return;
    }

    scrollName(text, scroller);
    font.setString(text);
    while (font.getGlobalWidth() > border && text.size() > 1)
    {
        text.erase(text.size() - 1, 1);
        font.setString(text);
    }
}


[[nodiscard]] float MenuGame::getLevelListHeight() const
{
    return levelLabelHeight * (focusHeld ? 1 : lvlDrawer->levelDataIds->size()) + slctFrameSize;
}

void MenuGame::calcScrollSpeed()
{
    // Only speed up the animation if there are more than 16 levels.
    scrollSpeed = baseScrollSpeed * std::max(lvlDrawer->levelDataIds->size() / 16.f, 1.f);
}

void MenuGame::calcLevelChangeScroll(const int dir)
{
    scrollSpeed = baseScrollSpeed;

    // When in the favorites menu, the packIdx variable stores the index
    // of the pack the currently selected level belongs to. However
    // the favorite levels are displayed as if they belonged to a single pack,
    // so for the purposes of this function the pack index of the
    // favorites menu is always 0.
    const int actualPackIdx{isFavoriteLevels() ? 0 : lvlDrawer->packIdx};
    float     scroll;

    if (dir < 0)
    {
        // If we are approaching the top of the pack show either the first
        // level label and the next pack label or two previous pack labels.
        if (lvlDrawer->currentIndex < 2)
        {
            scroll = packLabelHeight * (actualPackIdx + 1 - (2 - lvlDrawer->currentIndex));
        }
        else
        {
            //...otherwise just show the two previous level labels.
            scroll = packLabelHeight * (actualPackIdx + 1) + levelLabelHeight * (lvlDrawer->currentIndex + dir) +
                     slctFrameSize;
        }

        checkWindowTopScroll(scroll, [this](const float target) { lvlDrawer->YScrollTo = target; });
        return;
    }

    const int size{static_cast<int>(lvlDrawer->levelDataIds->size())};
    // If we are approaching the bottom of the pack show either the
    // last level label and the next pack label or two next pack labels...
    if (lvlDrawer->currentIndex >= size - 2 && actualPackIdx != static_cast<int>(getSelectablePackInfosSize()) - 1)
    {
        scroll = packLabelHeight * (actualPackIdx + 1 + (2 - (size - 1 - lvlDrawer->currentIndex))) +
                 levelLabelHeight * size + 3.f * slctFrameSize;
    }
    else
    {
        //...otherwise just show the two next level labels.
        scroll = packLabelHeight * (actualPackIdx + 1) +
                 levelLabelHeight *
                     std::min(lvlDrawer->currentIndex + dir + 1, static_cast<int>(lvlDrawer->levelDataIds->size())) +
                 2.f * slctFrameSize;
    }

    checkWindowBottomScroll(scroll, [this](const float target) { lvlDrawer->YScrollTo = target; });
}

void MenuGame::calcPackChangeScrollFold(const float mLevelListHeight)
{
    if (packChangeDirection == -2)
    {
        return;
    }

    // Make sure the last level and the two before it fit on screen.
    const float scroll{packLabelHeight * lvlDrawer->packIdx + slctFrameSize +
                       std::max(0.f, packLabelHeight + slctFrameSize + mLevelListHeight - packChangeOffset)};

    // As soon as the bottom of the level list goes out of the screen
    // switch pack by setting the packChangeOffset to the height of the
    // level list.
    checkWindowTopScroll(scroll,
                         [this, mLevelListHeight](const float target)
    {
        lvlDrawer->YScrollTo = lvlDrawer->YOffset = target;
        packChangeOffset                          = mLevelListHeight;
    });
}

void MenuGame::calcPackChangeScrollStretch(const float mLevelListHeight)
{
    float                                          scrollTop, scrollBottom;
    sf::base::FixedFunction<void(const float), 64> action{[this](const float target)
    { lvlDrawer->YScrollTo = lvlDrawer->YOffset = target; }};

    if (packChangeDirection == -2)
    {
        // The last pack does not have a "next pack", therefore it gets a
        // special treatement, which comes after this if statement.
        if (lvlDrawer->packIdx != static_cast<int>(getSelectablePackInfosSize()) - 1)
        {
            // Height of the top of the pack label of the current pack.
            scrollTop = packLabelHeight * lvlDrawer->packIdx;
            // Height of the bottom of the next pack label.
            scrollBottom = scrollTop + 2.f * (packLabelHeight + slctFrameSize) +
                           std::max(0.f, mLevelListHeight - packChangeOffset);

            // With particularly long lists it only makes sense to show
            // the stretch animation for as long as we can see the pack
            // label on screen. After the pack label is outside of the draw
            // window we only see a standing still list since we no more
            // have a point of reference and the list is programmed to keep
            // scrollBottom inside the window. If this occurs cut the
            // animation short.
            sf::base::FixedFunction<void(const float), 64> specialAction{
                [this, &action, scrollTop, mLevelListHeight](const float target)
            {
                if (scrollTop < -lvlDrawer->YOffset)
                {
                    packChangeOffset = 0.f;
                    action(h - (scrollTop + 2.f * (packLabelHeight + slctFrameSize) + std::max(0.f, mLevelListHeight)));
                    return;
                }
                action(target);
            }};

            // The bottom must prevail.
            if (!checkWindowBottomScrollWithResult(scrollBottom, specialAction))
            {
                checkWindowTopScroll(scrollTop, action);
            }

            return;
        }

        // Top of the pack label.
        scrollTop = packLabelHeight * getSelectablePackInfosSize() + slctFrameSize;
        // Bottom of the level list.
        scrollBottom = scrollTop + std::max(0.f, mLevelListHeight - packChangeOffset);

        // The bottom must prevail.
        if (!checkWindowBottomScrollWithResult(scrollBottom, action))
        {
            checkWindowTopScroll(scrollTop, action);
        }
        return;
    }

    // The list is shifted to try fit all levels in the pack.
    // If that is not possible just include the pack label
    // + whatever amount of levels it's possible to fit on screen.
    scrollTop    = packLabelHeight * (lvlDrawer->packIdx - 1);
    scrollBottom = scrollTop + packLabelHeight +
                   std::min(packLabelHeight + slctFrameSize + mLevelListHeight - packChangeOffset, h);

    checkWindowBottomScroll(scrollBottom, action);
    checkWindowTopScroll(scrollTop, action);
}

void MenuGame::quickPackFoldStretch()
{
    // Top of the pack label of the previous pack.
    const float scrollTop{packLabelHeight * (lvlDrawer->packIdx - 1)};
    // Bottom of the pack label of the next pack.
    const float scrollBottom{scrollTop + 2.f * packLabelHeight + slctFrameSize + getLevelListHeight()};

    checkWindowBottomScroll(scrollBottom, [this](const float target) {
        lvlDrawer->YScrollTo = lvlDrawer->YOffset = target;
    });
    checkWindowTopScroll(scrollTop, [this](const float target) { lvlDrawer->YScrollTo = lvlDrawer->YOffset = target; });
    adjustLevelsOffset();
}

void MenuGame::scrollLevelListToTargetY(float mFT)
{
    if (std::abs(lvlDrawer->YOffset - lvlDrawer->YScrollTo) <= Utils::epsilon)
    {
        return;
    }

    if (lvlDrawer->YOffset < lvlDrawer->YScrollTo)
    {
        lvlDrawer->YOffset += mFT * scrollSpeed;
        if (lvlDrawer->YOffset >= lvlDrawer->YScrollTo)
        {
            lvlDrawer->YOffset = lvlDrawer->YScrollTo;
        }
        return;
    }

    lvlDrawer->YOffset -= mFT * scrollSpeed;
    if (lvlDrawer->YOffset <= lvlDrawer->YScrollTo)
    {
        lvlDrawer->YOffset = lvlDrawer->YScrollTo;
    }
}

inline constexpr int descLines{5};

void MenuGame::checkWindowTopScroll(const float scroll, sf::base::FixedFunction<void(const float), 64> action)
{
    const float target{-scroll};
    if (target <= lvlDrawer->YOffset)
    {
        return;
    }

    action(target);
}

bool MenuGame::checkWindowTopScrollWithResult(const float scroll, sf::base::FixedFunction<void(const float), 64> action)
{
    const float target{-scroll};
    if (target <= lvlDrawer->YOffset)
    {
        return false;
    }

    action(target);
    return true;
}

void MenuGame::checkWindowBottomScroll(const float scroll, sf::base::FixedFunction<void(const float), 64> action)
{
    const float target{h - scroll};
    if (target >= lvlDrawer->YOffset)
    {
        return;
    }

    action(target);
}

bool MenuGame::checkWindowBottomScrollWithResult(const float scroll, sf::base::FixedFunction<void(const float), 64> action)
{
    const float target{h - scroll};
    if (target >= lvlDrawer->YOffset)
    {
        return false;
    }

    action(target);
    return true;
}

void MenuGame::resetNamesScrolls()
{
    for (int i{0}; i < static_cast<int>(Label::ScrollsSize); ++i)
    {
        namesScroll[i] = 0;
    }
}

void MenuGame::resetLevelNamesScrolls()
{
    // Reset all scrolls except the ones relative to the pack.
    namesScroll[static_cast<int>(Label::LevelName)] = 0.f;
    for (int i = static_cast<int>(Label::MusicName); i < static_cast<int>(Label::ScrollsSize); ++i)
    {
        namesScroll[i] = 0.f;
    }
}

[[nodiscard]] float MenuGame::getMaximumTextWidth() const
{
    return w * 0.33f - 2.f * textToQuadBorder;
}

void MenuGame::formatLevelDescription()
{
    levelDescription.clear();

    sf::base::Vector<sf::base::String> words;

    {
        strBuf.clear();
        sf::base::String& desc = strBuf;

        desc += assets.getLevelData((*lvlDrawer->levelDataIds)[lvlDrawer->currentIndex]).description;

        if (desc.empty())
        {
            return;
        }

        Utils::uppercasify(desc);

        // Split description into words.
        desc += '\n'; // Add a safety newline

        for (sf::base::SizeT i{0}, j{0}; i < desc.size(); ++i)
        {
            if (desc[i] == '\n')
            {
                words.emplaceBack(sf::base::String(desc.toStringView().substrByPosLen(j, i - j + 1))); // include newline.
                j = i + 1;
            }
            else if (desc[i] == ' ')
            {
                words.emplaceBack(sf::base::String(desc.toStringView().substrByPosLen(j, i - j)));
                j = i + 1; // skip the space.
            }
        }
    }

    // Group words into lines depending on whether
    // they fit within the maximum width.
    const float      maxWidth{getMaximumTextWidth()};
    sf::base::String candidate;
    sf::base::String temp;
    for (sf::base::SizeT i{0}; i < words.size() && levelDescription.size() < descLines; ++i)
    {
        if (!candidate.empty())
        {
            temp = " " + words[i];
            txtSelectionSmall.font.setString(candidate + temp);
        }
        else
        {
            temp = words[i];
            txtSelectionSmall.font.setString(temp);
        }

        // If last character is a newline...
        if (!temp.empty() && temp[temp.size() - 1] == '\n')
        {
            // ...if it all fits add to the vector as a single line...
            if (txtSelectionSmall.font.getGlobalWidth() < maxWidth)
            {
                candidate += temp;
                levelDescription.pushBack(candidate);
            }
            else
            {
                // ...otherwise add "candidate" to the vector and add the new
                // word on its own.
                levelDescription.pushBack(candidate);
                if (levelDescription.size() < descLines)
                {
                    levelDescription.pushBack(words[i]);
                }
            }
            candidate.clear();
            continue;
        }

        // If there is no newline check if the line fits...
        if (txtSelectionSmall.font.getGlobalWidth() < maxWidth)
        {
            candidate += temp;
            continue;
        }

        // ...if it doesn't add to the vector "candidate" and then set it to be
        // just the overflowing word, to be checked upon in the next cycles.
        levelDescription.pushBack(candidate);
        candidate = words[i];
    }

    // Add whatever is left if it fits.
    if (levelDescription.size() < descLines)
    {
        levelDescription.pushBack(candidate);
    }
}

void MenuGame::changeFavoriteLevelsToProfile()
{
    // Each profile has its own favorite levels.
    // Copy the `ProfileData` favorites into the menu's favorites vector.

    favoriteLevelDataIds.clear();

    for (const sf::base::String& id : assets.getCurrentLocalProfile().getFavoriteLevelIds())
    {
        favoriteLevelDataIds.pushBack(id);
    }

    sf::base::quickSort(favoriteLevelDataIds.begin(),
                        favoriteLevelDataIds.end(),
                        [this](const sf::base::String& a, const sf::base::String& b) -> bool
    {
        return ssvu::toLower(std::string(assets.getLevelData(a).name.cStr())) <
               ssvu::toLower(std::string(assets.getLevelData(b).name.cStr()));
    });

    const int sz{static_cast<int>(favoriteLevelDataIds.size())};

    // If the new current profile has no favorites force the level selection
    // to use the regular drawer.
    if (sz == 0)
    {
        lvlDrawer = &lvlSlct;
        return;
    }

    // Make sure the level index is within boundaries.
    ssvu::clamp(favSlct.currentIndex, 0, sz - 1);

    // Resize the level offset parameters.
    favSlct.lvlOffsets.resize(sz);
}


[[nodiscard]] bool MenuGame::isFavoriteLevels() const
{
    return lvlDrawer->isFavorites;
}

[[nodiscard]] sf::base::SizeT MenuGame::getSelectablePackInfosSize() const
{
    return isFavoriteLevels() ? 1 : assets.getSelectablePackInfos().size();
}

[[nodiscard]] const PackInfo& MenuGame::getNthSelectablePackInfo(const sf::base::SizeT i)
{
    return assets.getSelectablePackInfos()[i];
}

void MenuGame::addRemoveFavoriteLevel()
{
    const LevelData& data{assets.getLevelData((*lvlDrawer->levelDataIds)[lvlDrawer->currentIndex])};

    const sf::base::String levelID{data.packId + "_" + data.id};

    // Level is a favorite so remove it.
    if (isLevelFavorite)
    {
        assets.getCurrentLocalProfile().removeFavoriteLevel(levelID);
        favoriteLevelDataIds.erase(std::find(favoriteLevelDataIds.begin(), favoriteLevelDataIds.end(), levelID));
        favSlct.lvlOffsets.popBack();

        // Make sure the index is within bounds.
        if (!favSlct.levelDataIds->empty())
        {
            ssvu::clamp(favSlct.currentIndex, 0, static_cast<int>(favSlct.levelDataIds->size()) - 1);
        }
        else
        {
            favSlct.currentIndex = 0;
        }

        if (isFavoriteLevels())
        {
            adjustLevelsOffset();
        }

        // If the favorite levels vector is empty
        // after the removal force exit from the
        // favorites menu.
        if (favSlct.levelDataIds->empty())
        {
            favSlct.XOffset = 0.f; // this way the menu is not drawn anymore.
            lvlDrawer       = &lvlSlct;
        }
        else
        {
            // Make sure there is no empty space between the end of the list
            // and the bottom of the window. Needed to ensure the selected
            // level is always on screen.
            const float scroll{
                h - (packLabelHeight + 2.f * slctFrameSize + levelLabelHeight * favSlct.levelDataIds->size())};
            if (scroll > lvlDrawer->YOffset)
            {
                favSlct.YOffset = favSlct.YScrollTo = scroll;
            }
        }

        // Update the looks.
        setIndex(lvlDrawer->currentIndex);
    }
    else
    {
        assets.getCurrentLocalProfile().addFavoriteLevel(levelID);
        favSlct.lvlOffsets.emplaceBack(0.f);
        isLevelFavorite = true;

        // Add the level to the favorites vector
        // keeping it sorted in alphabetical order.
        auto                   it{favoriteLevelDataIds.begin()};
        const auto             end{favoriteLevelDataIds.end()};
        const sf::base::String tweakedFavName{
            sf::base::String(ssvu::toLower(std::string(assets.getLevelData(levelID).name.cStr())))};

        sf::base::String tweakedLevelName;
        while (it != end)
        {
            tweakedLevelName = sf::base::String(ssvu::toLower(std::string(assets.getLevelData(*it).name.cStr())));
            if (tweakedLevelName > tweakedFavName)
            {
                break;
            }

            ++it;
        }

        if (it == end)
        {
            favoriteLevelDataIds.emplaceBack(levelID);
        }
        else
        {
            favoriteLevelDataIds.insert(it, levelID);
        }

        // Just in case.
        ssvu::clamp(favSlct.currentIndex, 0, static_cast<int>(favSlct.levelDataIds->size()) - 1);
    }

    playSoundOverride("select.ogg");
}

void MenuGame::switchToFromFavoriteLevels()
{
    if (state != States::LevelSelection || favSlct.levelDataIds->empty())
    {
        return;
    }

    // Quickly finish any ongoing pack changes.
    if (packChangeState == PackChange::Folding)
    {
        changePack();
    }

    packChangeState      = PackChange::Rest;
    lvlDrawer->YScrollTo = lvlDrawer->YOffset;
    packChangeOffset     = 0.f;

    lvlDrawer = isFavoriteLevels() ? &lvlSlct : &favSlct;
    setIndex(lvlDrawer->currentIndex); // update the looks
    adjustLevelsOffset();
    resetNamesScrolls();
    playSoundOverride("select.ogg");
}

void MenuGame::drawLevelSelectionRightSide(LevelDrawer& drawer, const bool revertOffset)
{
    // total distance from the top of the text
    // to the outer border of the label.
    const float outerFrame{textToQuadBorder + slctFrameSize};
    const float packLabelOffset{w * 0.33f - outerFrame};
    const float quadsIndent{w - packLabelOffset};
    const float txtIndent{w - packLabelOffset / 2.f};
    const float levelIndent{quadsIndent + outerFrame};
    const float panelOffset{calcMenuOffset(drawer.XOffset, w - quadsIndent, revertOffset)};

    const auto& infos{assets.getSelectablePackInfos()};
    int         packsSize, levelsSize;
    if (drawer.isFavorites)
    {
        levelsSize = drawer.levelDataIds->size();
        packsSize  = 1;
    }
    else
    {
        packsSize  = infos.size();
        levelsSize = focusHeld ? 1 : drawer.levelDataIds->size();
    }

    static sf::base::String tempString;
    float                   prevLevelIndent{0.f}, height{0.f};
    sf::Vec2f               topLeft, topRight, bottomRight, bottomLeft;

    // The drawing order is: levels list then pack labels.
    // The reason for it is that when a pack is deselected the
    // level list slides up and it must do so below the previous pack labels.
    // Therefore pack labels must be drawn above everything else (aka must
    // be drawn last).

    topLeft    = {w / 2.f, 2.f};
    tempString = isFavoriteLevels() ? "PRESS F2 TO SHOW ALL LEVELS" : "PRESS F2 TO SHOW FAVORITE LEVELS";
    renderTextCentered(tempString, txtSelectionSmall.font, topLeft);
    tempString = "\nHOLD FOCUS TO JUMP BETWEEN PACKS";
    renderTextCentered(tempString, txtSelectionSmall.font, topLeft);

    //----------------------------------------
    // LEVELS LIST

    int       i;
    sf::Color alphaTextColor{menuTextColor.r, menuTextColor.g, menuTextColor.b, 150};
    txtSelectionMedium.font.setFillColor(menuTextColor);
    height = packLabelHeight * (isFavoriteLevels() ? 1 : drawer.packIdx + 1) + slctFrameSize - packChangeOffset +
             drawer.YOffset;

    for (i = 0; i < levelsSize; ++i)
    {
        //-------------------------------------
        // Quads
        menuQuads.clear();
        menuQuads.reserve_quad(3);

        // If the list is folding give all level labels the same alignment
        if (packChangeState != PackChange::Rest)
        {
            drawer.lvlOffsets[i] = 0.f;
        }
        else
        {
            calcMenuItemOffset(drawer.lvlOffsets[i], i == drawer.currentIndex);
        }

        float indent = quadsIndent + panelOffset;
        if (!focusHeld)
        {
            indent -= drawer.lvlOffsets[i];
        }

        // Top frame
        if (i > 0 && drawer.lvlOffsets[i - 1] > drawer.lvlOffsets[i])
        {
            createQuad(menuQuadColor, prevLevelIndent, w, height, height + slctFrameSize);
        }
        else
        {
            createQuad(menuQuadColor, indent, w, height, height + slctFrameSize);
        }

        // Side frame
        createQuad(menuQuadColor, indent, indent + slctFrameSize, height + slctFrameSize, height + levelLabelHeight);

        // Body
        const sf::Vec2f bodyMins{indent + slctFrameSize, height + slctFrameSize};

        const sf::Vec2f bodyMaxs{w, height + levelLabelHeight};

        const bool mouseOverlap = overlayMouseOverlapAndUpdateHover(bodyMins, bodyMaxs);

        const sf::Color c = i == drawer.currentIndex ? menuSelectionColor : alphaTextColor;

        createQuad(mouseOverlapColor(mouseOverlap, c), bodyMins, bodyMaxs);

        // TODO (P2): cleanup mouse control
        if (mouseOverlap && mouseLeftRisingEdge())
        {
            if (!mustPlay && HRClock::now() - lastMouseClick < std::chrono::milliseconds(160))
            {
                mustPlay = true;
            }
            else if (!mustChangeIndexTo.hasValue())
            {
                mustChangeIndexTo.emplace(i);
            }
        }

        drawOverlay(menuQuads);
        prevLevelIndent = indent;

        //-------------------------------------
        // Level data
        const LevelData* const levelData{&assets.getLevelData((*drawer.levelDataIds)[i])};
        if (levelData == nullptr)
        {
            continue;
        }

        //-------------------------------------
        // Level name

        indent = levelIndent + panelOffset;
        if (!focusHeld)
        {
            indent -= drawer.lvlOffsets[i];
        }
        height += textToQuadBorder;

        tempString = focusHeld ? "..." : levelData->name;
        Utils::uppercasify(tempString);

        const sf::Color c0 = mouseOverlapColor(mouseOverlap, menuQuadColor);

        const auto              currentDiffMult = levelData->getNthDiffMult(diffMultIdx);
        const sf::base::String& levelValidator  = levelData->getValidator(currentDiffMult);

        renderText(tempString, txtSelectionBig.font, {indent, height - txtSelectionBig.height * fontHeightOffset}, c0);

        if (!levelData->unscored && hexagonClient.isLevelSupportedByServer(levelValidator))
        {
            const float padding = 5.f;
            const float width   = 50.f;

            menuQuads.clear();
            menuQuads.reserve_quad(1);

            createQuad(menuQuadColor,
                       w - width - padding,
                       w,
                       height - textToQuadBorder,
                       height - textToQuadBorder + txtSelectionRanked.height + padding + 1.f);

            drawOverlay(menuQuads);

            renderText("RANKED",
                       txtSelectionRanked.font,
                       {w - width, height - txtSelectionRanked.height * fontHeightOffset - 3.f},
                       mouseOverlapColor(mouseOverlap, c));
        }

        //-------------------------------------
        // Author
        height += txtSelectionBig.height + textToQuadBorder;

        tempString = focusHeld ? "..." : levelData->author;
        Utils::uppercasify(tempString);

        const sf::Color c1 = mouseOverlapColor(mouseOverlap, menuQuadColor);

        renderText(tempString, txtSelectionSmall.font, {indent, height - txtSelectionSmall.height * fontHeightOffset}, c1);

        height += txtSelectionSmall.height + textToQuadBorder - slctFrameSize;
    }

    // Bottom frame for the last element
    menuQuads.clear();
    menuQuads.reserve_quad(1);
    createQuad(menuQuadColor, prevLevelIndent, w, height, height + slctFrameSize);
    drawOverlay(menuQuads);

    height += slctFrameSize;
    i = ssvu::getMod(drawer.packIdx + 1, packsSize);
    if (i == 0)
    {
        height = drawer.YOffset;
    }

    //----------------------------------------
    // PACKS LABELS

    const float arrowWidth{packLabelHeight / 2.f - textToQuadBorder};

    do
    {
        // Quads
        menuQuads.clear();
        menuQuads.reserve_quad(2);

        float temp = quadsIndent - outerFrame + panelOffset;

        createQuad(menuTextColor, temp - slctFrameSize, w, height, height + packLabelHeight + slctFrameSize);

        const sf::Vec2f bodyMins{temp, height + slctFrameSize};
        const sf::Vec2f bodyMaxs{w, height + packLabelHeight};

        const bool mouseOverlap = overlayMouseOverlapAndUpdateHover(bodyMins, bodyMaxs);

        createQuad(mouseOverlapColor(mouseOverlap, menuQuadColor), bodyMins, bodyMaxs);

        // TODO (P2): cleanup mouse control
        if (mouseOverlap && !mustChangePackIndexTo.hasValue() && mouseLeftRisingEdge())
        {
            mustChangePackIndexTo.emplace(i);
        }

        drawOverlay(menuQuads);

        // Name & >
        if (drawer.isFavorites)
        {
            tempString = "FAVORITES";
        }
        else
        {
            tempString = assets.getPackData(infos[i].id).name;
            Utils::uppercasify(tempString);
        }

        txtSelectionMedium.font.setString(tempString);
        temp = std::max(txtIndent - txtSelectionMedium.font.getGlobalWidth() / 2.f,
                        quadsIndent + arrowWidth + 2.f * slctFrameSize + outerFrame) +
               panelOffset;

        txtSelectionMedium.font.position = {temp, height + outerFrame - txtSelectionMedium.height * fontHeightOffset};

        const sf::Color oldC = txtSelectionMedium.font.getFillColor();
        txtSelectionMedium.font.setFillColor(mouseOverlapColor(mouseOverlap, menuTextColor));
        drawOverlay(txtSelectionMedium.font);
        txtSelectionMedium.font.setFillColor(oldC);

        menuQuads.clear();
        menuQuads.reserve_quad(2);

        if (i == drawer.packIdx)
        {
            // Draw > pointing downward, coordinates look a bit complicated
            // cause it's aligned with the middle point of the regular > arrows
            // The arrow is (packLabelHeight / 2.f - textToQuadBorder + 2.f *
            // slctFrameSize) wide
            height += (packLabelHeight - textToQuadBorder - slctFrameSize) / 2.f;
            temp = quadsIndent + arrowWidth / 2.f + slctFrameSize + panelOffset;

            topLeft    = {temp - arrowWidth, height};
            bottomLeft = {temp - arrowWidth, height + 2.f * slctFrameSize};

            height += arrowWidth;

            topRight    = {temp, height};
            bottomRight = {temp, height + 2.f * slctFrameSize};

            menuQuads.batch_unsafe_emplace_back_quad(menuTextColor, topLeft, bottomLeft, bottomRight, topRight);

            topLeft    = {temp, height};
            bottomLeft = {temp, height + 2.f * slctFrameSize};

            height -= arrowWidth;
            temp += arrowWidth;

            topRight    = {temp, height};
            bottomRight = {temp, height + 2.f * slctFrameSize};

            menuQuads.batch_unsafe_emplace_back_quad(menuTextColor, topLeft, bottomLeft, bottomRight, topRight);

            drawOverlay(menuQuads);
        }
        else
        {
            height += slctFrameSize / 2.f;
            temp = quadsIndent + panelOffset;

            topLeft  = {temp, height + textToQuadBorder};
            topRight = {temp + 2.f * slctFrameSize, height + textToQuadBorder};

            height += packLabelHeight / 2.f;
            temp += packLabelHeight / 2.f - textToQuadBorder;

            bottomLeft  = {temp, height};
            bottomRight = {temp + 2.f * slctFrameSize, height};

            menuQuads.batch_unsafe_emplace_back_quad(menuTextColor, topLeft, bottomLeft, bottomRight, topRight);

            topLeft  = {temp, height};
            topRight = {temp + 2.f * slctFrameSize, height};

            height += packLabelHeight / 2.f;
            temp = quadsIndent + panelOffset;

            bottomLeft  = {temp, height - textToQuadBorder};
            bottomRight = {temp + 2.f * slctFrameSize, height - textToQuadBorder};

            menuQuads.batch_unsafe_emplace_back_quad(menuTextColor, topLeft, bottomLeft, bottomRight, topRight);

            drawOverlay(menuQuads);
            height -= slctFrameSize / 2.f;
        }

        i = ssvu::getMod(i + 1, packsSize);
        if (i == 0)
        {
            height = drawer.YOffset;
        }
    } while (i != ssvu::getMod(drawer.packIdx + 1, packsSize));
}

void MenuGame::drawLevelSelectionLeftSide(LevelDrawer& drawer, const bool revertOffset)
{
    if (currentPack == nullptr)
    {
        return;
    }

    constexpr float lineThickness{2.f};

    const LevelData& levelData{assets.getLevelData((*drawer.levelDataIds)[drawer.currentIndex])};

    const float maxPanelOffset{w * 0.33f};
    const float panelOffset{calcMenuOffset(levelDetailsOffset, maxPanelOffset, revertOffset)};
    const float smallInterline{txtSelectionSmall.height * 1.5f};
    const float mediumInterline{txtSelectionSmall.height / 2.f};
    const float textXPos{textToQuadBorder - panelOffset};
    const float textRightBorder{getMaximumTextWidth()};

    const float width{maxPanelOffset - panelOffset};
    float       height{textToQuadBorder};

    //-------------------------------------
    // Backdrop - Right border

    menuQuads.clear();
    menuQuads.reserve_quad(2);
    createQuad({menuTextColor.r, menuTextColor.g, menuTextColor.b, 150}, 0, width, 0, h);
    createQuad(menuQuadColor, width, width + lineThickness, 0, h);
    drawOverlay(menuQuads);
    menuQuads.clear();

    //-------------------------------------
    // Level name

    sf::base::String tempString{levelData.name};
    scrollNameRightBorder(tempString, txtSelectionBig.font, namesScroll[static_cast<int>(Label::LevelName)], textRightBorder);
    renderText(tempString, txtSelectionBig.font, {textXPos, height - txtSelectionBig.height * fontHeightOffset});

    //-------------------------------------
    // Level description

    height += txtSelectionBig.height + textToQuadBorder - txtSelectionSmall.height * 0.7f;

    int i;
    for (i = 0; i < static_cast<int>(levelDescription.size()); ++i)
    {
        renderText(levelDescription[i], txtSelectionSmall.font, {textXPos, height});
        height += i == descLines - 1 ? txtSelectionSmall.height : smallInterline;
    }
    if (i != descLines)
    {
        height += smallInterline * std::max(0, descLines - 1 - i) + txtSelectionSmall.height;
    }

    height += textToQuadBorder + txtSelectionSmall.height * 0.7f;

    //-------------------------------------
    // Difficulty

    menuQuads.reserve_more_quad(2);

    // Top line
    height += lineThickness;
    createQuad(menuQuadColor, 0, width, height - lineThickness, height);


    txtSelectionSmall.font.setFillColor(menuQuadColor);
    txtSelectionMedium.font.setFillColor(menuQuadColor);

    // Text
    height += textToQuadBorder;
    const float difficultyHeight{height - txtSelectionMedium.height * fontHeightOffset};

    renderText("DIFFICULTY: ", txtSelectionMedium.font, {textXPos, difficultyHeight}, menuQuadColor);

    tempString = levelData.difficultyMults.size() > 1 ? "< " + ssvu::toStr(levelData.getNthDiffMult(diffMultIdx)) + " >"
                                                      : "NONE";

    const float difficultyBumpFactor = 1.f + ((difficultyBumpEffect / difficultyBumpEffectMax) * 0.25f);
    txtSelectionMedium.font.scale    = {difficultyBumpFactor, difficultyBumpFactor};

    renderText(tempString,
               txtSelectionMedium.font,
               {textXPos + txtSelectionMedium.font.getGlobalBounds().size.y, difficultyHeight});

    txtSelectionMedium.font.scale = {1.f, 1.f};

    // Bottom line
    height += txtSelectionMedium.height + textToQuadBorder + lineThickness;

    createQuad(menuQuadColor, 0, width, height, height - lineThickness);

    //-------------------------------------
    // Pack info

    // "PACK"
    height += textToQuadBorder;

    renderText("PACK",
               txtSelectionMedium.font,
               {textToQuadBorder - panelOffset, height - txtSelectionMedium.height * fontHeightOffset});

    // Pack name
    height += txtSelectionMedium.height + mediumInterline;
    tempString = currentPack->name;


    scrollNameRightBorder(tempString, "NAME: ", txtSelectionSmall.font, namesScroll[static_cast<int>(Label::PackName)], textRightBorder);
    renderText(tempString, txtSelectionSmall.font, {textXPos, height - txtSelectionSmall.height * fontHeightOffset});

    // Pack author
    height += txtSelectionSmall.height + mediumInterline;

    tempString = currentPack->author;
    scrollNameRightBorder(tempString,
                          "AUTHOR: ",
                          txtSelectionSmall.font,
                          namesScroll[static_cast<int>(Label::PackAuthor)],
                          textRightBorder);
    renderText(tempString, txtSelectionSmall.font, {textXPos, height - txtSelectionSmall.height * fontHeightOffset});

    // Version
    height += txtSelectionSmall.height + mediumInterline;

    tempString = "VERSION: " + ssvu::toStr(currentPack->version);
    Utils::uppercasify(tempString);
    renderText(tempString, txtSelectionSmall.font, {textXPos, height - txtSelectionSmall.height * fontHeightOffset});

    // Bottom line
    menuQuads.reserve_more_quad(1);
    height += txtSelectionSmall.height + txtSelectionMedium.height / 2.f + lineThickness;

    createQuad(menuQuadColor, 0, width, height, height - lineThickness);

    //-------------------------------------
    // Music info

    // "MUSIC"
    height += textToQuadBorder;

    renderText("MUSIC",
               txtSelectionMedium.font,
               {textToQuadBorder - panelOffset, height - txtSelectionMedium.height * fontHeightOffset});

    // Track name
    const MusicData& musicDataTemp = assets.getMusicData(levelData.packId, levelData.musicId);
    height += txtSelectionMedium.height + mediumInterline;
    tempString = musicDataTemp.name;

    scrollNameRightBorder(tempString, "NAME: ", txtSelectionSmall.font, namesScroll[static_cast<int>(Label::MusicName)], textRightBorder);
    renderText(tempString, txtSelectionSmall.font, {textXPos, height - txtSelectionSmall.height * fontHeightOffset});

    // Track author
    height += txtSelectionSmall.height + mediumInterline;
    tempString = musicDataTemp.author;

    scrollNameRightBorder(tempString,
                          "AUTHOR: ",
                          txtSelectionSmall.font,
                          namesScroll[static_cast<int>(Label::MusicAuthor)],
                          textRightBorder);
    renderText(tempString, txtSelectionSmall.font, {textXPos, height - txtSelectionSmall.height * fontHeightOffset});

    // Album name
    height += txtSelectionSmall.height + mediumInterline;
    tempString = !musicDataTemp.album.empty() ? musicDataTemp.album : "NONE";

    scrollNameRightBorder(tempString, "ALBUM: ", txtSelectionSmall.font, namesScroll[static_cast<int>(Label::MusicAlbum)], textRightBorder);
    renderText(tempString, txtSelectionSmall.font, {textXPos, height - txtSelectionSmall.height * fontHeightOffset});

    height += txtSelectionSmall.height + textToQuadBorder;

    //-------------------------------------
    // Favorite "button"

    menuQuads.reserve_more_quad(10);
    const float favoriteButtonBottom{height + 3.f * txtSelectionMedium.height};

    // Frame
    createQuad(menuQuadColor, lineThickness - panelOffset, width, height, height + lineThickness);
    createQuad(menuQuadColor, -panelOffset, lineThickness - panelOffset, height, favoriteButtonBottom);
    createQuad(menuQuadColor, width, width, height, favoriteButtonBottom);
    createQuad(menuQuadColor, lineThickness - panelOffset, width, favoriteButtonBottom - lineThickness, favoriteButtonBottom);

    // Backdrop
    const sf::Vec2f bodyMins{lineThickness - panelOffset, height + lineThickness};

    const sf::Vec2f bodyMaxs{width, favoriteButtonBottom - lineThickness};

    const bool mouseOverlap = overlayMouseOverlapAndUpdateHover(bodyMins, bodyMaxs);

    createQuad(mouseOverlapColor(mouseOverlap, menuSelectionColor), bodyMins, bodyMaxs);

    // TODO (P2): cleanup mouse control
    if (mouseOverlap && !mustFavorite && mouseLeftRisingEdge())
    {
        mustFavorite = true;
    }

    // Also renders all previous quads
    drawOverlay(menuQuads);
    menuQuads.clear();

    renderTextCenteredOffset(isLevelFavorite ? "[F1] UNFAVORITE" : "[F1]    FAVORITE",
                             txtSelectionMedium.font,
                             {maxPanelOffset / 2.f, height + txtSelectionMedium.height * (1.f - fontHeightOffset)},
                             -panelOffset,
                             mouseOverlapColor(mouseOverlap, menuQuadColor));

    height = favoriteButtonBottom + textToQuadBorder;

    //-------------------------------------
    // Leaderboard

    // Personal best
    renderText("LOCAL PERSONAL BEST",
               txtSelectionMedium.font,
               {textToQuadBorder - panelOffset, height - txtSelectionMedium.height * fontHeightOffset});

    height += txtSelectionMedium.height + textToQuadBorder;

    const auto currentDiffMult = levelData.getNthDiffMult(diffMultIdx);

    if (levelData.unscored)
    {
        renderText("N/A",
                   txtSelectionSmall.font,
                   {textToQuadBorder - panelOffset, height - txtSelectionSmall.height * fontHeightOffset});
    }
    else
    {
        const sf::base::String& localLevelValidator = levelData.getValidatorWithoutPackId(currentDiffMult);

        tempString = localLevelValidator;
        renderText(sf::base::String(ssvu::toStr(assets.getCurrentLocalProfile().getScore(tempString)) + "s"),
                   txtSelectionSmall.font,
                   {textToQuadBorder - panelOffset, height - txtSelectionSmall.height * fontHeightOffset});
    }

    // Line
    height += txtSelectionSmall.height + textToQuadBorder + lineThickness;
    menuQuads.reserve_quad(1);
    createQuad(menuQuadColor, 0, width, height, height - lineThickness);

    // "LEADERBOARD"
    height += textToQuadBorder;
    renderTextCenteredOffset("ONLINE LEADERBOARD",
                             txtSelectionBig.font,
                             {maxPanelOffset / 2.f, height - txtSelectionBig.height * fontHeightOffset * .8f},
                             -panelOffset);

    // Line
    height += txtSelectionScore.height + txtSelectionBig.height / 2.f + 3.f;
    menuQuads.reserve_more_quad(1);
    createQuad(menuQuadColor, 0, width, height, height + lineThickness);
    height += lineThickness;

    height += txtSelectionSmall.height;

    const sf::base::String& levelValidator = levelData.getValidator(currentDiffMult);

    if (!levelData.unscored && hexagonClient.getState() == HexagonClient::State::LoggedIn_Ready &&
        hexagonClient.isLevelSupportedByServer(levelValidator) && leaderboardCache->shouldRequestScores(levelValidator))
    {
        hexagonClient.tryRequestTopScoresAndOwnScore(levelValidator);
        leaderboardCache->requestedScores(levelValidator);
    }

    const bool gotScoreInfo = leaderboardCache->hasInformation(levelValidator);

    if (levelData.unscored)
    {
        renderText("LEADERBOARD DISABLED FOR THIS LEVEL",
                   txtSelectionSmall.font,
                   {textToQuadBorder - panelOffset, height - txtSelectionSmall.height * fontHeightOffset});
    }
    else if (hexagonClient.getState() != HexagonClient::State::LoggedIn_Ready)
    {
        renderText("PLEASE LOG IN TO LOAD LEADERBOARD",
                   txtSelectionSmall.font,
                   {textToQuadBorder - panelOffset, height - txtSelectionSmall.height * fontHeightOffset});
    }
    else if (!hexagonClient.isLevelSupportedByServer(levelValidator))
    {
        renderText("THIS LEVEL IS NOT SUPPORTED BY THE SERVER",
                   txtSelectionSmall.font,
                   {textToQuadBorder - panelOffset, height - txtSelectionSmall.height * fontHeightOffset});
    }
    else if (!gotScoreInfo)
    {
        renderText("...",
                   txtSelectionSmall.font,
                   {textToQuadBorder - panelOffset, height - txtSelectionSmall.height * fontHeightOffset});
    }
    else
    {
        SSVOH_ASSERT(!levelData.unscored);
        SSVOH_ASSERT(gotScoreInfo);
        SSVOH_ASSERT(hexagonClient.isLevelSupportedByServer(levelValidator));
        SSVOH_ASSERT(hexagonClient.getState() == HexagonClient::State::LoggedIn_Ready);


        const auto drawEntry =
            [&](const int i, const std::string& userNameStd, const sf::base::U64 scoreTimestamp, const double scoreValue)
        {
            const float score = scoreValue;

            const auto tp = Utils::toTimepoint(scoreTimestamp);

            const sf::base::String timestampStr = Utils::formatTimepoint(tp, "%Y-%m-%d %H:%M:%S");

            const sf::base::String posStr   = Utils::concat('#', i + 1);
            sf::base::String       scoreStr = sf::base::String(ssvu::toStr(score)) + 's';

            sf::base::String playerStr = sf::base::String(userNameStd);
            if (playerStr.size() > 19)
            {
                playerStr.resize(16);
                playerStr += "...";
            }

            const float tx = textToQuadBorder - panelOffset;
            const float ty = height - txtSelectionMedium.height * fontHeightOffset + txtSelectionSmall.height - 9.f;

            constexpr float ySpacing = 11.f;

            renderText(timestampStr, txtSelectionSmall.font, {tx, ty});
            renderText(posStr, txtSelectionMedium.font, {tx, ty + ySpacing});
            renderText(scoreStr, txtSelectionMedium.font, {tx + 58.f, ty + ySpacing});
            renderText(playerStr, txtSelectionMedium.font, {tx + 185.f, ty + ySpacing});

            height += txtSelectionMedium.height + txtSelectionSmall.height + txtSelectionSmall.height + 10.f;
        };

        if (gotScoreInfo)
        {
            const auto scores = leaderboardCache->getScores(levelValidator);

            if (!scores.empty())
            {
                int index = 0;
                for (const Database::ProcessedScore& ps : scores)
                {
                    drawEntry(index, ps.userName, ps.scoreTimestamp, ps.scoreValue);
                    ++index;
                }

                height -= txtSelectionMedium.height + txtSelectionSmall.height;
            }
            else
            {
                const float tx = textToQuadBorder - panelOffset;
                const float ty = height - txtSelectionMedium.height * fontHeightOffset;

                renderText("NO SCORES FOUND", txtSelectionMedium.font, {tx, ty});
            }
        }

        // Line
        height += txtSelectionScore.height + txtSelectionBig.height / 2.f;
        menuQuads.reserve_more_quad(1);
        createQuad(menuQuadColor, 0, width, height, height + lineThickness);
        height += lineThickness;

        height += txtSelectionSmall.height;

        renderText("YOUR POSITION",
                   txtSelectionSmall.font,
                   {textToQuadBorder - panelOffset, height - txtSelectionSmall.height * fontHeightOffset});

        height += txtSelectionSmall.height * 2.f + 5.f;

        if (gotScoreInfo)
        {
            const auto* ownScore = leaderboardCache->getOwnScore(levelValidator);

            if (ownScore == nullptr)
            {
                const float tx = textToQuadBorder - panelOffset;
                const float ty = height - txtSelectionMedium.height * fontHeightOffset;

                renderText("NO OWN SCORE SET", txtSelectionMedium.font, {tx, ty});
            }
            else
            {
                drawEntry(ownScore->position, ownScore->userName, ownScore->scoreTimestamp, ownScore->scoreValue);
            }
        }
    }

    drawOverlay(menuQuads);
}

void MenuGame::draw()
{
    mouseHovering   = false;
    mouseWasPressed = mousePressed;
    mousePressed    = (ignoreInputs == 0) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    if (mustRefresh)
    {
        mustRefresh = false;

        refreshCamera();
        adjustLevelsOffset();
        adjustMenuOffset(true);
        resetNamesScrolls();
    }

    styleData.computeColors();
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
    if (mainOrAbove && hgMenuBg != nullptr && useNewUI)
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
        if (winSz.x > 0 && winSz.y > 0 &&
            (!menuBgTexture.hasValue() || menuBgTexture->getSize() != winSz))
        {
            if (auto rt = sf::RenderTexture::create(winSz); rt.hasValue())
            {
                menuBgTexture          = SSVOH_MOVE(rt);
                menuBgTexture->setSmooth(true);
                hgMenuBg->renderTarget = &*menuBgTexture;
            }
        }
        if (winSz.x > 0 && winSz.y > 0 &&
            (!menuBgBlurTextureH.hasValue() || menuBgBlurTextureH->getSize() != winSz))
        {
            if (auto rt = sf::RenderTexture::create(winSz); rt.hasValue())
            {
                menuBgBlurTextureH = SSVOH_MOVE(rt);
                menuBgBlurTextureH->setSmooth(true);
            }
        }

        // Lazy-load the blur shader once. If the file's missing we fall
        // back to a passthrough copy below.
        if (!menuBgBlurShaderLoadAttempted)
        {
            menuBgBlurShaderLoadAttempted = true;
            if (auto sh = sf::Shader::loadFromFile({.fragmentPath = "Assets/menuBackgroundBlur.frag"});
                sh.hasValue())
            {
                menuBgBlurShader = SSVOH_MOVE(sh);
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
            const auto runPass = [&](const sf::Texture& srcTex, sf::RenderTarget& dstTarget,
                                     sf::Glsl::Vec2 direction)
            {
                const sf::Vec2u sz = srcTex.getSize();
                sf::RenderStates rs{};
                rs.texture = &srcTex;
                rs.view    = sf::View::fromScreenSize(sz.to<sf::Vec2f>());

                if (menuBgBlurShader.hasValue())
                {
                    if (auto loc = menuBgBlurShader->getUniformLocation("u_resolution"); loc.hasValue())
                    {
                        menuBgBlurShader->setUniform(*loc,
                                                     sf::Glsl::Vec2{static_cast<float>(sz.x),
                                                                    static_cast<float>(sz.y)});
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
                // Intermediate allocation failed — fall back to a single
                // pass so the menu still draws (just no blur).
                runPass(menuBgTexture->getTexture(), window.getRenderWindow(), {0.f, 0.f});
            }
        }
        else
        {
            // Texture allocation failed — fall back to direct render so
            // the menu still has a backdrop, just without the blur.
            hgMenuBg->getGame().onDraw();
        }

        window.draw(sf::RectangleShapeData{
            .size = window.getRenderWindow().getSize().toVec2f(),
            .fillColor = sf::Color{0, 0, 0, 25},
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

    // Only draw the legacy hexagon background past the loading screens —
    // skipped when the new UI's background level is doing the same job.
    if (mainOrAbove && (hgMenuBg == nullptr || !useNewUI))
    {
        menuBackgroundTris.clear();

        styleData.drawBackgroundMenu(menuBackgroundTris,
                                     sf::Vec2f{0.f, 0.f},
                                     levelStatus.sides,
                                     Config::getDarkenUnevenBackgroundChunk() && levelStatus.darkenUnevenBackgroundChunk,
                                     Config::getBlackAndWhite(),
                                     fourByThree);

        drawBackground(menuBackgroundTris);
    }

    // The legacy "CURRENT PROFILE: <name>" line has been removed (single-
    // profile model — see `playLocally`). Missing-dependency warnings
    // still surface if any packs need attention.
    if (mainOrAbove && state != States::LevelSelection)
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

    float indentBig{400.f}, indentSmall{540.f}, profileIndent{-100.f};
    // We need different values to fit menus in 4:3
    if (fourByThree)
    {
        indentBig     = 280.f;
        indentSmall   = 410.f;
        profileIndent = -75.f;
    }

    switch (state)
    {
        case States::EpilepsyWarning:
            drawOverlay(epilepsyWarning, sf::RenderStates{.texture = &txEpilepsyWarning});
            renderText("PRESS ANY KEY OR BUTTON TO CONTINUE", txtProf.font, {txtProf.height, h - txtProf.height * 2.7f + 5.f});
            return;

        case States::SMain:
            // New immediate-mode UI takes over the main screen.
            if (useNewUI)
            {
                drawNewMainMenu();
                // Legacy `drawGraphics()` painted the title bar logo,
                // version overlay and credits bars — all retired by the
                // new UI. Only the online-status bar still belongs here.
                drawOnlineStatus();
                break;
            }

            // Fold previous menus
            if (optionsMenu.getCategory().getOffset() != 0.f)
            {
                drawMainMenu(optionsMenu.getCategoryByName("options"), w - indentBig, true);
            }

            if (profileSelectionMenu.getCategory().getOffset() != 0.f)
            {
                drawProfileSelection(profileIndent, true);
            }

            if (enteringTextOffset != 0.f)
            {
                drawEnteringText(profileIndent, true);
            }

            if (levelDetailsOffset != 0.f)
            {
                drawLevelSelectionRightSide(*lvlDrawer, true);
                drawLevelSelectionLeftSide(*lvlDrawer, true);
            }

            // Draw main menu (right)
            drawMainSubmenus(mainMenu.getCategories(), indentBig);
            drawGraphics();
            drawOnlineStatus();
            break;

        case States::MOpts:
            // fold the main menu
            if (mainMenu.getCategory().getOffset() != 0.f)
            {
                drawMainMenu(mainMenu.getCategoryByName("main"), w - indentBig, true);
            }

            // Option Menu (right)
            drawMainMenu(optionsMenu.getCategoryByName("options"), w - indentBig, false);

            // Draw options submenus (left)
            drawSubmenusSmall(optionsMenu.getCategories(), indentSmall);
            drawGraphics();
            drawOnlineStatus();
            break;

        case States::MOnline:
            // fold the main menu
            if (mainMenu.getCategory().getOffset() != 0.f)
            {
                drawMainMenu(mainMenu.getCategoryByName("main"), w - indentBig, true);
            }

            // Online Menu (right)
            drawMainMenu(onlineMenu.getCategoryByName("options"), w - indentBig, false);

            // Draw online submenus (left)
            drawSubmenusSmall(onlineMenu.getCategories(), indentSmall);
            drawGraphics();
            drawOnlineStatus();
            break;

        case States::LevelSelection:
            // fold the main menu
            if (mainMenu.getCategory().getOffset() != 0.f)
            {
                drawMainMenu(mainMenu.getCategoryByName("main"), w - indentBig, true);
            }

            if (isFavoriteLevels())
            {
                drawLevelSelectionRightSide(favSlct, false);
                if (lvlSlct.XOffset != 0.f)
                {
                    drawLevelSelectionRightSide(lvlSlct, true);
                }
            }
            else
            {
                drawLevelSelectionRightSide(lvlSlct, false);
                if (favSlct.XOffset != 0.f)
                {
                    drawLevelSelectionRightSide(favSlct, true);
                }
            }

            drawLevelSelectionLeftSide(*lvlDrawer, false);
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

    if (!dialogBox.empty())
    {
        dialogBox.draw(getOverlayView(), dialogBoxTextColor, styleData.getColor(0));
    }

    if (!mouseWasPressed && mousePressed)
    {
        lastMouseClick = HRClock::now();
    }
}

[[nodiscard]] float MenuGame::getFPSMult() const
{
    // multiplier for FPS consistent drawing operations.
    return 200.f / window.getFPS();
}

void MenuGame::drawGraphics()
{
    drawOverlay(titleBar, sf::RenderStates{.texture = &txTitleBar});
    drawOverlay(creditsBar1, sf::RenderStates{.texture = &txCreditsBar1});
    drawOverlay(creditsBar2, sf::RenderStates{.texture = txCreditsBar2});
    drawOverlay(txtVersion.font);
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
