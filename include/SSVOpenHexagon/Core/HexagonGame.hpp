// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_HEXAGONGAME
#define HG_HEXAGONGAME

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
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
		friend class MenuGame;

		private:
			HGAssets& assets;
			const LevelData* levelData;

			ssvs::GameState game;
			ssvs::GameWindow& window;
			sses::Manager manager;
			ssvs::Camera backgroundCamera{window, {ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(), Config::getHeight() * Config::getZoomFactor()}}};
			ssvs::Camera overlayCamera{window, {{Config::getWidth() / 2.f, Config::getHeight() / 2.f}, Vec2f(Config::getWidth(), Config::getHeight())}};
			std::vector<ssvs::Camera> depthCameras;
			ssvu::TimelineManager effectTimelineManager;
			Factory factory{*this, manager, ssvs::zeroVec2f};
			Lua::LuaContext	lua;
			LevelStatus levelStatus;
			MusicData musicData;
			StyleData styleData;
			ssvu::Timeline timeline, eventTimeline, messageTimeline;
			sf::Text messageText{"", assets.get<sf::Font>("imagine.ttf"), static_cast<unsigned int>(38.f / Config::getZoomFactor())};
			ssvs::VertexVector<sf::PrimitiveType::Quads> flashPolygon{4};
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
			ssvs::VertexVector<sf::PrimitiveType::Quads> wallQuads;
			ssvs::VertexVector<sf::PrimitiveType::Triangles> playerTris;

			MenuGame* mgPtr;

			HexagonGame(HGAssets& mAssets, ssvs::GameWindow& mGameWindow);

			// Gameplay methods
			void newGame(const std::string& mId, bool mFirstPlay, float mDifficultyMult);
			void death(bool mForce = false);

			// Other methods
			void executeEvents(ssvuj::Obj& mRoot, float mTime);

			// Graphics-related methods
			inline void render(sf::Drawable& mDrawable) { window.draw(mDrawable); }

			// Setters
			void setSides(unsigned int mSides);

			// Getters
			inline ssvs::GameState& getGame()					{ return game; }
			inline float getRadius() const						{ return status.radius; }
			inline const sf::Color& getColor(int mIdx) const	{ return styleData.getColor(mIdx); }
			inline float getSpeedMultDM() const					{ return levelStatus.speedMult * (pow(difficultyMult, 0.65f)); }
			inline float getDelayMultDM() const					{ return levelStatus.delayMult / (pow(difficultyMult, 0.10f)); }
			inline float getRotationSpeed() const				{ return levelStatus.rotationSpeed; }
			inline unsigned int getSides() const				{ return levelStatus.sides; }
			inline float getWallSkewLeft() const				{ return levelStatus.wallSkewLeft; }
			inline float getWallSkewRight() const				{ return levelStatus.wallSkewRight; }
			inline float getWallAngleLeft() const				{ return levelStatus.wallAngleLeft; }
			inline float getWallAngleRight() const				{ return levelStatus.wallAngleRight; }
			inline float get3DEffectMult() const				{ return levelStatus._3dEffectMultiplier; }
			inline HexagonGameStatus& getStatus()				{ return status; }
			inline LevelStatus& getLevelStatus()				{ return levelStatus; }
			inline HGAssets& getAssets()						{ return assets; }
			sf::Color getColorMain() const;
			inline float getMusicDMSyncFactor()					{ return Config::getMusicSpeedDMSync() ? pow(difficultyMult, 0.12f) : 1.f; }

			// Input
			inline bool getInputFocused() const	{ return inputFocused; }
			inline bool getInputSwap() const	{ return inputSwap; }
			inline int getInputMovement() const	{ return inputMovement; }
	};
}

#endif
