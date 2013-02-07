// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HEXAGONGAME_H_
#define HEXAGONGAME_H_

#include <map>
#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <json/json.h>
#include <json/reader.h>
#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/EventData.h"
#include "Data/StyleData.h"
#include "Utils/Utils.h"
#pragma GCC system_header
#include <SSVLuaWrapper.h>

using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;

namespace hg
{
	constexpr float baseSpeed{5};
	constexpr float baseThickness{40};

	class MenuGame;
	class PatternManager;

	class HexagonGame
	{
		friend class PatternManager;

		private:
			GameState game;
			GameWindow& window;
			Manager manager;
			RenderTexture gameTexture;
			Sprite gameSprite;
			Lua::LuaContext	lua;
			Vector2f centerPos{0,0};
			LevelData levelData;
			MusicData musicData;
			StyleData styleData;
			Music* musicPtr{nullptr};
			Timeline timeline;
			Timeline messageTimeline;
			Text* messageTextPtr{nullptr};
			vector<EventData*> eventPtrs;
			queue<EventData*> eventPtrQueue;
			VertexArray flashPolygon{PrimitiveType::Quads, 4};
			bool firstPlay{true};

			// New game parameters
			float currentTime{0};
			float incrementTime{0};
			float timeStop{1};
			bool randomSideChangesEnabled{true};
			bool incrementEnabled{true};

			float pulse					{75};
			float pulseDirection		{1};
			float pulseDelay			{0};
			float pulseDelayHalf		{0};

			float beatPulse				{0};
			float beatPulseDelay		{0};

			float flashEffect			{0};

			float radius{75};
			float fastSpin{0};
			bool hasDied{false};
			bool mustRestart{false};
			string restartId{""};
			bool restartFirstTime{true};
			float difficultyMult{1};

			// LUA-related methods
			void initLua();
			void runLuaFile(string mFileName);
			template<typename R, typename... Args> R runLuaFunction(const std::string& variableName, const Args&... args)
			{
				try { return lua.callLuaFunction<R>(variableName, std::make_tuple(args...)); }
				catch(runtime_error &error) { cout << variableName << endl << "LUA runtime error: " << endl << toStr(error.what()) << endl << endl; }

				return R();
			}

			// Update methods
			void update(float);
			inline void updateTimeStop(float mFrameTime);
			inline void updateIncrement();
			inline void updateEvents(float mFrameTime);
			inline void updateLevel(float mFrameTime);	
			inline void updatePulse(float mFrameTime);
			inline void updateBeatPulse(float mFrameTime);
			inline void updateKeys();
			inline void updateRotation(float mFrameTime);
			inline void updateFlash(float mFrameTime);

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
			void addMessage(string mMessage, float mDuration);
			void clearMessage();

			// Level/menu loading/unloading/changing
			void checkAndSaveScore();
			void goToMenu();
			void changeLevel(string mId, bool mFirstTime);

			// Wall spawn
			void wall(int mSide, float mThickness);
			void wallAdj(int mSide, float mThickness, float mSpeedAdj);

		public:
			MenuGame* mgPtr;

			HexagonGame(GameWindow& mGameWindow);

			// Gameplay methods
			void newGame(string mId, bool mFirstPlay, float mDifficultyMult);
			void death();

			// Other methods
			bool isKeyPressed(sf::Keyboard::Key mKey);
			bool isButtonPressed(sf::Mouse::Button mButton);
			void executeEvents(Json::Value& mRoot, float mTime);

			// Graphics-related methods
			void recreateTextures();
			void drawOnTexture(sf::Drawable&);
			void drawOnWindow(sf::Drawable&);

			// Properties
			GameState& getGame();
			float getRadius();
			Color getColorMain();
			Color getColor(int mIndex);
			float getSpeedMultiplier();
			float getDelayMultiplier();
			float getRotationSpeed();
			int getSides();
			void setSpeedMultiplier(float mSpeedMultiplier);
			void setDelayMultiplier(float mDelayMultiplier);
			void setRotationSpeed(float mRotationSpeed);
			void setSides(int mSides);
			float getWallSkewLeft();
			float getWallSkewRight();
			float getWallAngleLeft();
			float getWallAngleRight();
			float get3DEffectMult();
	};
}
#endif /* HEXAGONGAME_H_ */
