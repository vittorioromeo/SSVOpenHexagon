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
#include "SSVOpenHexagon/Utils/CameraView.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <SSVOpenHexagon/MenuSystem/SSVMenuSystem.hpp>
#include <functional>
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
    ETLPNewBoot,
    LoadingScreen,
    EpilepsyWarning,
    SLPSelectBoot,
    SMain,
    LevelSelection,
    MOpts,
    MOnline,
    SLPSelect,
    ETLPNew
};

class MenuGame
{
public:
    //---------------------------------------
    // Hexagon game callbacks (to avoid physical dependency)
    std::function<void(const ssvs::Input::Trigger&, int)> fnHGTriggerRefresh;

    std::function<void(const sf::base::String&, const sf::base::String&, bool, float, bool)> fnHGNewGame;

    std::function<void()> fnHGUpdateRichPresenceCallbacks;

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
        drawWithView(sf::View{{0.f, 0.f}, {getWindowWidth(), getWindowHeight()}}, drawable);
    }

    template <typename TDrawable>
    void drawScreen(const TDrawable& drawable, sf::RenderStates states)
    {
        drawWithView(sf::View{{0.f, 0.f}, {getWindowWidth(), getWindowHeight()}}, drawable, states);
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

    bool   wasFocusHeld;
    bool   focusHeld;
    float  wheelProgress;
    float  touchDelay;
    States state;
    int    packChangeDirection;

    void leftRightActionImpl(bool left);
    void leftAction();
    void rightAction();
    void upAction();
    void downAction();
    void okAction();
    void eraseAction();
    void exitAction();

    void changePackTo(const int idx);
    void changePack();
    void changePackQuick(const int direction);
    void changePackAction(const int direction);

    [[nodiscard]] ssvms::Menu* getCurrentMenu() noexcept;
    [[nodiscard]] bool         isInMenu() noexcept;
    void                       ignoreInputsAfterMenuExec();

    //---------------------------------------
    // State changes

    void changeStateTo(const States mState);

    //---------------------------------------
    // Update

    LevelStatus levelStatus;
    int         ignoreInputs;

    void update(float mFT);
    void setIndex(int mIdx);
    void refreshCamera();
    void reloadAssets(const bool reloadEntirePack);
    void setIgnoreAllInputs(const unsigned int presses);

    //---------------------------------------
    // Drawing

    float            w;
    float            h;
    int              scrollbarOffset{0};
    bool             fourByThree{false};
    ssvms::Menu      welcomeMenu;
    ssvms::Menu      mainMenu;
    ssvms::Menu      optionsMenu;
    ssvms::Menu      onlineMenu;
    ssvms::Menu      profileSelectionMenu;
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
    Utils::FastVertexVectorTris menuQuads;

    // Mouse control
    HRTimePoint             lastMouseClick{};
    bool                    mouseHovering{false};
    bool                    mouseWasPressed{false};
    bool                    mousePressed{false};
    bool                    mustFavorite{false};
    bool                    mustPlay{false};
    sf::base::Optional<int> mustChangeIndexTo;
    sf::base::Optional<int> mustChangePackIndexTo;
    sf::base::Optional<int> mustUseMenuItem;
    bool                    mouseCursorVisible{true};
    sf::Vec2i               lastMouseMovedPosition{};

    sf::base::String strBuf;

    void playSelectedLevel();

    void setMouseCursorVisible(const bool x);

    [[nodiscard]] bool isMouseCursorVisible() const;

    [[nodiscard]] bool overlayMouseOverlap(const sf::Vec2f mins, const sf::Vec2f maxs) const;

    [[nodiscard]] bool overlayMouseOverlapAndUpdateHover(const sf::Vec2f mins, const sf::Vec2f maxs);

    [[nodiscard]] sf::Color mouseOverlapColor(const bool mouseOverlap, const sf::Color& c) const;

    [[nodiscard]] bool mouseLeftRisingEdge() const;

    void draw();

    // Helper functions
    [[nodiscard]] float getFPSMult() const;

    void drawGraphics();

    void drawOnlineStatus();

    void adjustMenuOffset(const bool resetMenuOffset);

    [[nodiscard]] float calcMenuOffset(float& offset, const float maxOffset, const bool revertOffset, const bool speedUp = false);

    void calcMenuItemOffset(float& offset, bool selected);

    void createQuad(const sf::Color& color, float x1, float x2, float y1, float y2);

    void createQuad(const sf::Color& color, const sf::Vec2f mins, const sf::Vec2f maxs);

    void createQuad(const sf::Color& color, const sf::Rect2f& rect);

    void createQuadTrapezoid(const sf::Color& color, float x1, float x2, float x3, float y1, float y2, bool left);

    [[nodiscard]] std::pair<int, int> getScrollbarNotches(const int size, const int maxSize) const;

    void drawScrollbar(const float      totalHeight,
                       const int        size,
                       const int        notches,
                       const float      x,
                       const float      y,
                       const sf::Color& color);

    void drawMainSubmenus(const sf::base::Vector<sf::base::UniquePtr<ssvms::Category>>& subMenus, const float indent);

    void drawSubmenusSmall(const sf::base::Vector<sf::base::UniquePtr<ssvms::Category>>& subMenus, const float indent);

    // Load menu
    LoadInfo&                            loadInfo;
    sf::base::Array<std::string_view, 2> randomTip;
    float                                hexagonRotation;
    void                                 drawLoadResults();

    // Main menu
    float menuHalfHeight;

    void drawMainMenu(ssvms::Category& mSubMenu, float baseIndent, const bool revertOffset);

    // Options menu
    void drawOptionsSubmenus(ssvms::Category& mSubMenu, float baseIndent, const bool revertOffset);

    // Profiles Menu
    sf::base::String formatSurvivalTime(ProfileData* data);
    void             drawProfileSelection(const float xOffset, const bool revertOffset);
    void             drawProfileSelectionBoot();

    // Entering text menu
    float enteringTextOffset;
    void  drawEnteringText(const float xOffset, const bool revertOffset);
    void  drawEnteringTextBoot();

    // Level selection menu
    enum class PackChange
    {
        Rest,
        Folding,
        Stretching
    };

    enum class Label
    {
        LevelName,
        PackName,
        PackAuthor,
        MusicName,
        MusicAuthor,
        MusicAlbum,
        ScrollsSize
    };

    // To keep data of the regular level selection and favorites separated.
    struct LevelDrawer
    {
        int packIdx{0};
        int currentIndex{0};

        // Pointer to avoid heavy copy loads.
        const sf::base::Vector<sf::base::String>* levelDataIds;

        float                   XOffset{0.f};   // to make the menu slide in/out
        float                   YOffset{0.f};   // to scroll up and down the menu
        float                   YScrollTo{0.f}; // height list must scroll to show current item
        sf::base::Vector<float> lvlOffsets;     // xOffset of the single level labels

        bool isFavorites{false};
    };

    bool                               isLevelFavorite;
    sf::base::Vector<sf::base::String> favoriteLevelDataIds;
    LevelDrawer                        lvlSlct;
    LevelDrawer                        favSlct;
    LevelDrawer*                       lvlDrawer;

    void                          changeFavoriteLevelsToProfile();
    [[nodiscard]] bool            isFavoriteLevels() const;
    [[nodiscard]] sf::base::SizeT getSelectablePackInfosSize() const;
    [[nodiscard]] const PackInfo& getNthSelectablePackInfo(const sf::base::SizeT i);

    int                                diffMultIdx{0};
    bool                               firstLevelSelection{true};
    PackChange                         packChangeState{PackChange::Rest};
    float                              namesScroll[static_cast<int>(Label::ScrollsSize)]{0};
    sf::base::Vector<sf::base::String> levelDescription;
    float                              textToQuadBorder{0.f};
    float                              slctFrameSize{0.f};
    float                              packLabelHeight{0.f};
    float                              levelLabelHeight{0.f};
    float                              packChangeOffset{0.f}; // level list yOffset when being fold
    float                              levelDetailsOffset{0.f};
    static inline constexpr float      baseScrollSpeed{30.f};
    float                              scrollSpeed{baseScrollSpeed};

    // Login at startup
    bool mustShowLoginAtStartup{true};
    void openLoginDialogBoxAndStartLoginProcess();

    // First timer tips
    bool  showFirstTimeTips{false};
    bool  mustShowFTTMainMenu{true};
    bool  mustShowFTTLevelSelect{true};
    bool  mustShowFTTDeathTips{true};
    float dialogBoxDelay{0.f};

    void addRemoveFavoriteLevel();
    void switchToFromFavoriteLevels();

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

    void checkWindowTopScroll(const float scroll, std::function<void(const float)> action);
    bool checkWindowTopScrollWithResult(const float scroll, std::function<void(const float)> action);

    void checkWindowBottomScroll(const float scroll, std::function<void(const float)> action);
    bool checkWindowBottomScrollWithResult(const float scroll, std::function<void(const float)> action);

    void scrollName(sf::base::String& text, float& scroller);

    void scrollNameRightBorder(sf::base::String& text, const sf::base::String key, sf::Text& font, float& scroller, float border);

    void scrollNameRightBorder(sf::base::String& text, sf::Text& font, float& scroller, const float border);

    void resetNamesScrolls();

    void resetLevelNamesScrolls();

    [[nodiscard]] float getMaximumTextWidth() const;

    void formatLevelDescription();

    void drawLevelSelectionRightSide(LevelDrawer& drawer, const bool revertOffset);

    void drawLevelSelectionLeftSide(LevelDrawer& drawer, const bool revertOffset);

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
