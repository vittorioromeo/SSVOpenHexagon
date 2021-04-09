// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/RandomNumberGenerator.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/LevelStatus.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadata.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadataProxy.hpp"
#include "SSVOpenHexagon/Utils/Timeline2.hpp"
#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

#include <SSVStart/GameSystem/GameSystem.hpp>
#include <SSVStart/Camera/Camera.hpp>
#include <SSVStart/VertexVector/VertexVector.hpp>
#include <SSVStart/Utils/Vector2.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVUtils/Timeline/Timeline.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <cstdint>
#include <sstream>
#include <unordered_set>
#include <functional>
#include <optional>
#include <functional>

namespace hg {

class Audio;
class HGAssets;
class MenuGame;
class HexagonClient;

class HexagonGame
{
    friend MenuGame;

private:
    Steam::steam_manager* steamManager;
    Discord::discord_manager* discordManager;
    bool discordHung{false};
    bool steamHung{false};
    std::int8_t discordAttempt{1};
    std::int8_t steamAttempt{1};

    HGAssets& assets;
    sf::Font& font;

    Audio& audio;

    const LevelData* levelData;

    ssvs::GameState game;
    ssvs::GameWindow* window;

    HexagonClient* hexagonClient;

    // IMGUI Lua Console
    sf::Clock ilcDeltaClock;
    std::vector<std::string> ilcCmdLog;
    std::vector<std::string> ilcHistory;
    int ilcHistoryPos{-1};
    std::string ilcCmdBuffer;
    std::string ilcTrackBuffer;
    bool ilcShowConsole{false};
    bool ilcShowConsoleNext{false};
    std::vector<std::string> ilcLuaTracked;
    std::vector<std::string> ilcLuaTrackedNames;
    std::vector<std::string> ilcLuaTrackedResults;
    bool debugPause{false};

    std::vector<std::string> execScriptPackPathContext;

public:
    int ilcTextEditCallback(ImGuiInputTextCallbackData* data);

public:
    CPlayer player;
    std::vector<CWall> walls;
    CCustomWallManager cwManager;
    float timeUntilRichPresenceUpdate = 0.f;

private:
    std::optional<ssvs::Camera> backgroundCamera;
    std::optional<ssvs::Camera> overlayCamera;

    ssvu::TimelineManager effectTimelineManager;

    const sf::Vector2f centerPos{ssvs::zeroVec2f};

    Lua::LuaContext lua;
    std::unordered_set<std::string> calledDeprecatedFunctions;

    LevelStatus levelStatus;
    MusicData musicData;
    StyleData styleData;

    Utils::timeline2 timeline;
    Utils::timeline2_runner timelineRunner;

    Utils::timeline2 eventTimeline;
    Utils::timeline2_runner eventTimelineRunner;

    Utils::timeline2 messageTimeline;
    Utils::timeline2_runner messageTimelineRunner;



    sf::Text messageText{
        "", font, ssvu::toNum<unsigned int>(38.f / Config::getZoomFactor())};

    sf::Text pbText{
        "", font, ssvu::toNum<unsigned int>(65.f / Config::getZoomFactor())};

    ssvs::VertexVector<sf::PrimitiveType::Quads> flashPolygon{4};

    struct Particle
    {
        sf::Sprite sprite;
        sf::Vector2f velocity;
        float angularVelocity;
    };

    std::vector<Particle> particles;
    bool mustSpawnPBParticles{false};
    float nextPBParticleSpawn{0.f};
    float pbTextGrowth{0.f};

    sf::Sprite keyIconLeft;
    sf::Sprite keyIconRight;
    sf::Sprite keyIconFocus;
    sf::Sprite keyIconSwap;
    sf::Sprite replayIcon;

    sf::RectangleShape levelInfoRectangle;
    sf::Text levelInfoTextLevel{"", font};
    sf::Text levelInfoTextPack{"", font};
    sf::Text levelInfoTextAuthor{"", font};
    sf::Text levelInfoTextBy{"", font};
    sf::Text levelInfoTextDM{"", font};

    bool firstPlay{true};
    bool restartFirstTime{true};
    bool inputFocused{false};
    bool inputSwap{false};
    bool mustTakeScreenshot{false};
    bool mustChangeSides{false};
    bool mustStart{false};

    random_number_generator rng;
    HexagonGameStatus status;

    float deathInputIgnore{0.f};

    struct ActiveReplay
    {
        replay_file replayFile;
        replay_player replayPlayer;
        std::string replayPackName;
        std::string replayLevelName;

        ActiveReplay(const replay_file& mReplayFile)
            : replayFile{mReplayFile}, replayPlayer{replayFile._data}
        {}
    };

    std::optional<ActiveReplay> activeReplay;

    random_number_generator::seed_type lastSeed{};
    replay_data lastReplayData{};
    bool lastFirstPlay{};
    double lastPlayedScore{};

    std::string restartId;
    float difficultyMult{1};
    int inputImplLastMovement{0};
    int inputMovement{0};
    bool inputImplCW{false};
    bool inputImplCCW{false};
    std::ostringstream os;

    sf::Text fpsText{
        "0", font, ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor())};
    sf::Text timeText{
        "0", font, ssvu::toNum<unsigned int>(70.f / Config::getZoomFactor())};
    sf::Text text{
        "", font, ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor())};
    sf::Text replayText{
        "", font, ssvu::toNum<unsigned int>(20.f / Config::getZoomFactor())};

    // Color of the polygon in the center.
    CapColor capColor;

    std::string packId;
    std::string levelId;

    // Lua related methods
    void initLua_Utils();
    void initLua_AudioControl();
    void initLua_MainTimeline();
    void initLua_EventTimeline();
    void initLua_LevelControl();
    void initLua_StyleControl();
    void initLua_WallCreation();
    void initLua_Steam();
    void initLua_CustomWalls();
    void initLua_Deprecated();

    void initLua();
    void runLuaFile(const std::string& mFileName)
    {
        try
        {
            Utils::runLuaFile(lua, mFileName);
        }
        catch(...)
        {
            if(!Config::getDebug())
            {
                goToMenu(false /* mSendScores */, true /* mError */);
            }
        }
    }

    // Wall creation
    void createWall(int mSide, float mThickness, const SpeedData& mSpeed,
        const SpeedData& mCurve = SpeedData{}, float mHueMod = 0);

public:
    // For testing
    std::function<void(const replay_file&)> onReplayCreated;

    // For testing
    void setMustStart(const bool x)
    {
        mustStart = x;
    }

    void initLuaAndPrintDocs();

    template <typename T, typename... TArgs>
    T runLuaFunction(const std::string& mName, const TArgs&... mArgs)
    {
        try
        {
            return Utils::runLuaFunction<T, TArgs...>(lua, mName, mArgs...);
        }
        catch(const std::runtime_error& mError)
        {
            std::cout << "[runLuaFunction] Runtime error on \"" << mName
                      << "\" with level \"" << levelData->name << "\": \n"
                      << mError.what() << '\n'
                      << std::endl;

            if(!Config::getDebug())
            {
                goToMenu(false /* mSendScores */, true /* mError */);
            }
        }
        catch(...)
        {
            std::cout << "[runLuaFunction] Unknown runtime error on \"" << mName
                      << "\" with level \"" << levelData->name << "\": \n"
                      << '\n'
                      << std::endl;

            if(!Config::getDebug())
            {
                goToMenu(false /* mSendScores */, true /* mError */);
            }
        }

        return T();
    }

    template <typename T, typename... TArgs>
    auto runLuaFunctionIfExists(const std::string& mName, const TArgs&... mArgs)
    {
        try
        {
            return Utils::runLuaFunctionIfExists<T, TArgs...>(
                lua, mName, mArgs...);
        }
        catch(std::runtime_error& mError)
        {
            std::cout << "[runLuaFunctionIfExists] Runtime error on \"" << mName
                      << "\" with level \"" << levelData->name << "\": \n"
                      << mError.what() << '\n'
                      << std::endl;

            if(!Config::getDebug())
            {
                goToMenu(false /* mSendScores */, true /* mError */);
            }
        }
        catch(...)
        {
            std::cout << "[runLuaFunction] Unknown error on \"" << mName
                      << "\" with level \"" << levelData->name << std::endl;

            if(!Config::getDebug())
            {
                goToMenu(false /* mSendScores */, true /* mError */);
            }
        }

        return decltype(
            Utils::runLuaFunctionIfExists<T, TArgs...>(lua, mName, mArgs...)){};
    }

    void raiseWarning(
        const std::string& mFunctionName, const std::string& mAdditionalInfo);
    void setLastReplay(const replay_file& mReplayFile);

private:
    void start();

    void initKeyIcons();
    void initFlashEffect();

    // Update methods
    void update(ssvu::FT mFT);
    void updateInput();
    void updateInput_UpdateJoystickControls();
    void updateInput_UpdateTouchControls();
    void updateInput_ResolveInputImplToInputMovement();
    void updateInput_RecordCurrentInputToLastReplayData();
    void updateWalls(ssvu::FT mFT);
    void updateIncrement();
    void updateEvents(ssvu::FT mFT);
    void updateLevel(ssvu::FT mFT);
    void updateCustomWalls(ssvu::FT mFT);
    void updatePulse(ssvu::FT mFT);
    void updateBeatPulse(ssvu::FT mFT);
    void updateRotation(ssvu::FT mFT);
    void updateFlash(ssvu::FT mFT);
    void update3D(ssvu::FT mFT);
    void updateText(ssvu::FT mFT);
    void updateKeyIcons();
    void updateLevelInfo();
    void updateParticles(ssvu::FT mFT);

    // Post update methods
    void postUpdate();
    void postUpdate_ImguiLuaConsole();

    // Draw methods
    void draw();

    // Gameplay methods
    void incrementDifficulty();
    void sideChange(unsigned int mSideNumber);

    // Draw methods
    void drawText_TimeAndStatus(const sf::Color& offsetColor);
    void drawText_Message(const sf::Color& offsetColor);
    void drawText_PersonalBest(const sf::Color& offsetColor);
    void drawText();
    void drawKeyIcons();
    void drawLevelInfo();
    void drawParticles();
    void drawImguiLuaConsole();

    // Data-related methods
    void setLevelData(const LevelData& mLevelData, bool mMusicFirstPlay);
    void playLevelMusic();
    void playLevelMusicAtTime(float mSeconds);
    void stopLevelMusic();

    // Message-related methods
    void addMessage(std::string mMessage, double mDuration, bool mSoundToggle);
    void clearMessages();

    enum class CheckSaveScoreResult
    {
        Ineligible,
        Invalid,
        Local_NewBest,
        Local_NoNewBest,
        Online_LessThan8Secs,
        Online_ConnectionError,
        Online_VersionMismatch,
        Online_Sent
    };

    // Level/menu loading/unloading/changing
    CheckSaveScoreResult checkAndSaveScore();
    void goToMenu(bool mSendScores = true, bool mError = false);

    void invalidateScore(const std::string& mReason);

    [[nodiscard]] bool imguiLuaConsoleHasInput();

    template <typename T>
    auto makeLuaAccessor(T& obj, const std::string& prefix);

    static void nameFormat(std::string& name)
    {
        name[0] = std::toupper(name[0]);
    }

    [[nodiscard]] static std::string diffFormat(float diff)
    {
        char buf[255];
        std::snprintf(buf, sizeof(buf), "%g", diff);
        return buf;
    }

    [[nodiscard]] static std::string timeFormat(float time)
    {
        char buf[255];
        std::snprintf(buf, sizeof(buf), "%.3f", time);
        return buf;
    }

public:
    void performPlayerSwap(const bool mPlaySound);
    void performPlayerKill();

public:
    Utils::FastVertexVectorTris backgroundTris;
    Utils::FastVertexVectorQuads wallQuads;
    Utils::FastVertexVectorTris playerTris;
    Utils::FastVertexVectorTris capTris;
    Utils::FastVertexVectorQuads wallQuads3D;
    Utils::FastVertexVectorTris playerTris3D;

    MenuGame* mgPtr;

    HexagonGame(Steam::steam_manager* mSteamManager,
        Discord::discord_manager* mDiscordManager, HGAssets& mAssets,
        Audio& mAudio, ssvs::GameWindow* mGameWindow,
        HexagonClient* mHexagonClient);

    ~HexagonGame();

    void refreshTrigger(const ssvs::Input::Trigger& trigger, const int bindID)
    {
        game.refreshTrigger(trigger, bindID);
    }

    // Gameplay methods
    void newGame(const std::string& mPackId, const std::string& mId,
        bool mFirstPlay, float mDifficultyMult, bool executeLastReplay);

    void death(bool mForce = false);

    [[nodiscard]] double executeGameUntilDeath();

    [[nodiscard]] double runReplayUntilDeathAndGetScore(
        const replay_file& mReplayFile);

    // Other methods
    void executeEvents(ssvuj::Obj& mRoot, float mTime);
    void updateRichPresenceCallbacks();

    void playSoundOverride(const std::string& mId);
    void playSoundAbort(const std::string& mId);
    void playPackSoundOverride(
        const std::string& mPackId, const std::string& mId);

    // Graphics-related methods
    void render(sf::Drawable& mDrawable,
        const sf::RenderStates& mStates = sf::RenderStates::Default)
    {
        if(window == nullptr)
        {
            ssvu::lo("hg::HexagonGame::render")
                << "Attempted to render without a game window\n";

            return;
        }

        window->draw(mDrawable, mStates);
    }

    // Setters
    void setSides(unsigned int mSides);

    // Getters
    [[nodiscard]] ssvs::GameState& getGame() noexcept
    {
        return game;
    }

    [[nodiscard]] float getRadius() const noexcept
    {
        return status.radius;
    }

    [[nodiscard]] const sf::Color& getColor(int mIdx) const noexcept
    {
        return styleData.getColor(mIdx);
    }

    [[nodiscard]] float getSpeedMultDM() const noexcept
    {
        const auto res =
            levelStatus.speedMult * (std::pow(difficultyMult, 0.65f));

        if(!levelStatus.hasSpeedMaxLimit())
        {
            return res;
        }

        return (res < levelStatus.speedMax) ? res : levelStatus.speedMax;
    }

    [[nodiscard]] float getDelayMultDM() const noexcept
    {
        const auto res =
            levelStatus.delayMult / (std::pow(difficultyMult, 0.10f));

        if(!levelStatus.hasDelayMaxLimit())
        {
            return res;
        }

        return (res < levelStatus.delayMax) ? res : levelStatus.delayMax;
    }

    [[nodiscard]] float getRotationSpeed() const noexcept
    {
        return levelStatus.rotationSpeed;
    }

    [[nodiscard]] unsigned int getSides() const noexcept
    {
        return levelStatus.sides;
    }

    [[nodiscard]] float getWallSkewLeft() const noexcept
    {
        return levelStatus.wallSkewLeft;
    }

    [[nodiscard]] float getWallSkewRight() const noexcept
    {
        return levelStatus.wallSkewRight;
    }

    [[nodiscard]] float getWallAngleLeft() const noexcept
    {
        return levelStatus.wallAngleLeft;
    }

    [[nodiscard]] float getWallAngleRight() const noexcept
    {
        return levelStatus.wallAngleRight;
    }

    [[nodiscard]] float get3DEffectMult() const noexcept
    {
        return levelStatus._3dEffectMultiplier;
    }

    [[nodiscard]] HexagonGameStatus& getStatus() noexcept
    {
        return status;
    }

    [[nodiscard]] const HexagonGameStatus& getStatus() const noexcept
    {
        return status;
    }

    [[nodiscard]] LevelStatus& getLevelStatus()
    {
        return levelStatus;
    }

    [[nodiscard]] HGAssets& getAssets()
    {
        return assets;
    }

    [[nodiscard]] sf::Color getColorMain() const;
    [[nodiscard]] sf::Color getColorPlayer() const;
    [[nodiscard]] sf::Color getColorText() const;

    [[nodiscard]] float getMusicDMSyncFactor() const;

    void refreshMusicPitch();

    // Input
    [[nodiscard]] bool getInputFocused() const;
    [[nodiscard]] float getPlayerSpeedMult() const;
    [[nodiscard]] bool getInputSwap() const;
    [[nodiscard]] int getInputMovement() const;

    // Pack information
    [[nodiscard]] const PackData& getPackData() const noexcept;
    [[nodiscard]] const std::string& getPackId() const noexcept;
    [[nodiscard]] const std::string& getPackDisambiguator() const noexcept;
    [[nodiscard]] const std::string& getPackAuthor() const noexcept;
    [[nodiscard]] const std::string& getPackName() const noexcept;
    [[nodiscard]] int getPackVersion() const noexcept;

    [[nodiscard]] bool inReplay() const noexcept;
    [[nodiscard]] bool mustReplayInput() const noexcept;
    [[nodiscard]] bool mustShowReplayUI() const noexcept;

    [[nodiscard]] float getSwapCooldown() const noexcept;

    [[gnu::always_inline, nodiscard]] const sf::Vector2f&
    getCenterPos() const noexcept
    {
        return centerPos;
    }
};

} // namespace hg
