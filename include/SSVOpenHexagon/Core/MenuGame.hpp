// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/HexagonDialogBox.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/FontHeight.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Core/LuaScripting.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include <SSVStart/GameSystem/GameSystem.hpp>
#include <SSVStart/Camera/Camera.hpp>
#include <SSVStart/VertexVector/VertexVector.hpp>
#include <SSVStart/Utils/Vector2.hpp>

#include <SSVMenuSystem/SSVMenuSystem.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <cctype>
#include <array>
#include <utility>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <chrono>
#include <optional>

namespace hg
{

class Audio;
class HexagonGame;
class HexagonClient;

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

class LeaderboardCache
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

private:
    struct CachedScores
    {
        std::vector<Database::ProcessedScore> _scores;
        std::optional<Database::ProcessedScore> _ownScore;
        TimePoint _cacheTime;
    };

    std::unordered_map<std::string, CachedScores> _levelValidatorToScores;
    const std::vector<Database::ProcessedScore> _emptyScores;

public:
    void receivedScores(const std::string& levelValidator,
        const std::vector<Database::ProcessedScore>& scores);

    void receivedOwnScore(const std::string& levelValidator,
        const Database::ProcessedScore& score);

    void requestedScores(const std::string& levelValidator);

    [[nodiscard]] bool shouldRequestScores(
        const std::string& levelValidator) const;

    [[nodiscard]] const std::vector<Database::ProcessedScore>& getScores(
        const std::string& levelValidator) const;

    [[nodiscard]] const Database::ProcessedScore* getOwnScore(
        const std::string& levelValidator) const;
};

class MenuGame
{
private:
    //---------------------------------------
    // Classes

    Steam::steam_manager& steamManager;
    Discord::discord_manager& discordManager;
    HGAssets& assets;
    sf::Font& imagine;
    Audio& audio;
    HexagonGame& hexagonGame;
    ssvs::GameState game;
    ssvs::GameWindow& window;
    HexagonClient& hexagonClient;
    HexagonDialogBox dialogBox;
    LeaderboardCache leaderboardCache;

    Lua::LuaContext lua;
    std::vector<std::string> execScriptPackPathContext;
    const PackData* currentPack{nullptr};

    //---------------------------------------
    // Initialization

    void initAssets();
    void initInput();
    void initLua();
    void initMenus();
    void playLocally();
    std::pair<const unsigned int, const unsigned int>
    pickRandomMainMenuBackgroundStyle();

    //---------------------------------------
    // Assets

    static constexpr std::array<const char*, 3> creditsIds{
        "creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png"};
    sf::Sprite titleBar{assets.get<sf::Texture>("titleBar.png")};
    sf::Sprite creditsBar1{assets.get<sf::Texture>("creditsBar1.png")};
    sf::Sprite creditsBar2{assets.get<sf::Texture>("creditsBar2.png")};
    sf::Sprite epilepsyWarning{assets.get<sf::Texture>("epilepsyWarning.png")};


    //---------------------------------------
    // Online status bar

    sf::Sprite sOnline;
    sf::RectangleShape rsOnlineStatus{sf::Vector2f{128.f, 32.f}};
    sf::Text txtOnlineStatus{"", imagine, 24};

    void initOnlineIcons()
    {
        assets.get<sf::Texture>("onlineIcon.png").setSmooth(true);
        assets.get<sf::Texture>("onlineIconFail.png").setSmooth(true);
        sOnline.setTexture(assets.get<sf::Texture>("onlineIconFail.png"));
    }

    //---------------------------------------
    // Text Entering

    std::vector<char> enteredChars;

    [[nodiscard]] bool isEnteringText() const noexcept
    {
        return state <= States::ETLPNewBoot || state == States::ETLPNew;
    }

    //---------------------------------------
    // Cameras
    ssvs::Camera backgroundCamera{window,
        {ssvs::zeroVec2f, {Config::getSizeX() * Config::getZoomFactor(),
                              Config::getSizeY() * Config::getZoomFactor()}}};

    ssvs::Camera overlayCamera{
        window, {{Config::getWidth() / 2.f,
                     Config::getHeight() * Config::getZoomFactor() / 2.f},
                    {Config::getWidth() * Config::getZoomFactor(),
                        Config::getHeight() * Config::getZoomFactor()}}};

    bool mustRefresh{false};

    //---------------------------------------
    // Navigation

    bool wasFocusHeld{false};
    bool focusHeld{false};
    float wheelProgress{0.f};
    float touchDelay{0.f};
    States state{States::LoadingScreen};

    void leftAction();
    void rightAction();
    void upAction();
    void downAction();
    void okAction();
    void eraseAction();
    void exitAction();
    int packChangeDirection{0};
    void changePack();
    void changePackQuick(const int direction);
    void changePackAction(const int direction);

    [[nodiscard]] ssvms::Menu* getCurrentMenu() noexcept
    {
        switch(state)
        {
            case States::SMain: return &mainMenu;
            case States::MOpts: return &optionsMenu;
            case States::MOnline: return &onlineMenu;
            case States::SLPSelectBoot:
            case States::SLPSelect: return &profileSelectionMenu;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
            default: return nullptr;
#pragma GCC diagnostic pop
        }
    }

    [[nodiscard]] bool isInMenu() noexcept
    {
        return getCurrentMenu() != nullptr;
    }


    //---------------------------------------
    // State changes

    void changeStateTo(const States mState);

    //---------------------------------------
    // Update

    LevelStatus levelStatus;
    int ignoreInputs{0};
    void update(ssvu::FT mFT);
    void setIndex(int mIdx);
    void refreshCamera();
    void reloadAssets(const bool reloadEntirePack);
    void setIgnoreAllInputs(const unsigned int presses);

    //---------------------------------------
    // Drawing

    float w, h;
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

        void updateHeight()
        {
            height = hg::Utils::getFontHeight(font);
        }
    };

    MenuFont txtVersion{.font{"", imagine, 40}};
    MenuFont txtProf{.font{"", imagine, 21}};
    MenuFont txtLoadBig{.font{"", imagine, 80}};
    MenuFont txtLoadSmall{.font{"", imagine}};
    MenuFont txtMenuBig{.font{"", imagine, 80}};
    MenuFont txtMenuSmall{.font{"", imagine}};
    MenuFont txtProfile{.font{"", imagine, 35}};
    MenuFont txtRandomTip{.font{"", imagine, 38}};
    MenuFont txtInstructionsBig{.font{"", imagine, 50}};
    MenuFont txtInstructionsMedium{.font{"", imagine}};
    MenuFont txtInstructionsSmall{.font{"", imagine, 24}};
    MenuFont txtEnteringText{.font{"", imagine, 60}};
    MenuFont txtSelectionBig{.font{"", imagine, 32}};
    MenuFont txtSelectionMedium{.font{"", imagine, 24}};
    MenuFont txtSelectionLSmall{.font{"", imagine, 16}};
    MenuFont txtSelectionSmall{.font{"", imagine, 16}};
    MenuFont txtSelectionScore{.font{"", imagine, 32}};
    sf::Color menuTextColor;
    sf::Color menuQuadColor;
    sf::Color menuSelectionColor;
    sf::Color dialogBoxTextColor;
    Utils::FastVertexVectorTris menuBackgroundTris;
    Utils::FastVertexVectorQuads menuQuads;

    void draw();
    void render(sf::Drawable& mDrawable)
    {
        window.draw(mDrawable);
    }

    // Helper functions
    [[nodiscard]] float getFPSMult() const
    {
        // multiplier for FPS consistent drawing operations.
        return 200.f / window.getFPS();
    }

    void drawGraphics()
    {
        render(titleBar);
        render(creditsBar1);
        render(creditsBar2);
        render(txtVersion.font);
    }

    void drawOnlineStatus();

    void adjustMenuOffset(const bool resetMenuOffset);
    float calcMenuOffset(float& offset, const float maxOffset,
        const bool revertOffset, const bool speedUp = false);
    void calcMenuItemOffset(float& offset, bool selected);

    void createQuad(
        const sf::Color& color, float x1, float x2, float y1, float y2);
    void createQuadTrapezoid(const sf::Color& color, float x1, float x2,
        float x3, float y1, float y2, bool left);

    std::pair<int, int> getScrollbarNotches(
        const int size, const int maxSize) const;
    void drawScrollbar(const float totalHeight, const int size,
        const int notches, const float x, const float y,
        const sf::Color& color);

    void drawMainSubmenus(
        const std::vector<std::unique_ptr<ssvms::Category>>& subMenus,
        const float indent);
    void drawSubmenusSmall(
        const std::vector<std::unique_ptr<ssvms::Category>>& subMenus,
        const float indent);

    // Load menu
    const hg::HGAssets::LoadInfo& loadInfo;
    std::array<std::string_view, 2> randomTip;
    float hexagonRotation{0.f};
    void drawLoadResults();

    // Main menu
    float menuHalfHeight{0.f};
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
    float enteringTextOffset{0.f};
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

    bool isLevelFavorite{false};
    std::vector<std::string> favoriteLevelDataIds;
    LevelDrawer lvlSlct;
    LevelDrawer favSlct{
        .levelDataIds = &favoriteLevelDataIds, .isFavorites = true};
    LevelDrawer* lvlDrawer{&lvlSlct};

    void changeFavoriteLevelsToProfile();

    [[nodiscard]] bool isFavoriteLevels() const
    {
        return lvlDrawer->isFavorites;
    }

    [[nodiscard]] std::size_t getSelectablePackInfosSize() const
    {
        return isFavoriteLevels() ? 1 : assets.getSelectablePackInfos().size();
    }

    [[nodiscard]] const auto& getNthSelectablePackInfo(const std::size_t i)
    {
        return assets.getSelectablePackInfos().at(i);
    }

    int diffMultIdx{0};
    std::vector<float> diffMults;
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

    [[nodiscard]] float getLevelListHeight() const
    {
        return levelLabelHeight *
                   (focusHeld ? 1 : lvlDrawer->levelDataIds->size()) +
               slctFrameSize;
    }

    void calcScrollSpeed()
    {
        // Only speed up the animation if there are more than 16 levels.
        scrollSpeed = baseScrollSpeed *
                      std::max(lvlDrawer->levelDataIds->size() / 16.f, 1.f);
    }
    void calcLevelChangeScroll(const int dir);
    void calcPackChangeScrollFold(const float mLevelListHeight);
    void calcPackChangeScrollStretch(const float mLevelListHeight);
    void quickPackFoldStretch();
    void scrollLevelListToTargetY(ssvu::FT mFT);
    void checkWindowTopScroll(
        const float scroll, std::function<void(const float)> action)
    {
        const float target{-scroll};
        if(target <= lvlDrawer->YOffset)
        {
            return;
        }
        action(target);
    }
    void checkWindowBottomScroll(
        const float scroll, std::function<void(const float)> action)
    {
        const float target{h - scroll};
        if(target >= lvlDrawer->YOffset)
        {
            return;
        }
        action(target);
    }

    void scrollName(std::string& text, float& scroller);
    void scrollNameRightBorder(std::string& text, const std::string key,
        sf::Text& font, float& scroller, float border);
    void scrollNameRightBorder(
        std::string& text, sf::Text& font, float& scroller, const float border);

    void resetNamesScrolls()
    {
        for(int i = 0; i < static_cast<int>(Label::ScrollsSize); ++i)
        {
            namesScroll[i] = 0;
        }
    }
    void resetLevelNamesScrolls()
    {
        // Reset all scrolls except the ones relative to the pack.
        namesScroll[static_cast<int>(Label::LevelName)] = 0.f;
        for(int i = static_cast<int>(Label::MusicName);
            i < static_cast<int>(Label::ScrollsSize); ++i)
        {
            namesScroll[i] = 0.f;
        }
    }

    float getMaximumTextWidth() const
    {
        return w * 0.33f - 2.f * textToQuadBorder;
    }
    void formatLevelDescription();
    void drawLevelSelectionRightSide(
        LevelDrawer& drawer, const bool revertOffset);
    void drawLevelSelectionLeftSide(
        LevelDrawer& drawer, const bool revertOffset);

    // Text rendering
    void renderText(
        const std::string& mStr, sf::Text& mText, const sf::Vector2f& mPos)
    {
        mText.setString(mStr);
        mText.setPosition(mPos);
        render(mText);
    }
    void renderText(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const sf::Color& mColor)
    {
        mText.setFillColor(mColor);
        renderText(mStr, mText, mPos);
    }
    void renderText(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos)
    {
        mText.setCharacterSize(mSize);
        renderText(mStr, mText, mPos);
    }
    void renderText(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos,
        const sf::Color& mColor)
    {
        mText.setCharacterSize(mSize);
        mText.setFillColor(mColor);
        renderText(mStr, mText, mPos);
    }

    // Text rendering centered
    void renderTextCentered(
        const std::string& mStr, sf::Text& mText, const sf::Vector2f& mPos)
    {
        mText.setString(mStr);
        mText.setPosition(mPos.x - ssvs::getGlobalHalfWidth(mText), mPos.y);
        render(mText);
    }
    void renderTextCentered(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const sf::Color& mColor)
    {
        mText.setFillColor(mColor);
        renderTextCentered(mStr, mText, mPos);
    }
    void renderTextCentered(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos)
    {
        mText.setCharacterSize(mSize);
        renderTextCentered(mStr, mText, mPos);
    }
    void renderTextCentered(const std::string& mStr, sf::Text& mText,
        const unsigned int mSize, const sf::Vector2f& mPos,
        const sf::Color& mColor)
    {
        mText.setCharacterSize(mSize);
        mText.setFillColor(mColor);
        renderTextCentered(mStr, mText, mPos);
    }

    // Text rendering centered with an offset
    void renderTextCenteredOffset(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const float xOffset)
    {
        mText.setString(mStr);
        mText.setPosition(
            xOffset + mPos.x - ssvs::getGlobalHalfWidth(mText), mPos.y);
        render(mText);
    }

    void renderTextCenteredOffset(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const float xOffset, const sf::Color& mColor)
    {
        mText.setFillColor(mColor);
        renderTextCenteredOffset(mStr, mText, mPos, xOffset);
    }

    //---------------------------------------
    // Misc / Unused

    std::string scoresMessage;
    float exitTimer{0}, currentCreditsId{0};
    bool mustTakeScreenshot{false};
    std::string currentLeaderboard, enteredStr, leaderboardString;

    void runLuaFile(const std::string& mFileName);
    void changeResolutionTo(unsigned int mWidth, unsigned int mHeight);
    void playSoundOverride(const std::string& assetId);

    [[nodiscard]] float getWindowWidth() const noexcept
    {
        return window.getRenderWindow().getSize().x;
    }

    [[nodiscard]] float getWindowHeight() const noexcept
    {
        return window.getRenderWindow().getSize().y;
    }

    //---------------------------------------
    // Input boxes

    enum class DialogInputState
    {
        Nothing,

        Registration_EnteringUsername,
        Registration_EnteringPassword,

        Login_EnteringUsername,
        Login_EnteringPassword,

        DeleteAccount_EnteringPassword,
    };

    DialogInputState dialogInputState{DialogInputState::Nothing};

    std::string registrationUsername;
    std::string registrationPassword;
    std::string loginUsername;
    std::string loginPassword;
    std::string deleteAccountPassword;

    void showDialogBox(const std::string& msg);
    void showInputDialogBox(const std::string& msg);
    void showInputDialogBoxNice(const std::string& title,
        const std::string& inputType, const std::string& extra = "");

public:
    MenuGame(Steam::steam_manager& mSteamManager,
        Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
        Audio& mAudio, HexagonGame& mHexagonGame, ssvs::GameWindow& mGameWindow,
        HexagonClient& mHexagonClient);

    ~MenuGame();

    void init(bool mErrored);
    void init(bool mErrored, const std::string& pack, const std::string& level);
    bool loadCommandLineLevel(
        const std::string& pack, const std::string& level);

    [[nodiscard]] ssvs::GameState& getGame() noexcept
    {
        return game;
    }

    void returnToLevelSelection()
    {
        adjustLevelsOffset();
        lvlDrawer->XOffset = 0.f;
        setIgnoreAllInputs(1); // otherwise you go back to the main menu
    }

    void refreshBinds();
};

} // namespace hg
