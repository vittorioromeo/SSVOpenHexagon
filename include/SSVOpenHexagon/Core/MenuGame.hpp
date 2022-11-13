// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/HexagonDialogBox.hpp"

#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/LevelStatus.hpp"

#include "SSVOpenHexagon/Utils/Clock.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/UniquePtr.hpp"

#include <SSVStart/Camera/Camera.hpp>

#include <SSVMenuSystem/SSVMenuSystem.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/System/Vector2.hpp>

#include <array>
#include <chrono>
#include <cctype>
#include <functional>
#include <optional>
#include <string_view>
#include <string>
#include <utility>
#include <vector>

namespace ssvs {
class GameWindow;
}

namespace ssvs::Input {
class Trigger;
}

namespace hg {

class HGAssets;
class Audio;
class HexagonGame;
class HexagonClient;
class LeaderboardCache;
class ProfileData;

struct PackData;
struct PackInfo;
struct LoadInfo;

namespace Steam {
class steam_manager;
}

namespace Discord {
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

    std::function<void(
        const std::string&, const std::string&, bool, float, bool)>
        fnHGNewGame;

    std::function<void()> fnHGUpdateRichPresenceCallbacks;

private:
    //---------------------------------------
    // Classes

    Steam::steam_manager& steamManager;
    Discord::discord_manager& discordManager;
    HGAssets& assets;
    sf::Font& openSquare;
    sf::Font& openSquareBold;
    Audio& audio;
    ssvs::GameState game;
    ssvs::GameWindow& window;
    HexagonClient& hexagonClient;
    HexagonDialogBox dialogBox;
    Utils::UniquePtr<LeaderboardCache> leaderboardCache;

    Lua::LuaContext lua;
    std::vector<std::string> execScriptPackPathContext;
    const PackData* currentPack;

    //---------------------------------------
    // Initialization

    void initAssets();
    void initInput();
    void initLua();
    void initMenus();
    void playLocally();

    [[nodiscard]] std::pair<const unsigned int, const unsigned int>
    pickRandomMainMenuBackgroundStyle();

    //---------------------------------------
    // Assets

    static constexpr std::array<const char*, 3> creditsIds{
        "creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png"};

    sf::Sprite titleBar;
    sf::Sprite creditsBar1;
    sf::Sprite creditsBar2;
    sf::Sprite epilepsyWarning;


    //---------------------------------------
    // Online status bar

    sf::Sprite sOnline;
    sf::RectangleShape rsOnlineStatus;
    sf::Text txtOnlineStatus;

    void initOnlineIcons();

    //---------------------------------------
    // Text Entering

    std::vector<char> enteredChars;

    [[nodiscard]] bool isEnteringText() const noexcept;

    //---------------------------------------
    // Cameras
    ssvs::Camera backgroundCamera;
    ssvs::Camera overlayCamera;

    bool mustRefresh;

    //---------------------------------------
    // Navigation

    bool wasFocusHeld;
    bool focusHeld;
    float wheelProgress;
    float touchDelay;
    States state;
    int packChangeDirection;

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
    [[nodiscard]] bool isInMenu() noexcept;
    void ignoreInputsAfterMenuExec();

    //---------------------------------------
    // State changes

    void changeStateTo(const States mState);

    //---------------------------------------
    // Update

    LevelStatus levelStatus;
    int ignoreInputs;

    void update(ssvu::FT mFT);
    void setIndex(int mIdx);
    void refreshCamera();
    void reloadAssets(const bool reloadEntirePack);
    void setIgnoreAllInputs(const unsigned int presses);

    //---------------------------------------
    // Drawing

    float w;
    float h;
    int scrollbarOffset{0};
    bool fourByThree{false};
    ssvms::Menu welcomeMenu;
    ssvms::Menu mainMenu;
    ssvms::Menu optionsMenu;
    ssvms::Menu onlineMenu;
    ssvms::Menu profileSelectionMenu;
    const LevelData* levelData;
    StyleData styleData;

    // The height of the font is an important value necessary to properly
    // draw all the menus and it only changes when the resolution does.
    // So we only calculate it on boot and when the res changes and store it.
    struct MenuFont
    {
        sf::Text font;
        float height;

        void updateHeight();
    };

    MenuFont txtVersion;
    MenuFont txtProf;
    MenuFont txtLoadBig;
    MenuFont txtLoadSmall;
    MenuFont txtRandomTip;
    MenuFont txtMenuBig;
    MenuFont txtMenuSmall;
    MenuFont txtMenuTiny;
    MenuFont txtProfile;
    MenuFont txtInstructionsBig;
    MenuFont txtInstructionsMedium;
    MenuFont txtInstructionsSmall;
    MenuFont txtEnteringText;
    MenuFont txtSelectionBig;
    MenuFont txtSelectionMedium;
    MenuFont txtSelectionSmall;
    MenuFont txtSelectionScore;
    MenuFont txtSelectionRanked;
    sf::Color menuTextColor;
    sf::Color menuQuadColor;
    sf::Color menuSelectionColor;
    sf::Color dialogBoxTextColor;
    Utils::FastVertexVectorTris menuBackgroundTris;
    Utils::FastVertexVectorTris menuQuads;

    // Mouse control
    HRTimePoint lastMouseClick{};
    bool mouseHovering{false};
    bool mouseWasPressed{false};
    bool mousePressed{false};
    bool mustFavorite{false};
    bool mustPlay{false};
    std::optional<int> mustChangeIndexTo;
    std::optional<int> mustChangePackIndexTo;
    std::optional<int> mustUseMenuItem;
    bool mouseCursorVisible{true};
    sf::Vector2i lastMouseMovedPosition{};

    std::string strBuf;

    void playSelectedLevel();

    void setMouseCursorVisible(const bool x);

    [[nodiscard]] bool isMouseCursorVisible() const;

    [[nodiscard]] bool overlayMouseOverlap(
        const sf::Vector2f& mins, const sf::Vector2f& maxs) const;

    [[nodiscard]] bool overlayMouseOverlapAndUpdateHover(
        const sf::Vector2f& mins, const sf::Vector2f& maxs);

    [[nodiscard]] sf::Color mouseOverlapColor(
        const bool mouseOverlap, const sf::Color& c) const;

    [[nodiscard]] bool mouseLeftRisingEdge() const;

    void draw();
    void render(sf::Drawable& mDrawable);

    // Helper functions
    [[nodiscard]] float getFPSMult() const;

    void drawGraphics();

    void drawOnlineStatus();

    void adjustMenuOffset(const bool resetMenuOffset);

    [[nodiscard]] float calcMenuOffset(float& offset, const float maxOffset,
        const bool revertOffset, const bool speedUp = false);

    void calcMenuItemOffset(float& offset, bool selected);

    void createQuad(
        const sf::Color& color, float x1, float x2, float y1, float y2);

    void createQuad(const sf::Color& color, const sf::Vector2f& mins,
        const sf::Vector2f& maxs);

    void createQuad(const sf::Color& color, const sf::FloatRect& rect);

    void createQuadTrapezoid(const sf::Color& color, float x1, float x2,
        float x3, float y1, float y2, bool left);

    [[nodiscard]] std::pair<int, int> getScrollbarNotches(
        const int size, const int maxSize) const;

    void drawScrollbar(const float totalHeight, const int size,
        const int notches, const float x, const float y,
        const sf::Color& color);

    void drawMainSubmenus(
        const std::vector<ssvms::UniquePtr<ssvms::Category>>& subMenus,
        const float indent);

    void drawSubmenusSmall(
        const std::vector<ssvms::UniquePtr<ssvms::Category>>& subMenus,
        const float indent);

    // Load menu
    LoadInfo& loadInfo;
    std::array<std::string_view, 2> randomTip;
    float hexagonRotation;
    void drawLoadResults();

    // Main menu
    float menuHalfHeight;

    void drawMainMenu(
        ssvms::Category& mSubMenu, float baseIndent, const bool revertOffset);

    // Options menu
    void drawOptionsSubmenus(
        ssvms::Category& mSubMenu, float baseIndent, const bool revertOffset);

    // Profiles Menu
    std::string formatSurvivalTime(ProfileData* data);
    void drawProfileSelection(const float xOffset, const bool revertOffset);
    void drawProfileSelectionBoot();

    // Entering text menu
    float enteringTextOffset;
    void drawEnteringText(const float xOffset, const bool revertOffset);
    void drawEnteringTextBoot();

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
        const std::vector<std::string>* levelDataIds;

        float XOffset{0.f};   // to make the menu slide in/out
        float YOffset{0.f};   // to scroll up and down the menu
        float YScrollTo{0.f}; // height list must scroll to show current item
        std::vector<float> lvlOffsets; // xOffset of the single level labels

        bool isFavorites{false};
    };

    bool isLevelFavorite;
    std::vector<std::string> favoriteLevelDataIds;
    LevelDrawer lvlSlct;
    LevelDrawer favSlct;
    LevelDrawer* lvlDrawer;

    void changeFavoriteLevelsToProfile();
    [[nodiscard]] bool isFavoriteLevels() const;
    [[nodiscard]] std::size_t getSelectablePackInfosSize() const;
    [[nodiscard]] const PackInfo& getNthSelectablePackInfo(const std::size_t i);

    int diffMultIdx{0};
    bool firstLevelSelection{true};
    PackChange packChangeState{PackChange::Rest};
    float namesScroll[static_cast<int>(Label::ScrollsSize)]{0};
    std::vector<std::string> levelDescription;
    float textToQuadBorder{0.f};
    float slctFrameSize{0.f};
    float packLabelHeight{0.f};
    float levelLabelHeight{0.f};
    float packChangeOffset{0.f}; // level list yOffset when being fold
    float levelDetailsOffset{0.f};
    static inline constexpr float baseScrollSpeed{30.f};
    float scrollSpeed{baseScrollSpeed};

    // Login at startup
    bool mustShowLoginAtStartup{true};
    void openLoginDialogBoxAndStartLoginProcess();

    // First timer tips
    bool showFirstTimeTips{false};
    bool mustShowFTTMainMenu{true};
    bool mustShowFTTLevelSelect{true};
    bool mustShowFTTDeathTips{true};
    float dialogBoxDelay{0.f};

    void addRemoveFavoriteLevel();
    void switchToFromFavoriteLevels();

    // Visual effects
    float difficultyBumpEffect{0.f};
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
    void scrollLevelListToTargetY(ssvu::FT mFT);

    void checkWindowTopScroll(
        const float scroll, std::function<void(const float)> action);
    bool checkWindowTopScrollWithResult(
        const float scroll, std::function<void(const float)> action);

    void checkWindowBottomScroll(
        const float scroll, std::function<void(const float)> action);
    bool checkWindowBottomScrollWithResult(
        const float scroll, std::function<void(const float)> action);

    void scrollName(std::string& text, float& scroller);

    void scrollNameRightBorder(std::string& text, const std::string key,
        sf::Text& font, float& scroller, float border);

    void scrollNameRightBorder(
        std::string& text, sf::Text& font, float& scroller, const float border);

    void resetNamesScrolls();

    void resetLevelNamesScrolls();

    [[nodiscard]] float getMaximumTextWidth() const;

    void formatLevelDescription();

    void drawLevelSelectionRightSide(
        LevelDrawer& drawer, const bool revertOffset);

    void drawLevelSelectionLeftSide(
        LevelDrawer& drawer, const bool revertOffset);

    // Text rendering
    void renderText(
        const std::string& mStr, sf::Text& mText, const sf::Vector2f& mPos);

    void renderText(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const sf::Color& mColor);

    void renderText(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos);

    void renderText(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos,
        const sf::Color& mColor);

    // Text rendering centered
    void renderTextCentered(
        const std::string& mStr, sf::Text& mText, const sf::Vector2f& mPos);

    void renderTextCentered(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const sf::Color& mColor);

    void renderTextCentered(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos);

    void renderTextCentered(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos,
        const sf::Color& mColor);

    // Text rendering centered with an offset
    void renderTextCenteredOffset(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const float xOffset);

    void renderTextCenteredOffset(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const float xOffset, const sf::Color& mColor);

    //---------------------------------------
    // Misc / Unused

    std::string scoresMessage;
    float exitTimer{0}, currentCreditsId{0};
    bool mustTakeScreenshot{false};
    std::string currentLeaderboard, enteredStr, leaderboardString;

    void runLuaFile(const std::string& mFileName);
    void changeResolutionTo(unsigned int mWidth, unsigned int mHeight);
    void playSoundOverride(const std::string& assetId);

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

    std::string registrationUsername;
    std::string registrationPassword;
    std::string registrationPasswordConfirm;
    std::string loginUsername;
    std::string loginPassword;
    std::string deleteAccountPassword;

    void showDialogBox(const std::string& msg);
    void showInputDialogBox(const std::string& msg);
    void showInputDialogBoxNice(const std::string& title,
        const std::string& inputType, const std::string& extra = "");
    void showInputDialogBoxNiceWithDefault(const std::string& title,
        const std::string& inputType, const std::string& def,
        const std::string& extra = "");

public:
    MenuGame(Steam::steam_manager& mSteamManager,
        Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
        Audio& mAudio, ssvs::GameWindow& mGameWindow,
        HexagonClient& mHexagonClient);

    ~MenuGame();

    void init(bool mErrored);
    void init(bool mErrored, const std::string& pack, const std::string& level);

    bool loadCommandLineLevel(
        const std::string& pack, const std::string& level);

    [[nodiscard]] ssvs::GameState& getGame() noexcept;

    void returnToLevelSelection();

    void refreshBinds();
};

} // namespace hg
