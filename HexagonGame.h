// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_HEXAGONGAME
#define HG_HEXAGONGAME

#include <map>
#include <vector>
#include <queue>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <json/json.h>
#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "HGStatus.h"
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/EventData.h"
#include "Data/StyleData.h"
#include "Global/Config.h"
#include "Global/Factory.h"
#pragma GCC system_header
#include <SSVLuaWrapper.h>

namespace hg
{
	class MenuGame;

	class HexagonGame
	{
		private:
			ssvs::GameState game;
			ssvs::GameWindow& window;
			sses::Manager manager;
			ssvs::Camera backgroundCamera{window, {{0, 0}, {getWidth() * getZoomFactor(), getHeight() * getZoomFactor()}}};
			ssvs::Camera overlayCamera{window, {{getWidth() / 2.f, getHeight() / 2.f}, sf::Vector2f(getWidth(), getHeight())}};
			std::vector<ssvs::Camera> depthCameras;
			ssvs::TimelineManager effectTimelineManager;
			Factory factory{*this, manager, {0, 0}};
			Lua::LuaContext	lua;
			LevelData levelData;
			MusicData musicData;
			StyleData styleData;
			sf::Music* musicPtr{nullptr};
			ssvs::Timeline timeline, messageTimeline;
			sf::Text* messageTextPtr{nullptr};
			std::vector<EventData*> eventPtrs;
			std::queue<EventData*> eventPtrQueue;
			sf::VertexArray flashPolygon{sf::PrimitiveType::Quads, 4};
			bool firstPlay{true}, restartFirstTime{true};
			HexagonGameStatus status;
			std::string restartId{""};
			float difficultyMult{1};
			int inputMovement{0};
			bool inputFocused{false};

			// LUA-related methods
			void initLua();
			void runLuaFile(std::string mFileName);
			template<typename R, typename... Args> R runLuaFunction(const std::string& variableName, const Args&... args)
			{
				try { return lua.callLuaFunction<R>(variableName, std::make_tuple(args...)); }
				catch(std::runtime_error &error)
				{
					std::cout << variableName << std::endl << "LUA runtime error: " << std::endl << ssvs::Utils::toStr(error.what()) << std::endl << std::endl; 
				}

				return R();
			}

			void initFlashEffect();

			// Update methods
			void update(float mFrameTime);
			void updateTimeStop(float mFrameTime);
			void updateIncrement();
			void updateEvents(float mFrameTime);
			void updateLevel(float mFrameTime);
			void updatePulse(float mFrameTime);
			void updateBeatPulse(float mFrameTime);
			void updateRotation(float mFrameTime);
			void updateFlash(float mFrameTime);
			void update3D(float mFrameTime);

			// Draw methods
			void draw();

			// Gameplay methods
			void incrementDifficulty();
			void sideChange(int mSideNumber);

			// Draw methods
			void drawText();

			// Data-related methods
			void setLevelData(LevelData mLevelSettings, bool mMusicFirstPlay);
			void playLevelMusic();
			void stopLevelMusic();

			// Message-related methods
			void addMessage(std::string mMessage, float mDuration);
			void clearMessage();

			// Level/menu loading/unloading/changing
			void checkAndSaveScore();
			void goToMenu();
			void changeLevel(std::string mId, bool mFirstTime);

		public:
			MenuGame* mgPtr;

			HexagonGame(ssvs::GameWindow& mGameWindow);

			// Gameplay methods
			void newGame(std::string mId, bool mFirstPlay, float mDifficultyMult);
			void death();

			// Other methods
			void executeEvents(Json::Value& mRoot, float mTime);

			// Graphics-related methods
			void render(sf::Drawable&);

			// Properties
			ssvs::GameState& getGame();
			float getRadius();
			sf::Color getColorMain();
			sf::Color getColor(int mIndex);
			float getSpeedMultiplier();
			float getDelayMultiplier();
			float getRotationSpeed();
			unsigned int getSides();
			void setSpeedMultiplier(float mSpeedMultiplier);
			void setDelayMultiplier(float mDelayMultiplier);
			void setRotationSpeed(float mRotationSpeed);
			void setSides(unsigned int mSides);
			float getWallSkewLeft();
			float getWallSkewRight();
			float getWallAngleLeft();
			float getWallAngleRight();
			float get3DEffectMult();
			HexagonGameStatus& getStatus();

			// Input
			bool getInputFocused();
			int getInputMovement();
	};
}
#endif
