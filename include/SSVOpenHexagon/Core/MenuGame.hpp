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
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include "SFML/Graphics/Color.hpp"
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

#include "SFML/Base/Array.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <string_view>
#include <utility>

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
    // Hexagon game callbacks (to avoid physical dependency)
    sf::base::FixedFunction<void(const ssvs::Input::Trigger&, int), 64> fnHGTriggerRefresh;

    sf::base::FixedFunction<void(const sf::base::String&, const sf::base::String&, bool, float, bool), 64> fnHGNewGame;

    sf::base::FixedFunction<void(), 64> fnHGUpdateRichPresenceCallbacks;

private:
    [[nodiscard]] sf::View getBackgroundView() const
    {
        return Utils::computeCameraView(backgroundCamera, backgroundCameraTransform);
    }

    [[nodiscard]] sf::View getOverlayView() const
    {
        return Utils::computeCameraView(overlayCamera, overlayCameraTransform);
    }

    template <typename TDrawable>
    void drawWithView(const sf::View& view, const TDrawable& drawable)
    {
        drawWithView(view, drawable, sf::RenderStates{});
    }

    template <typename TDrawable>
    void drawWithView(const sf::View& view, const TDrawable& drawable, sf::RenderStates states)
    {
        states.view = view;
        window.getRenderWindow().draw(drawable, states);
    }

    template <typename TDrawable>
    void drawBackground(const TDrawable& drawable)
    {
        drawWithView(getBackgroundView(), drawable);
    }

    template <typename TDrawable>
    void drawBackground(const TDrawable& drawable, sf::RenderStates states)
    {
        drawWithView(getBackgroundView(), drawable, states);
    }

    template <typename TDrawable>
    void drawOverlay(const TDrawable& drawable)
    {
        drawWithView(getOverlayView(), drawable);
    }

    template <typename TDrawable>
    void drawOverlay(const TDrawable& drawable, sf::RenderStates states)
    {
        drawWithView(getOverlayView(), drawable, states);
    }

    template <typename TDrawable>
    void drawScreen(const TDrawable& drawable)
    {
        drawWithView(sf::View{.center = {0.f, 0.f}, .size = {getWindowWidth(), getWindowHeight()}}, drawable);
    }

    template <typename TDrawable>
    void drawScreen(const TDrawable& drawable, sf::RenderStates states)
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

    // Pushes the previewed level's `styleData` colors (plus a contrasting
    // row backdrop) into the new UI's theme. Run each frame because pulsing
    // styles can shift hue.
    void applyLevelThemeToContext(hg::ui::Context& ctx) const;

    // Drains pending Steam Workshop events each frame: hot-installs newly-
    // downloaded packs, mirrors subscribe/unsubscribe state into the
    // browse-screen's cached item list, etc. No-op if Steam isn't running.
    void pumpWorkshopEvents();

    //---------------------------------------
    // Initialization

    void initAssets();
    void initInput();
    void initLua();
    void initMenus();
    void playLocally();

    [[nodiscard]] std::pair<const unsigned int, const unsigned int> pickRandomMainMenuBackgroundStyle();

    //---------------------------------------
    // Assets

    static constexpr sf::base::Array<const char*, 3> creditsIds{"creditsBar2.png",
                                                                "creditsBar2b.png",
                                                                "creditsBar2c.png"};

    sf::Texture& txTitleBar;
    sf::Texture& txCreditsBar1;
    sf::Texture* txCreditsBar2;
    sf::Texture& txEpilepsyWarning;

    sf::Sprite titleBar;
    sf::Sprite creditsBar1;
    sf::Sprite creditsBar2;
    sf::Sprite epilepsyWarning;


    //---------------------------------------
    // Online status bar

    sf::Texture*       txSOnline;
    sf::Sprite         sOnline;
    sf::RectangleShape rsOnlineStatus;
    sf::Text           txtOnlineStatus;

    void initOnlineIcons();

    //---------------------------------------
    // Text Entering

    sf::base::Vector<char> enteredChars;

    [[nodiscard]] bool isEnteringText() const noexcept;

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
    int              scrollbarOffset{0};
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

    MenuFont                    txtVersion;
    MenuFont                    txtProf;
    MenuFont                    txtLoadBig;
    MenuFont                    txtLoadSmall;
    MenuFont                    txtRandomTip;
    MenuFont                    txtMenuBig;
    MenuFont                    txtMenuSmall;
    MenuFont                    txtMenuTiny;
    MenuFont                    txtProfile;
    MenuFont                    txtInstructionsBig;
    MenuFont                    txtInstructionsMedium;
    MenuFont                    txtInstructionsSmall;
    MenuFont                    txtEnteringText;
    MenuFont                    txtSelectionBig;
    MenuFont                    txtSelectionMedium;
    MenuFont                    txtSelectionSmall;
    MenuFont                    txtSelectionScore;
    MenuFont                    txtSelectionRanked;
    sf::Color                   menuTextColor;
    sf::Color                   menuQuadColor;
    sf::Color                   menuSelectionColor;
    sf::Color                   dialogBoxTextColor;
    Utils::FastVertexVectorTris menuBackgroundTris;

    // Mouse control
    bool      mouseHovering{false};
    bool      mouseWasPressed{false};
    bool      mousePressed{false};
    bool      mouseCursorVisible{true};
    sf::Vec2i lastMouseMovedPosition{};

    sf::base::String strBuf;

    void setMouseCursorVisible(const bool x);

    [[nodiscard]] bool isMouseCursorVisible() const;

    void draw();

    // Helper functions
    [[nodiscard]] float getFPSMult() const;

    void drawOnlineStatus();

    void adjustMenuOffset(const bool resetMenuOffset);

    // Load menu data still surfaces missing-pack warnings on the main
    // screen, so `loadInfo` and `randomTip` remain. The legacy boot-time
    // load-results draw and its supporting graphics fields are gone.
    LoadInfo&                            loadInfo;
    sf::base::Array<std::string_view, 2> randomTip;
    float                                hexagonRotation;

    // Main menu
    float menuHalfHeight;

    // Profiles Menu
    sf::base::String formatSurvivalTime(ProfileData* data);

    // Entering text menu (legacy text-input dialog state)
    float enteringTextOffset;

    // Login at startup
    bool mustShowLoginAtStartup{true};
    void openLoginDialogBoxAndStartLoginProcess();

    // First timer tips
    bool  showFirstTimeTips{false};
    bool  mustShowFTTMainMenu{true};
    float dialogBoxDelay{0.f};

    // Visual effects
    float                         difficultyBumpEffect{0.f};
    static inline constexpr float difficultyBumpEffectMax{24.f};

    void adjustLevelsOffset();
    void updateLevelSelectionDrawingParameters();

    [[nodiscard]] float getLevelSelectionHeight() const;
    [[nodiscard]] float getLevelListHeight() const;

    void calcScrollSpeed();
    void calcLevelChangeScroll(const int dir);
    void calcPackChangeScrollFold(const float mLevelListHeight);
    void calcPackChangeScrollStretch(const float mLevelListHeight);
    void quickPackFoldStretch();
    void scrollLevelListToTargetY(float mFT);

    void checkWindowTopScroll(const float scroll, sf::base::FixedFunction<void(const float), 64> action);
    bool checkWindowTopScrollWithResult(const float scroll, sf::base::FixedFunction<void(const float), 64> action);

    void checkWindowBottomScroll(const float scroll, sf::base::FixedFunction<void(const float), 64> action);
    bool checkWindowBottomScrollWithResult(const float scroll, sf::base::FixedFunction<void(const float), 64> action);

    void scrollName(sf::base::String& text, float& scroller);

    void scrollNameRightBorder(sf::base::String& text, const sf::base::String key, sf::Text& font, float& scroller, float border);

    void scrollNameRightBorder(sf::base::String& text, sf::Text& font, float& scroller, const float border);

    void resetNamesScrolls();

    void resetLevelNamesScrolls();

    [[nodiscard]] float getMaximumTextWidth() const;

    void formatLevelDescription();

    // Text rendering
    void renderText(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos);

    void renderText(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos, const sf::Color& mColor);

    void renderText(const sf::base::String& mStr, sf::Text& mText, const unsigned int mSize, const sf::Vec2f mPos);

    void renderText(const sf::base::String& mStr,
                    sf::Text&               mText,
                    const unsigned int      mSize,
                    const sf::Vec2f         mPos,
                    const sf::Color&        mColor);

    // Text rendering centered
    void renderTextCentered(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos);

    void renderTextCentered(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos, const sf::Color& mColor);

    void renderTextCentered(const sf::base::String& mStr, sf::Text& mText, const unsigned int mSize, const sf::Vec2f mPos);

    void renderTextCentered(const sf::base::String& mStr,
                            sf::Text&               mText,
                            const unsigned int      mSize,
                            const sf::Vec2f         mPos,
                            const sf::Color&        mColor);

    // Text rendering centered with an offset
    void renderTextCenteredOffset(const sf::base::String& mStr, sf::Text& mText, const sf::Vec2f mPos, const float xOffset);

    void renderTextCenteredOffset(const sf::base::String& mStr,
                                  sf::Text&               mText,
                                  const sf::Vec2f         mPos,
                                  const float             xOffset,
                                  const sf::Color&        mColor);

    //---------------------------------------
    // Misc / Unused

    sf::base::String scoresMessage;
    float            exitTimer{0}, currentCreditsId{0};
    bool             mustTakeScreenshot{false};
    sf::base::String currentLeaderboard, enteredStr, leaderboardString;

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
