// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/HexagonDialogBox.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/LevelStatus.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/GameSystem/GameState.hpp"
#include "SSVOpenHexagon/GameSystem/GameWindow.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"
#include "SSVOpenHexagon/Utils/CameraView.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

namespace ssvs::Input
{
class Trigger;
}

namespace hg
{

class HGAssets;
class Audio;
class HexagonGame;
class HexagonClient;
class LeaderboardCache;
class ProfileData;

struct PackData;
struct PackInfo;
struct LoadInfo;
struct replay_file;

namespace Steam
{
class steam_manager;
}

namespace Discord
{
class discord_manager;
}

enum class States
{
    EpilepsyWarning,
    SMain
};

class MenuGame
{
public:
    //---------------------------------------
    // Hexagon game callbacks (to avoid physical dependency on
    // `HexagonGame`). All four are wired up by `main.cpp` once both
    // the menu and the foreground gameplay HG have been constructed.
    // Bundled into a single struct so the install block in `main.cpp`
    // reads as one cohesive unit and adding a fifth callback later
    // doesn't keep growing this header's public surface.
    struct HostCallbacks
    {
        // Re-bind a single trigger/binding on the gameplay HG. Called
        // when keyboard rebinds in the Options screen take effect.
        sf::base::FixedFunction<void(const ssvs::Input::Trigger&, int), 64> triggerRefresh;

        // Start a fresh gameplay run on the foreground HG.
        // Args: packId, levelId, firstPlay, difficultyMult, executeLastReplay.
        sf::base::FixedFunction<void(const sf::base::String&, const sf::base::String&, bool, float, bool), 64> newGame;

        // Watch a server-streamed replay. Receives a fully decompressed
        // `replay_file`. Wraps `setLastReplay + newGame(...,
        // executeLastReplay=true)` so the foreground HG plays the
        // replay back like a normal game.
        sf::base::FixedFunction<void(const replay_file&), 96> watchReplay;

        // Tick Discord/Steam rich-presence on the gameplay HG. Called
        // each menu update so the "what's the player doing right now"
        // reflects the current menu state.
        sf::base::FixedFunction<void(), 64> updateRichPresence;
    };

    HostCallbacks hostCallbacks;

private:
    [[nodiscard]] sf::View getOverlayView() const
    {
        return Utils::computeCameraView(overlayCamera, overlayCameraTransform);
    }

    template <typename TDrawable>
    void drawWithView(const sf::View& view, const TDrawable& drawable, sf::RenderStates states = {})
    {
        states.view = view;
        window.getRenderWindow().draw(drawable, states);
    }

    template <typename TDrawable>
    void drawOverlay(const TDrawable& drawable, sf::RenderStates states = {})
    {
        drawWithView(getOverlayView(), drawable, states);
    }

    template <typename TDrawable>
    void drawScreen(const TDrawable& drawable, sf::RenderStates states = {})
    {
        drawWithView(sf::View{.center = {0.f, 0.f}, .size = {getWindowWidth(), getWindowHeight()}}, drawable, states);
    }

    //---------------------------------------
    // Classes

    Steam::steam_manager&                 steamManager;
    Discord::discord_manager&             discordManager;
    HGAssets&                             assets;
    sf::Font&                             openSquare;
    sf::Font&                             openSquareBold;
    Audio&                                audio;
    ssvs::GameState                       game;
    ssvs::GameWindow&                     window;
    HexagonClient&                        hexagonClient;
    HexagonDialogBox                      dialogBox;
    sf::base::UniquePtr<LeaderboardCache> leaderboardCache;

    Lua::LuaContext                    lua;
    sf::base::Vector<sf::base::String> execScriptPackPathContext;
    const PackData*                    currentPack;

    //---------------------------------------
    // New immediate-mode UI (see `docs/UI_REWRITE_DESIGN.md`).

    // The new UI's persistent state (one-per-MenuGame).
    hg::ui::App      ui_app;
    hg::ui::Services ui_services;

    // Edges accumulated by the existing `*Action()` callbacks while the new
    // UI is active for the current screen. Drained into a fresh `ui::Input`
    // each frame in `drawNewMainMenu`, then reset.
    hg::ui::Input ui_pendingInput;

    // Last frame time in seconds, captured at top of `update()`. The new UI's
    // animation helpers read it via `Context::dt`.
    float ui_dt{1.f / 60.f};

    // Optional `HexagonGame` instances used as menu visuals. Both run in
    // `previewMode`. `hgMenuBg` plays a fixed backdrop level under the
    // entire menu; `hgPreview` renders the currently-selected level into
    // `previewTexture` for display in LevelSelect. Owned by `main.cpp`,
    // installed via `setMenuPreviewGames`.
    HexagonGame*                          hgMenuBg{nullptr};
    HexagonGame*                          hgPreview{nullptr};
    sf::base::Optional<sf::RenderTexture> previewTexture;
    sf::base::String                      previewLoadedLevelId; //!< prevents reloading on every frame

    // Validator for the (level, difficulty) currently shown in the
    // LevelSelect leaderboard pane. Set by `Services::onRequestLeaderboard`
    // and consumed each frame to publish `Services::leaderboardScores` from
    // `leaderboardCache`. Empty when no level is highlighted yet.
    sf::base::String currentLeaderboardValidator;

    // Last (validator, scoreTimestamp) pair we asked the server for.
    // Used by `onWatchReplay` to suppress duplicate requests when the
    // user mashes Enter on the same row -- the server already has a
    // throttle on its side, but we save bandwidth by stopping the spam
    // here. Cleared when the validator changes so re-watching a row
    // after a navigation away/back works.
    sf::base::String lastReplayRequestValidator;
    sf::base::U64    lastReplayRequestTimestamp{0};

    // Refresh `Services::{leaderboardScores,leaderboardStatus,leaderboardUnavailable}`
    // from the current `HexagonClient` state + `leaderboardCache` contents.
    // Called once per frame at the top of `drawNewMainMenu`. Selection
    // changes during the frame (via `onRequestLeaderboard`) accept a
    // one-frame display lag rather than re-publishing inline.
    void refreshLeaderboardSnapshot();

    // Helpers for `onRequestLeaderboard`, kept on the class so the
    // public lambda stays a one-liner. See implementations for the
    // detailed contracts.
    [[nodiscard]] bool tryUpdateLeaderboardValidator(const sf::base::String& levelId, float diffMult);
    void               maybeIssueLeaderboardTopScoresRequest();

    // Off-screen target the new UI renders into. Once filled each frame, a
    // full-screen quad copies it back to the window through
    // `menuAccentShader`, which replaces every magenta-saturated pixel with
    // an animated noise gradient. That's how text outlines and selection
    // pills get their colorful look without us having to compute per-pixel
    // gradients in the immediate-mode UI itself.
    sf::base::Optional<sf::RenderTexture> uiCompositeTexture;
    sf::base::Optional<sf::Shader>        menuAccentShader;
    bool                                  menuAccentShaderLoadAttempted{false};
    float                                 menuAccentShaderTime{0.f};

    // Off-screen target `hgMenuBg` paints into; composited back to the
    // window through `menuBgBlurShader` with a `u_blur` uniform driven by
    // `ui_app.backDepthAnim`. Result: the level visuals behind the menu
    // start crisp on the main screen and ease into a soft gaussian blur
    // as the user enters sub-screens.
    sf::base::Optional<sf::RenderTexture> menuBgTexture;
    // Intermediate ping-pong target between the horizontal and vertical
    // passes of the separable gaussian blur.
    sf::base::Optional<sf::RenderTexture> menuBgBlurTextureH;
    sf::base::Optional<sf::Shader>        menuBgBlurShader;
    bool                                  menuBgBlurShaderLoadAttempted{false};

public:
    // Installed once by the host (`main.cpp`) after both auxiliary HG
    // instances have been constructed. Kicks off the menu-background level
    // and prepares the preview render texture.
    void setMenuPreviewGames(HexagonGame*     menuBackground,
                             HexagonGame*     preview,
                             sf::base::String menuBackgroundPackId,
                             sf::base::String menuBackgroundLevelId);

    // True iff the new UI is the right place to handle this frame's input
    // for the given state. Used to short-circuit old `*Action()` cascades.
    [[nodiscard]] bool newUIActiveForCurrentState() const noexcept;

    void initNewUIServices();
    void drawNewMainMenu();

    // Drains pending Steam Workshop events each frame: hot-installs newly-
    // downloaded packs, mirrors subscribe/unsubscribe state into the
    // browse-screen's cached item list, etc. No-op if Steam isn't running.
    void pumpWorkshopEvents();

    //---------------------------------------
    // Initialization

    void initAssets();
    void initInput();
    void initLua();
    void playLocally();

    //---------------------------------------
    // Assets

    sf::Texture& txTitleBar;
    sf::Texture& txEpilepsyWarning;

    sf::Sprite titleBar;
    sf::Sprite epilepsyWarning;


    //---------------------------------------
    // Online status bar

    sf::Texture*       txSOnline;
    sf::Sprite         sOnline;
    sf::RectangleShape rsOnlineStatus;
    sf::Text           txtOnlineStatus;

    void initOnlineIcons();

    //---------------------------------------
    // Cameras
    sf::View             backgroundCamera;
    sf::View             overlayCamera;
    Utils::ViewTransform backgroundCameraTransform;
    Utils::ViewTransform overlayCameraTransform;

    bool mustRefresh;

    //---------------------------------------
    // Navigation

    States state;

    void ignoreInputsAfterMenuExec();

    //---------------------------------------
    // State changes

    void changeStateTo(const States mState);

    //---------------------------------------
    // Update

    LevelStatus levelStatus;
    int         ignoreInputs;

    void update(float mFT);
    void refreshCamera();
    void setIgnoreAllInputs(const unsigned int presses);

    //---------------------------------------
    // Drawing

    float            w;
    float            h;
    bool             fourByThree{false};
    const LevelData* levelData;
    StyleData        styleData;

    // The height of the font is an important value necessary to properly
    // draw all the menus and it only changes when the resolution does.
    // So we only calculate it on boot and when the res changes and store it.
    struct MenuFont
    {
        sf::Text font;
        float    height;

        void updateHeight();
    };

    // The two surviving labels: `txtProf` for the EpilepsyWarning splash,
    // `txtSelectionSmall` for the missing-pack warning anchored under the
    // (no longer drawn) title bar.
    MenuFont txtProf;
    MenuFont txtSelectionSmall;

    // Mouse state, latched at the start of `draw` and consumed by
    // `drawNewMainMenu` to feed the new UI's input snapshot.
    bool mouseWasPressed{false};
    bool mousePressed{false};

    sf::base::String strBuf;

    void setMouseCursorVisible(const bool x);

    void draw();

    void drawOnlineStatus();

    // Login at startup
    bool mustShowLoginAtStartup{true};
    void openLoginDialogBoxAndStartLoginProcess();

    // First timer tips
    bool  showFirstTimeTips{false};
    bool  mustShowFTTMainMenu{true};
    float dialogBoxDelay{0.f};

    // Single-overload renderer used by the surviving text draws.
    void renderText(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos);

    bool mustTakeScreenshot{false};

    void runLuaFile(const sf::base::String& mFileName);
    void changeResolutionTo(unsigned int mWidth, unsigned int mHeight);
    void playSoundOverride(const sf::base::String& assetId);

    [[nodiscard]] float getWindowWidth() const noexcept;

    [[nodiscard]] float getWindowHeight() const noexcept;

    //---------------------------------------
    // Input boxes

    enum class DialogInputState
    {
        Nothing,

        Registration_EnteringUsername,
        Registration_EnteringPassword,
        Registration_EnteringPasswordConfirm,

        Login_EnteringUsername,
        Login_EnteringPassword,

        DeleteAccount_EnteringPassword,
    };

    DialogInputState dialogInputState{DialogInputState::Nothing};

    sf::base::String registrationUsername;
    sf::base::String registrationPassword;
    sf::base::String registrationPasswordConfirm;
    sf::base::String loginUsername;
    sf::base::String loginPassword;
    sf::base::String deleteAccountPassword;

    void showDialogBox(const sf::base::String& msg);
    void showInputDialogBox(const sf::base::String& msg);
    void showInputDialogBoxNice(const sf::base::String& title,
                                const sf::base::String& inputType,
                                const sf::base::String& extra = "");
    void showInputDialogBoxNiceWithDefault(const sf::base::String& title,
                                           const sf::base::String& inputType,
                                           const sf::base::String& def,
                                           const sf::base::String& extra = "");

public:
    MenuGame(Steam::steam_manager&     mSteamManager,
             Discord::discord_manager& mDiscordManager,
             HGAssets&                 mAssets,
             Audio&                    mAudio,
             ssvs::GameWindow&         mGameWindow,
             HexagonClient&            mHexagonClient);

    ~MenuGame();

    void init(bool mErrored);
    void init(bool mErrored, const sf::base::String& pack, const sf::base::String& level);

    bool loadCommandLineLevel(const sf::base::String& pack, const sf::base::String& level);

    [[nodiscard]] ssvs::GameState& getGame() noexcept;

    void returnToLevelSelection();

    void refreshBinds();
};

} // namespace hg
