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
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/FPSWatcher.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadata.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadataProxy.hpp"
#include "SSVOpenHexagon/Utils/Timeline2.hpp"
#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

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
#include <optional>

namespace hg
{

class MenuGame;

class HexagonGame
{
    friend MenuGame;

private:
    Steam::steam_manager& steamManager;
    Discord::discord_manager& discordManager;
    bool discordHung{false};
    bool steamHung{false};
    std::int8_t discordAttempt{1};
    std::int8_t steamAttempt{1};

    HGAssets& assets;
    const LevelData* levelData;

    ssvs::GameState game;
    ssvs::GameWindow& window;

public:
    CPlayer player;
    std::vector<CWall> walls;
    CCustomWallManager cwManager;
    float timeUntilRichPresenceUpdate = 0.f;

private:
    ssvs::Camera backgroundCamera{window,
        {ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(),
                              Config::getHeight() * Config::getZoomFactor()}}};

    ssvs::Camera overlayCamera{
        window, {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
                    sf::Vector2f(Config::getWidth(), Config::getHeight())}};

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

    sf::Text messageText{"", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(38.f / Config::getZoomFactor())};

    ssvs::VertexVector<sf::PrimitiveType::Quads> flashPolygon{4};

    sf::Sprite keyIconLeft;
    sf::Sprite keyIconRight;
    sf::Sprite keyIconFocus;
    sf::Sprite keyIconSwap;
    sf::Sprite replayIcon;

    bool firstPlay{true};
    bool restartFirstTime{true};
    bool inputFocused{false};
    bool inputSwap{false};
    bool mustTakeScreenshot{false};
    bool mustChangeSides{false};

    random_number_generator rng;
    HexagonGameStatus status;

    struct ActiveReplay
    {
        replay_file replayFile;
        replay_player replayPlayer;
        std::string replayPackName;
        std::string replayLevelName;

        ActiveReplay(const replay_file& mReplayFile)
            : replayFile{mReplayFile}, replayPlayer{replayFile._data}
        {
        }
    };

    std::optional<ActiveReplay> activeReplay;

    random_number_generator::seed_type lastSeed;
    replay_data lastReplayData;
    bool lastFirstPlay;
    double lastPlayedScore;

    std::string restartId;
    float difficultyMult{1};
    int inputImplLastMovement{0};
    int inputMovement{0};
    bool inputImplCW{false};
    bool inputImplCCW{false};
    bool inputImplBothCWCCW{false};
    std::ostringstream os;

    FPSWatcher fpsWatcher;
    sf::Text fpsText{"0", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor())};
    sf::Text timeText{"0", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(70.f / Config::getZoomFactor())};
    sf::Text text{"", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor())};
    sf::Text replayText{"", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(20.f / Config::getZoomFactor())};

    // Color of the polygon in the center.
    CapColor capColor;

    struct CExprVec2f
    {
        float x;
        float y;
    };

    Utils::LuaMetadata luaMetadata;

    std::string packId;
    std::string levelId;

    // Lua related methods
    void redefineLuaFunctions();
    void destroyMaliciousFunctions();
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
            death();
        }
    }

    // Wall creation
    void createWall(int mSide, float mThickness, const SpeedData& mSpeed,
        const SpeedData& mCurve = SpeedData{}, float mHueMod = 0);

public:
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
                      << ssvu::toStr(mError.what()) << "\n"
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
                      << ssvu::toStr(mError.what()) << "\n"
                      << std::endl;

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
    void updateText();
    void updateKeyIcons();

    // Draw methods
public:
    Utils::FastVertexVector<sf::PrimitiveType::Quads> wallQuads;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> playerTris;
    Utils::FastVertexVector<sf::PrimitiveType::Quads> capQuads;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> capTris;
    Utils::FastVertexVector<sf::PrimitiveType::Quads> wallQuads3D;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> playerTris3D;
    sf::Vector2f offset3D;

    [[nodiscard]] const sf::Vector2f& get3DOffset() noexcept
    {
        return offset3D;
    }

private:
    static constexpr float piAndHalf{ssvu::piHalf * 3.f};

    void drawSetup();
    void draw2D();
    void drawProjections();
    void draw3D();
    void drawWrapup();
    void draw();
    void drawText_TimeAndStatus(const sf::Color& offsetColor);
    void drawText_Message(const sf::Color& offsetColor);
    void drawText();
    void drawKeyIcons();

    std::function<void()> drawFunc;
    void setDrawFunc()
    {
        if(!Config::get3D())
        {
            drawFunc = [this] { draw2D(); };
            return;
        }

        if(levelData->_3DDrawingMode)
        {
            drawFunc = [this] { draw3D(); };
        }
        else
        {
            drawFunc = [this] { drawProjections(); };
        }
    }

    // Gameplay methods
    void incrementDifficulty();
    void sideChange(unsigned int mSideNumber);

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

    template <typename F>
    Utils::LuaMetadataProxy addLuaFn(const std::string& name, F&& f)
    {
        lua.writeVariable(name, std::forward<F>(f));
        return Utils::LuaMetadataProxy{f, luaMetadata, name};
    }

    void printLuaDocs()
    {
        for(std::size_t i = 0; i < luaMetadata.getNumCategories(); ++i)
        {
            std::cout << '\n' << luaMetadata.prefixHeaders.at(i) << "\n\n";

            luaMetadata.forFnEntries(
                [](const std::string& ret, const std::string& name,
                    const std::string& args, const std::string& docs) {
                    std::cout << "* **`" << ret << " " << name << "(" << args
                              << ")`**: " << docs << "\n\n";
                }, i);
        }
    }

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

    MenuGame* mgPtr;

    HexagonGame(Steam::steam_manager& mSteamManager,
        Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
        ssvs::GameWindow& mGameWindow);

    void refreshTrigger(const ssvs::Input::Trigger& trigger, const int bindID)
    {
        game.refreshTrigger(trigger, bindID);
    }

    // Gameplay methods
    void newGame(const std::string& mPackId, const std::string& mId,
        bool mFirstPlay, float mDifficultyMult, bool executeLastReplay);
    void death(bool mForce = false);

    // Other methods
    void executeEvents(ssvuj::Obj& mRoot, float mTime);
    void updateRichPresenceCallbacks();

    // Graphics-related methods
    void render(sf::Drawable& mDrawable)
    {
        window.draw(mDrawable);
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

    [[nodiscard]] HexagonGameStatus& getStatus()
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

    [[nodiscard]] float getMusicDMSyncFactor() const
    {
        return levelStatus.syncMusicToDM ? std::pow(difficultyMult, 0.12f)
                                         : 1.f;
    }

    void setMusicPitch(sf::Music& current)
    {
        current.setPitch((getMusicDMSyncFactor()) *
                         Config::getMusicSpeedMult() * levelStatus.musicPitch);
    }

    // Input
    [[nodiscard]] bool getInputFocused() const;
    [[nodiscard]] float getPlayerSpeedMult() const;
    [[nodiscard]] bool getInputSwap() const;
    [[nodiscard]] int getInputMovement() const;

    template <typename F>
    [[nodiscard]] bool anyCustomWall(F&& f)
    {
        return cwManager.anyCustomWall(std::forward<F>(f));
    }

    // Pack information
    [[nodiscard]] const std::string& getPackId() const noexcept;
    [[nodiscard]] const std::string& getPackDisambiguator() const noexcept;
    [[nodiscard]] const std::string& getPackAuthor() const noexcept;
    [[nodiscard]] const std::string& getPackName() const noexcept;
    [[nodiscard]] int getPackVersion() const noexcept;

    [[nodiscard]] bool inReplay() const noexcept;
    [[nodiscard]] bool mustReplayInput() const noexcept;
    [[nodiscard]] bool mustShowReplayUI() const noexcept;

    [[nodiscard]] float getSwapCooldown() const noexcept;
};

} // namespace hg
