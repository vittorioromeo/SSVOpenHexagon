// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_HEXAGONGAME
#define HG_HEXAGONGAME

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Factory.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/FPSWatcher.hpp"

namespace hg
{
    class MenuGame;

    class HexagonGame
    {
        friend MenuGame;

    private:
        HGAssets& assets;
        const LevelData* levelData;

        ssvs::GameState game;
        ssvs::GameWindow& window;
        sses::Manager manager;
    public:
        std::vector<CWall> walls;

    private:
        ssvs::Camera backgroundCamera{
            window, {ssvs::zeroVec2f,
                        {Config::getWidth() * Config::getZoomFactor(),
                            Config::getHeight() * Config::getZoomFactor()}}};
        ssvs::Camera overlayCamera{
            window, {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
                        Vec2f(Config::getWidth(), Config::getHeight())}};
        ssvu::TimelineManager effectTimelineManager;
        Factory factory{*this, manager, ssvs::zeroVec2f};
        Lua::LuaContext lua;
        LevelStatus levelStatus;
        MusicData musicData;
        StyleData styleData;
        ssvu::Timeline timeline, eventTimeline, messageTimeline;
        sf::Text messageText{"", assets.get<sf::Font>("imagine.ttf"),
            ssvu::toNum<unsigned int>(38.f / Config::getZoomFactor())};
        ssvs::VertexVector<sf::PrimitiveType::Quads> flashPolygon{4};
        bool firstPlay{true}, restartFirstTime{true}, inputFocused{false},
            inputSwap{false}, mustTakeScreenshot{false}, mustChangeSides{false};
        HexagonGameStatus status;
        std::string restartId;
        float difficultyMult{1};
        int inputImplLastMovement, inputMovement{0};
        bool inputImplCW{false}, inputImplCCW{false}, inputImplBothCWCCW{false};
        std::ostringstream os;

        FPSWatcher fpsWatcher;
        sf::Text text{"", assets.get<sf::Font>("imagine.ttf"),
            ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor())};

        const Vec2f txt_pos{8, 8};
        const std::vector<Vec2f> txt_offsets{
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

        // LUA-related methods
        void initLua();
        inline void runLuaFile(const std::string& mFileName)
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

    public:
        template <typename T, typename... TArgs>
        inline T runLuaFunction(const std::string& mName, const TArgs&... mArgs)
        {
            return Utils::runLuaFunction<T, TArgs...>(lua, mName, mArgs...);
        }
        template <typename T, typename... TArgs>
        inline void runLuaFunctionIfExists(
            const std::string& mName, const TArgs&... mArgs)
        {
            Utils::runLuaFunctionIfExists<T, TArgs...>(lua, mName, mArgs...);
        }

    private:
        void initFlashEffect();

        // Update methods
        void update(FT mFT);
        void updateTimeStop(FT mFT);
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
        void stopLevelMusic();

        // Message-related methods
        void addMessage(const std::string& mMessage, float mDuration);

        // Level/menu loading/unloading/changing
        void checkAndSaveScore();
        void goToMenu(bool mSendScores = true);
        void changeLevel(const std::string& mId, bool mFirstTime);

        void invalidateScore();

    public:
        ssvs::VertexVector<sf::PrimitiveType::Quads> wallQuads;
        ssvs::VertexVector<sf::PrimitiveType::Triangles> playerTris;

        MenuGame* mgPtr;

        HexagonGame(HGAssets& mAssets, ssvs::GameWindow& mGameWindow);

        // Gameplay methods
        void newGame(
            const std::string& mId, bool mFirstPlay, float mDifficultyMult);
        void death(bool mForce = false);

        // Other methods
        void executeEvents(ssvuj::Obj& mRoot, float mTime);

        // Graphics-related methods
        inline void render(sf::Drawable& mDrawable)
        {
            window.draw(mDrawable);
        }

        // Setters
        void setSides(unsigned int mSides);

        // Getters
        inline ssvs::GameState& getGame() noexcept
        {
            return game;
        }
        inline float getRadius() const noexcept
        {
            return status.radius;
        }
        inline const sf::Color& getColor(int mIdx) const noexcept
        {
            return styleData.getColor(mIdx);
        }
        inline float getSpeedMultDM() const noexcept
        {
            return levelStatus.speedMult * (std::pow(difficultyMult, 0.65f));
        }
        inline float getDelayMultDM() const noexcept
        {
            return levelStatus.delayMult / (std::pow(difficultyMult, 0.10f));
        }
        inline float getRotationSpeed() const noexcept
        {
            return levelStatus.rotationSpeed;
        }
        inline unsigned int getSides() const noexcept
        {
            return levelStatus.sides;
        }
        inline float getWallSkewLeft() const noexcept
        {
            return levelStatus.wallSkewLeft;
        }
        inline float getWallSkewRight() const noexcept
        {
            return levelStatus.wallSkewRight;
        }
        inline float getWallAngleLeft() const noexcept
        {
            return levelStatus.wallAngleLeft;
        }
        inline float getWallAngleRight() const noexcept
        {
            return levelStatus.wallAngleRight;
        }
        inline float get3DEffectMult() const noexcept
        {
            return levelStatus._3dEffectMultiplier;
        }
        inline HexagonGameStatus& getStatus()
        {
            return status;
        }
        inline LevelStatus& getLevelStatus()
        {
            return levelStatus;
        }
        inline HGAssets& getAssets()
        {
            return assets;
        }
        sf::Color getColorMain() const;
        inline float getMusicDMSyncFactor()
        {
            return Config::getMusicSpeedDMSync()
                       ? std::pow(difficultyMult, 0.12f)
                       : 1.f;
        }

        // Input
        inline bool getInputFocused() const
        {
            return inputFocused;
        }
        inline bool getInputSwap() const
        {
            return inputSwap;
        }
        inline int getInputMovement() const
        {
            return inputMovement;
        }
    };
}

#endif
