// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_HEXAGONGAME
#define HG_HEXAGONGAME

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Core/HGStatus.h"
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Data/MusicData.h"
#include "SSVOpenHexagon/Data/StyleData.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Global/Factory.h"
#include "SSVOpenHexagon/Global/Typedefs.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Utils/FPSWatcher.h"

namespace hg
{
	class MenuGame;

	class HexagonGame
	{
		friend class MenuGame;

		private:
			HGAssets& assets;
			const LevelData* levelData;

			ssvs::GameState game;
			ssvs::GameWindow& window;
			sses::Manager manager;
			ssvs::Camera backgroundCamera{window, {{0, 0}, {Config::getWidth() * Config::getZoomFactor(), Config::getHeight() * Config::getZoomFactor()}}};
			ssvs::Camera overlayCamera{window, {{Config::getWidth() / 2.f, Config::getHeight() / 2.f}, ssvs::Vec2f(Config::getWidth(), Config::getHeight())}};
			std::vector<ssvs::Camera> depthCameras;
			ssvu::TimelineManager effectTimelineManager;
			Factory factory{*this, manager, {0, 0}};
			Lua::LuaContext	lua;
			LevelStatus levelStatus;
			MusicData musicData;
			StyleData styleData;
			ssvu::Timeline timeline, eventTimeline, messageTimeline;
			sf::Text messageText{"", assets.get<sf::Font>("imagine.ttf"), static_cast<unsigned int>(38.f / Config::getZoomFactor())};
			sf::VertexArray flashPolygon{sf::PrimitiveType::Quads, 4};
			bool firstPlay{true}, restartFirstTime{true}, inputFocused{false}, inputSwap{false}, mustTakeScreenshot{false}, mustChangeSides{false};
			HexagonGameStatus status;
			std::string restartId;
			float difficultyMult{1};
			int inputMovement{0};

			FPSWatcher fpsWatcher;
			sf::Text text{"", assets.get<sf::Font>("imagine.ttf"), static_cast<unsigned int>(25.f / Config::getZoomFactor())};

			// LUA-related methods
			void initLua();
			inline void runLuaFile(const std::string& mFileName) { try { Utils::runLuaFile(lua, mFileName); } catch(...) { death(); } }
			template<typename R, typename... Args> inline R runLuaFunction(const std::string& mName, const Args&... mArgs) { return Utils::runLuaFunction<R, Args...>(lua, mName, mArgs...); }

			void initFlashEffect();

			// Update methods
			void update(float mFT);
			void updateTimeStop(float mFT);
			void updateIncrement();
			void updateEvents(float mFT);
			void updateLevel(float mFT);
			void updatePulse(float mFT);
			void updateBeatPulse(float mFT);
			void updateRotation(float mFT);
			void updateFlash(float mFT);
			void update3D(float mFT);

			// Draw methods
			void draw();

			// Gameplay methods
			void incrementDifficulty();
			void sideChange(int mSideNumber);

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
			MenuGame* mgPtr;

			HexagonGame(HGAssets& mAssets, ssvs::GameWindow& mGameWindow);

			// Gameplay methods
			void newGame(const std::string& mId, bool mFirstPlay, float mDifficultyMult);
			void death(bool mForce = false);

			// Other methods
			void executeEvents(ssvuj::Obj& mRoot, float mTime);

			// Graphics-related methods
			void render(sf::Drawable&);

			// Setters
			void setSides(unsigned int mSides);

			// Getters
			inline ssvs::GameState& getGame()						{ return game; }
			inline float getRadius() const							{ return status.radius; }
			inline const sf::Color& getColor(int mIdx) const		{ return styleData.getColor(mIdx); }
			inline float getSpeedMultDM() const						{ return levelStatus.speedMult * (pow(difficultyMult, 0.65f)); }
			inline float getDelayMultDM() const						{ return levelStatus.delayMult / (pow(difficultyMult, 0.10f)); }
			inline float getRotationSpeed() const					{ return levelStatus.rotationSpeed; }
			inline unsigned int getSides() const					{ return levelStatus.sides; }
			inline float getWallSkewLeft() const					{ return levelStatus.wallSkewLeft; }
			inline float getWallSkewRight() const					{ return levelStatus.wallSkewRight; }
			inline float getWallAngleLeft() const					{ return levelStatus.wallAngleLeft; }
			inline float getWallAngleRight() const					{ return levelStatus.wallAngleRight; }
			inline float get3DEffectMult() const					{ return levelStatus._3dEffectMultiplier; }
			inline HexagonGameStatus& getStatus()					{ return status; }
			inline LevelStatus& getLevelStatus()					{ return levelStatus; }
			inline HGAssets& getAssets()							{ return assets; }
			sf::Color getColorMain() const;
			inline float getMusicDMSyncFactor()						{ return Config::getMusicSpeedDMSync() ? pow(difficultyMult, 0.12f) : 1.f; }

			// Input
			inline bool getInputFocused() const	{ return inputFocused; }
			inline bool getInputSwap() const	{ return inputSwap; }
			inline int getInputMovement() const	{ return inputMovement; }
	};
}

#endif
