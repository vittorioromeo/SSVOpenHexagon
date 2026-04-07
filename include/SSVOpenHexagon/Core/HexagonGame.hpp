// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Core/CustomTimelineManager.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/RandomNumberGeneratorTypes.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Data/CapColor.hpp"
#include "SSVOpenHexagon/Data/LevelStatus.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/GameSystem/GameState.hpp"
#include "SSVOpenHexagon/GameSystem/GameWindow.hpp"
#include "SSVOpenHexagon/Utils/CameraView.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/Timeline2.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <SFML/Base/UniquePtr.hpp>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>

struct ImGuiInputTextCallbackData;

namespace Json
{
class Value;
}

namespace ssvuj
{
using Obj = Json::Value;
}

namespace ssvs::Input
{
class Trigger;
}

namespace hg
{

class Audio;
class HGAssets;
class HexagonClient;
struct LevelData;
struct SpeedData;
struct PackData;
class random_number_generator;

namespace Steam
{
class steam_manager;
}

namespace Discord
{
class discord_manager;
}

class HexagonGame
{
private:
    struct TextUI
    {
        sf::Font& font;
        sf::Font& fontBold;

        sf::Text messageText;
        sf::Text pbText;

        sf::Text levelInfoTextLevel;
        sf::Text levelInfoTextPack;
        sf::Text levelInfoTextAuthor;
        sf::Text levelInfoTextBy;
        sf::Text levelInfoTextDM;

        sf::Text fpsText;
        sf::Text timeText;
        sf::Text text;
        sf::Text replayText;

        TextUI(HGAssets& mAssets);
    };

    sf::base::Optional<sf::Texture> nullTexture;

    Steam::steam_manager*     steamManager;
    Discord::discord_manager* discordManager;
    bool                      discordHung{false};
    bool                      steamHung{false};
    sf::base::I8              discordAttempt{1};
    sf::base::I8              steamAttempt{1};

    HGAssets&                  assets;
    sf::base::Optional<TextUI> textUI;

    Audio* audio;

    const LevelData* levelData;

    ssvs::GameState   game;
    ssvs::GameWindow* window;

    HexagonClient* hexagonClient;

    sf::base::Optional<sf::ImGuiContext> imguiCtx;

    // IMGUI Lua Console
    sf::Clock                          ilcDeltaClock;
    sf::base::Vector<sf::base::String> ilcCmdLog;
    sf::base::Vector<sf::base::String> ilcHistory;
    int                                ilcHistoryPos{-1};
    std::string                        ilcCmdBuffer;
    std::string                        ilcTrackBuffer;
    bool                               ilcShowConsole{false};
    bool                               ilcShowConsoleNext{false};
    sf::base::Vector<sf::base::String> ilcLuaTracked;
    sf::base::Vector<sf::base::String> ilcLuaTrackedNames;
    sf::base::Vector<sf::base::String> ilcLuaTrackedResults;
    bool                               debugPause{false};

    sf::base::Vector<sf::base::String> execScriptPackPathContext;

public:
    int ilcTextEditCallback(ImGuiInputTextCallbackData* data);

public:
    CPlayer                 player;
    sf::base::Vector<CWall> walls;
    CCustomWallManager      cwManager;
    float                   timeUntilRichPresenceUpdate = 0.f;

private:
    sf::base::Optional<sf::View> backgroundCamera;
    sf::base::Optional<sf::View> overlayCamera;
    Utils::ViewTransform         backgroundCameraTransform;
    Utils::ViewTransform         overlayCameraTransform;

    struct PreShakeCenters
    {
        sf::Vec2f background;
        sf::Vec2f overlay;
    };

    sf::base::Optional<PreShakeCenters> preShakeCenters;

    const sf::Vec2f centerPos{0.f, 0.f};

    Lua::LuaContext                      lua;
    std::unordered_set<sf::base::String> calledDeprecatedFunctions;

    LevelStatus levelStatus;
    MusicData   musicData;
    StyleData   styleData;

    Utils::timeline2        timeline;
    Utils::timeline2_runner timelineRunner;

    Utils::timeline2        eventTimeline;
    Utils::timeline2_runner eventTimelineRunner;

    Utils::timeline2        messageTimeline;
    Utils::timeline2_runner messageTimelineRunner;

    CustomTimelineManager _customTimelineManager;


    Utils::FastVertexVectorTris flashPolygon;

    struct Particle
    {
        sf::Sprite sprite;
        sf::Vec2f  velocity;
        float      angularVelocity;
    };

    struct TrailParticle
    {
        sf::Sprite sprite;
        float      angle;
    };

    struct SwapParticle
    {
        sf::Sprite sprite;
        sf::Vec2f  velocity;
    };

    sf::Texture* txStarParticle;
    sf::Texture* txSmallCircle;

    sf::base::Vector<Particle>      particles;
    sf::base::Vector<TrailParticle> trailParticles;
    sf::base::Vector<SwapParticle>  swapParticles;
    bool                            mustSpawnPBParticles{false};

    struct SwapParticleSpawnInfo
    {
        bool      ready;
        sf::Vec2f position;
        float     angle;
    };

    sf::base::Optional<SwapParticleSpawnInfo> swapParticlesSpawnInfo;
    float                                     nextPBParticleSpawn{0.f};
    float                                     pbTextGrowth{0.f};

    sf::Texture* txKeyIconLeft;
    sf::Texture* txKeyIconRight;
    sf::Texture* txKeyIconFocus;
    sf::Texture* txKeyIconSwap;
    sf::Texture* txReplayIcon;

    sf::Sprite keyIconLeft;
    sf::Sprite keyIconRight;
    sf::Sprite keyIconFocus;
    sf::Sprite keyIconSwap;
    sf::Sprite replayIcon;

    sf::RectangleShape levelInfoRectangle;

    bool firstPlay{true};
    bool restartFirstTime{true};
    bool inputFocused{false};
    bool inputSwap{false};
    bool mustTakeScreenshot{false};
    bool mustChangeSides{false};
    bool mustStart{false};

    sf::base::UniquePtr<random_number_generator> rng;
    HexagonGameStatus                            status;

    float deathInputIgnore{0.f};

    struct ActiveReplay
    {
        replay_file      replayFile;
        replay_player    replayPlayer;
        sf::base::String replayPackName;
        sf::base::String replayLevelName;

        explicit ActiveReplay(const replay_file& mReplayFile);
    };

    sf::base::Optional<ActiveReplay> activeReplay;

    random_number_generator_seed_type lastSeed{};
    replay_data                       lastReplayData{};
    bool                              lastFirstPlay{};
    double                            lastPlayedScore{};

    sf::base::String restartId;
    float            difficultyMult{1};
    int              inputImplLastMovement{0};
    int              inputMovement{0};
    bool             inputImplCW{false};
    bool             inputImplCCW{false};
    bool             playerNowReadyToSwap{false};

    sf::OutStringStream os;


    // Color of the polygon in the center.
    CapColor capColor;

    sf::base::String packId;
    sf::base::String levelId;

    // Lua related methods
    void initLua_Utils();
    void initLua_AudioControl();
    void initLua_MainTimeline();
    void initLua_EventTimeline();
    void initLua_CustomTimelines();
    void initLua_LevelControl();
    void initLua_StyleControl();
    void initLua_WallCreation();
    void initLua_Steam();
    void initLua_CustomWalls();
    void initLua_Deprecated();

    void initLua();
    void runLuaFile(const sf::base::String& mFileName);

    // Wall creation
    void createWall(int mSide, float mThickness, const SpeedData& mSpeed, const SpeedData& mCurve, float mHueMod);

public:
    // ------------------------------------------------------------------------
    // Testing-related utilities
    std::function<void(const replay_file&)> onDeathReplayCreated;

    void setMustStart(const bool x);

    bool executeRandomInputs{false};
    bool alwaysSpinRight{false};

    // ------------------------------------------------------------------------
    // Lua stuff

    void initLuaAndPrintDocs();

    void luaExceptionLippincottHandler(std::string_view mName);

    template <typename T, typename... TArgs>
    auto runLuaFunctionIfExists(std::string_view mName, const TArgs&... mArgs)
    try
    {
        return Utils::runLuaFunctionIfExists<T, TArgs...>(lua, mName, mArgs...);
    } catch (...)
    {
        luaExceptionLippincottHandler(mName);
        return decltype(Utils::runLuaFunctionIfExists<T, TArgs...>(lua, mName, mArgs...)){};
    }

    template <typename... TArgs>
    void runVoidLuaFunctionIfExists(std::string_view mName, const TArgs&... mArgs)
    {
        (void)runLuaFunctionIfExists<void>(mName, mArgs...);
    }

    void raiseWarning(const sf::base::String& mFunctionName, const sf::base::String& mAdditionalInfo);

    void setLastReplay(const replay_file& mReplayFile);

private:
    void start();

    void initKeyIcons();
    void initFlashEffect(int r, int g, int b);

    // Fast-forward
    sf::base::Optional<double> fastForwardTarget;
    void                       fastForwardTo(const double target);

    // Advance by ticks
    sf::base::Optional<int> advanceTickCount;
    void                    advanceByTicks(const int nTicks);

    // Update methods
    void update(float mFT, const float timescale);
    void updateInput();
    void updateInput_UpdateJoystickControls();
    void updateInput_UpdateTouchControls();
    void updateInput_ResolveInputImplToInputMovement();
    void updateInput_RecordCurrentInputToLastReplayData();
    void updateWalls(float mFT);
    void updateIncrement();
    void updateEvents(float mFT);
    void updateLevel(float mFT);
    void updateCustomTimelines();
    void updateCustomWalls(float mFT);
    void updatePulse(float mFT);
    void refreshPulse();
    void updateBeatPulse(float mFT);
    void refreshBeatPulse();
    void updateRotation(float mFT);
    void updateCameraShake(float mFT);
    void updateFlash(float mFT);
    void updatePulse3D(float mFT);
    void updateText(float mFT);
    void updateKeyIcons();
    void updateLevelInfo();
    void updateParticles(float mFT);
    void updateTrailParticles(float mFT);
    void updateSwapParticles(float mFT);

    // Post update methods
    void postUpdate();
    void postUpdate_ImguiLuaConsole();

    // Draw methods
    void draw();

    // Gameplay methods
    void incrementDifficulty();
    void sideChange(unsigned int mSideNumber);

    // Draw methods
    void drawText_TimeAndStatus(const sf::Color& offsetColor, const sf::RenderStates& mStates);
    void drawText_Message(const sf::Color& offsetColor, const sf::RenderStates& mStates);
    void drawText_PersonalBest(const sf::Color& offsetColor, const sf::RenderStates& mStates);
    void drawText(const sf::RenderStates& mStates);
    void drawKeyIcons();
    void drawLevelInfo(const sf::RenderStates& mStates);
    void drawParticles();
    void drawTrailParticles();
    void drawSwapParticles();
    void drawImguiLuaConsole();

    // Data-related methods
    void setLevelData(const LevelData& mLevelData, bool mMusicFirstPlay);
    void playLevelMusic();
    void playLevelMusicAtTime(float mSeconds);
    void stopLevelMusic();

    // Message-related methods
    void addMessage(sf::base::String mMessage, double mDuration, bool mSoundToggle);
    void clearMessages();

    enum class CheckScore
    {
        NoLocalProfile,
        Ineligible,
        Invalid,
        Valid,
    };

    // Level/menu loading/unloading/changing
    [[nodiscard]] bool shouldSaveScore();
    void               goToMenu(bool mSendScores = true, bool mError = false);

    void invalidateScore(const sf::base::String& mReason);

    [[nodiscard]] bool imguiLuaConsoleHasInput();

    template <typename T>
    auto makeLuaAccessor(T& obj, const sf::base::String& prefix);

    static void                           nameFormat(sf::base::String& name);
    [[nodiscard]] static sf::base::String diffFormat(float diff);
    [[nodiscard]] static sf::base::String timeFormat(float time);

private:
    void performPlayerSwap(const bool mPlaySound);
    void performPlayerKill();
    void saveReplay();

    Utils::FastVertexVectorTris backgroundTris;
    Utils::FastVertexVectorTris wallQuads;
    Utils::FastVertexVectorTris pivotQuads;
    Utils::FastVertexVectorTris playerTris;
    Utils::FastVertexVectorTris capTris;
    Utils::FastVertexVectorTris wallQuads3D;
    Utils::FastVertexVectorTris pivotQuads3D;
    Utils::FastVertexVectorTris playerTris3D;

public:
    std::function<void(const bool)> fnGoToMenu;

    HexagonGame(Steam::steam_manager*     mSteamManager,
                Discord::discord_manager* mDiscordManager,
                HGAssets&                 mAssets,
                Audio*                    mAudio,
                ssvs::GameWindow*         mGameWindow,
                HexagonClient*            mHexagonClient);

    ~HexagonGame();

    void refreshTrigger(const ssvs::Input::Trigger& trigger, const int bindID);

    // Gameplay methods
    void newGame(const sf::base::String& mPackId,
                 const sf::base::String& mId,
                 bool                    mFirstPlay,
                 float                   mDifficultyMult,
                 bool                    executeLastReplay);

    enum class SaveScoreIfNeededResult
    {
        NoWindow        = 0,
        ShouldNotSave   = 1,
        NotPersonalBest = 2,
        PersonalBest    = 3,
    };

    void                                  death(bool mForce = false);
    void                                  death_shakeCamera();
    void                                  death_flashEffect();
    [[nodiscard]] replay_file             death_createReplayFile();
    void                                  death_updateRichPresence();
    [[nodiscard]] SaveScoreIfNeededResult death_saveScoreIfNeeded();
    void                                  death_saveScoreIfNeededAndShowPBEffects();
    void                                  death_sendAndSaveReplay(const replay_file& rf);
    [[nodiscard]] bool death_sendReplay(const sf::base::String& levelValidator, const compressed_replay_file& crf);
    [[nodiscard]] bool death_saveReplay(sf::base::String filename, const compressed_replay_file& crf);

    struct GameExecutionResult
    {
        double playedTimeSeconds;
        double pausedTimeSeconds;
        double totalTimeSeconds;
        float  customScore;
    };

    [[nodiscard]] sf::base::Optional<GameExecutionResult> executeGameUntilDeath(const int   maxProcessingSeconds,
                                                                                const float timescale);

    [[nodiscard]] sf::base::Optional<GameExecutionResult> runReplayUntilDeathAndGetScore(
        const replay_file& mReplayFile,
        const int          maxProcessingSeconds,
        const float        timescale);

    // Other methods
    void executeEvents(ssvuj::Obj& mRoot, float mTime);
    void updateRichPresenceCallbacks();

    [[nodiscard]] bool shouldPlaySounds() const;
    [[nodiscard]] bool shouldPlayMusic() const;
    void               playSoundOverride(const sf::base::String& mId);
    void               playSoundAbort(const sf::base::String& mId);
    void               playPackSoundOverride(const sf::base::String& mPackId, const sf::base::String& mId);

    // Graphics-related methods
    template <typename TDrawable>
    void renderWithView(const sf::View& view, TDrawable&& drawable);

    template <typename TDrawable>
    void renderWithView(const sf::View& view, TDrawable&& drawable, sf::RenderStates states);

    // Setters
    void setSides(unsigned int mSides);

    // Getters
    [[nodiscard]] ssvs::GameState&         getGame() noexcept;
    [[nodiscard]] float                    getRadius() const noexcept;
    [[nodiscard]] const sf::Color&         getColor(int mIdx) const noexcept;
    [[nodiscard]] float                    getSpeedMultDM() const noexcept;
    [[nodiscard]] float                    getDelayMultDM() const noexcept;
    [[nodiscard]] float                    getRotationSpeed() const noexcept;
    [[nodiscard]] unsigned int             getSides() const noexcept;
    [[nodiscard]] float                    getWallSkewLeft() const noexcept;
    [[nodiscard]] float                    getWallSkewRight() const noexcept;
    [[nodiscard]] float                    getWallAngleLeft() const noexcept;
    [[nodiscard]] float                    getWallAngleRight() const noexcept;
    [[nodiscard]] HexagonGameStatus&       getStatus() noexcept;
    [[nodiscard]] const HexagonGameStatus& getStatus() const noexcept;
    [[nodiscard]] LevelStatus&             getLevelStatus();
    [[nodiscard]] HGAssets&                getAssets();
    [[nodiscard]] sf::Color                getColorMain() const;
    [[nodiscard]] sf::Color                getColorPlayer() const;
    [[nodiscard]] sf::Color                getColorPlayerAdjustedForSwap() const;
    [[nodiscard]] sf::Color                getColorPlayerTrail() const;
    [[nodiscard]] sf::Color                getColorText() const;
    [[nodiscard]] sf::Color                getColorCap() const;
    [[nodiscard]] sf::Color                getColorWall() const;
    [[nodiscard]] float                    getMusicDMSyncFactor() const;
    [[nodiscard]] float                    getOptionalMusicDMSyncFactor() const;

    void refreshMusicPitch();

    // Input
    [[nodiscard]] bool  getInputFocused() const;
    [[nodiscard]] float getPlayerSpeedMult() const;
    [[nodiscard]] bool  getInputSwap() const;
    [[nodiscard]] int   getInputMovement() const;

    // Pack information
    [[nodiscard]] const PackData&         getPackData() const noexcept;
    [[nodiscard]] const sf::base::String& getPackId() const noexcept;
    [[nodiscard]] const sf::base::String& getPackDisambiguator() const noexcept;
    [[nodiscard]] const sf::base::String& getPackAuthor() const noexcept;
    [[nodiscard]] const sf::base::String& getPackName() const noexcept;
    [[nodiscard]] int                     getPackVersion() const noexcept;

    [[nodiscard]] bool inReplay() const noexcept;
    [[nodiscard]] bool mustReplayInput() const noexcept;
    [[nodiscard]] bool mustShowReplayUI() const noexcept;

    [[nodiscard]] float getSwapCooldown() const noexcept;
};

} // namespace hg
