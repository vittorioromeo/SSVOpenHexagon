// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
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
#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"

namespace hg
{

class MenuGame;

class HexagonGame
{
    friend MenuGame;

private:
    Steam::steam_manager& steamManager;
    Discord::discord_manager& discordManager;

    HGAssets& assets;
    const LevelData* levelData;

    ssvs::GameState game;
    ssvs::GameWindow& window;

public:
    CPlayer player;
    std::vector<CWall> walls;
    CCustomWallManager cwManager;

private:
    ssvs::Camera backgroundCamera{window,
        {ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(),
                              Config::getHeight() * Config::getZoomFactor()}}};

    ssvs::Camera overlayCamera{
        window, {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
                    sf::Vector2f(Config::getWidth(), Config::getHeight())}};

    ssvu::TimelineManager effectTimelineManager;

    Lua::LuaContext lua;

    LevelStatus levelStatus;
    MusicData musicData;
    StyleData styleData;

    ssvu::Timeline timeline;
    ssvu::Timeline eventTimeline;
    ssvu::Timeline messageTimeline;

    sf::Text messageText{"", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(38.f / Config::getZoomFactor())};

    ssvs::VertexVector<sf::PrimitiveType::Quads> flashPolygon{4};

    bool firstPlay{true};
    bool restartFirstTime{true};
    bool inputFocused{false};
    bool inputSwap{false};
    bool mustTakeScreenshot{false};
    bool mustChangeSides{false};

    HexagonGameStatus status;
    std::string restartId;
    float difficultyMult{1};
    int inputImplLastMovement;
    int inputMovement{0};
    bool inputImplCW{false};
    bool inputImplCCW{false};
    bool inputImplBothCWCCW{false};
    std::ostringstream os;

    FPSWatcher fpsWatcher;
    sf::Text text{"", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor())};

    const sf::Vector2f txt_pos{8, 8};

    // Color of the polygon in the center.
    CapColor capColor;

    struct CExprVec2f
    {
        float x;
        float y;
    };

    static constexpr std::array txt_offsets{CExprVec2f{-1, -1},
        CExprVec2f{-1, 1}, CExprVec2f{1, -1}, CExprVec2f{1, 1}};

    Utils::LuaMetadata luaMetadata;

    // LUA-related methods
    void initLua_Utils();
    void initLua_Messages();
    void initLua_MainTimeline();
    void initLua_EventTimeline();
    void initLua_LevelControl();
    void initLua_StyleControl();
    void initLua_WallCreation();
    void initLua_Steam();
    void initLua_CustomWalls();

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
        return Utils::runLuaFunction<T, TArgs...>(lua, mName, mArgs...);
    }

    template <typename T, typename... TArgs>
    void runLuaFunctionIfExists(const std::string& mName, const TArgs&... mArgs)
    {
        Utils::runLuaFunctionIfExists<T, TArgs...>(lua, mName, mArgs...);
    }

private:
    void initFlashEffect();

    // Update methods
    void update(FT mFT);
    void updateIncrement();
    void updateEvents(FT mFT);
    void updateLevel(FT mFT);
    void updatePulse(FT mFT);
    void updateBeatPulse(FT mFT);
    void updateRotation(FT mFT);
    void updateFlash(FT mFT);
    void update3D(FT mFT);
    void updateText();

    // Draw methods
    void draw();

    // Gameplay methods
    void incrementDifficulty();
    void sideChange(unsigned int mSideNumber);

    // Draw methods
    void drawText();

    // Data-related methods
    void setLevelData(const LevelData& mLevelData, bool mMusicFirstPlay);
    void playLevelMusic();
    void playLevelMusicAtTime(float mSeconds);
    void stopLevelMusic();

    // Message-related methods
    void addMessage(std::string mMessage, float mDuration, bool mSoundToggle);
    void clearMessages();

    // Level/menu loading/unloading/changing
    void checkAndSaveScore();
    void goToMenu(bool mSendScores = true);
    void changeLevel(const std::string& mId, bool mFirstTime);

    void invalidateScore();


    template <typename F>
    Utils::LuaMetadataProxy addLuaFn(const std::string& name, F&& f)
    {
        lua.writeVariable(name, std::forward<F>(f));
        return Utils::LuaMetadataProxy{f, luaMetadata, name};
    }

    void printLuaDocs()
    {
        luaMetadata.forFnEntries(
            [](const std::string& ret, const std::string& name,
                const std::string& args, const std::string& docs) {
                std::cout << "* **`" << ret << " " << name << "(" << args
                          << ")`**: " << docs << '\n';
            });
    }


public:
    Utils::FastVertexVector<sf::PrimitiveType::Quads> wallQuads;
    Utils::FastVertexVector<sf::PrimitiveType::Quads> wallDebugQuads;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> playerDebugTris;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> playerTris;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> capTris;
    // Utils::FastVertexVector<sf::PrimitiveType::Triangles> capTrisBorder;
    Utils::FastVertexVector<sf::PrimitiveType::Quads> wallQuads3D;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> playerTris3D;

    MenuGame* mgPtr;

    HexagonGame(Steam::steam_manager& mSteamManager,
        Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
        ssvs::GameWindow& mGameWindow);

    // Gameplay methods
    void newGame(
        const std::string& mId, bool mFirstPlay, float mDifficultyMult);
    void death(bool mForce = false);

    // Other methods
    void executeEvents(ssvuj::Obj& mRoot, float mTime);

    // Graphics-related methods
    void render(sf::Drawable& mDrawable)
    {
        window.draw(mDrawable);
    }

    // Player methods
    void playerSwap(bool mSoundTog)
    {
        player.swap(*this, mSoundTog);
    }


    // Setters
    void setSides(unsigned int mSides);

    // Getters
    [[nodiscard]] ssvs::GameState& getGame() noexcept
    {
        return game;
    }

    [[nodiscard]] const sf::Vector2f& getFieldPos() const noexcept
    {
        return levelStatus.fieldPos;
    }

    [[nodiscard]] float getRadius() const noexcept
    {
        return status.radius;
    }

    [[nodiscard]] const StyleData& getStyleData() const noexcept
    {
        return styleData;
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

    [[nodiscard]] float getMusicDMSyncFactor() const
    {
        return Config::getMusicSpeedDMSync() ? std::pow(difficultyMult, 0.12f)
                                             : 1.f;
    }

    // Input
    [[nodiscard]] bool getInputFocused() const
    {
        return inputFocused;
    }

    [[nodiscard]] bool getInputSwap() const;

    [[nodiscard]] int getInputMovement() const
    {
        return inputMovement;
    }

    template <typename F>
    [[nodiscard]] bool anyCustomWall(F&& f)
    {
        return cwManager.anyCustomWall(std::forward<F>(f));
    }
};

} // namespace hg
