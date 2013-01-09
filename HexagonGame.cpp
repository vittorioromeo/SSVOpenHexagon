/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <SFML/Audio.hpp>
#include <SSVStart.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Data/StyleData.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Global/Factory.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "MenuGame.h"
#include "PatternManager.h"

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	HexagonGame::HexagonGame(GameWindow& mGameWindow) : window(mGameWindow), pm{new PatternManager{this}}
	{
		recreateTextures();

		game.addUpdateFunc([&](float frameTime) { update(frameTime); });
		game.addDrawFunc([&](){ gameTexture.clear(Color::Black); }, -2);

		if(!getNoBackground())
			game.addDrawFunc([&](){ styleData.drawBackground(gameTexture, centerPos, getSides()); }, -1);

		game.addDrawFunc([&](){ manager.draw(); }, 			0);
		game.addDrawFunc([&](){ gameTexture.display(); }, 	1);
		game.addDrawFunc([&](){ drawOnWindow(gameSprite); },2);
		game.addDrawFunc([&](){ drawText(); }, 				3);
	}
	HexagonGame::~HexagonGame() { delete pm; }

	void HexagonGame::newGame(string mId, bool mFirstPlay, float mDifficultyMult)
	{
		firstPlay = mFirstPlay;
		setLevelData(getLevelData(mId), mFirstPlay);
		difficultyMult = mDifficultyMult;

		// Audio cleanup
		stopAllSounds();
		playSound("play");
		stopLevelMusic();
		playLevelMusic();

		// Events cleanup
		clearMessage();

		for(auto eventPtr : eventPtrs) delete eventPtr;
		eventPtrs.clear();

		while(!eventPtrQueue.empty()) { delete eventPtrQueue.front(); eventPtrQueue.pop(); }
		eventPtrQueue = queue<EventData*>{};

		// Parameters cleanup
		currentTime = 0;
		incrementTime = 0;
		timeStop = 50;
		randomSideChangesEnabled = true;
		incrementEnabled = true;

		pulse			= 75;
		pulseDirection	= 1;
		pulseDelay		= 0;
		pulseDelayHalf	= 0;
		beatPulse		= 0;
		beatPulseDelay	= 0;
		
		radius = 75;
		fastSpin = 0;
		hasDied = false;
		mustRestart = false;
		restartId = mId;
		restartFirstTime = false;
		setSides(levelData.getSides());
		gameSprite.setRotation(0);

		// Manager cleanup
		manager.clear();
		createPlayer(manager, this, centerPos);

		// Timeline cleanup
		timeline = Timeline{};
		messageTimeline = Timeline{};

		// LUA context cleanup
		if(!mFirstPlay) runLuaFunction<void>("onUnload");
		lua = Lua::LuaContext{};
		initLua();
		runLuaFile(levelData.getValueString("lua_file"));
		runLuaFunction<void>("onLoad");

		// Random rotation direction
		if(getRnd(0, 100) > 50) setRotationSpeed(getRotationSpeed() * -1);

		// Reset zoom
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
	}
	void HexagonGame::death()
	{
		playSound("death");
		playSound("game_over");

		hasDied = true;
		stopLevelMusic();
		checkAndSaveScore();
	}

	void HexagonGame::incrementDifficulty()
	{
		playSound("level_up");

		setRotationSpeed(levelData.getRotationSpeed() + levelData.getRotationSpeedIncrement() * getSign(getRotationSpeed()));
		setRotationSpeed(levelData.getRotationSpeed() * -1);
		
		if(fastSpin < 0 && abs(getRotationSpeed()) > levelData.getValueFloat("rotation_speed_max"))
			setRotationSpeed(levelData.getValueFloat("rotation_speed_max") * getSign(getRotationSpeed()));

		fastSpin = levelData.getFastSpin();
		timeline.insert(timeline.getCurrentIndex() + 1, new Do([&]{ sideChange(getRnd(levelData.getSidesMin(), levelData.getSidesMax() + 1)); }));
	}
	void HexagonGame::sideChange(int mSideNumber)
	{
		if(manager.getComponentPtrs("wall").size() > 0)
		{
			timeline.insert(timeline.getCurrentIndex() + 1, new Do([&]{ clearAndResetTimeline(timeline); }));
			timeline.insert(timeline.getCurrentIndex() + 1, new Do([&, mSideNumber]{ sideChange(mSideNumber); }));
			timeline.insert(timeline.getCurrentIndex() + 1, new Wait(1));

			return;
		}

		runLuaFunction<void>("onIncrement");
		setSpeedMultiplier(levelData.getSpeedMultiplier() + levelData.getSpeedIncrement());
		setDelayMultiplier(levelData.getDelayMultiplier() + levelData.getDelayIncrement());

		if(randomSideChangesEnabled) setSides(mSideNumber);
	}

	void HexagonGame::checkAndSaveScore()
	{
		string validator{getScoreValidator(levelData.getId(), getPulse(), difficultyMult)};
		if(getScore(validator) < currentTime) setScore(validator, currentTime);
		saveCurrentProfile();
	}
	void HexagonGame::goToMenu()
	{
		stopAllSounds();
		playSound("beep");

		checkAndSaveScore();
		runLuaFunction<void>("onUnload");
		window.setGame(&mgPtr->getGame());
		mgPtr->init();
	}
	void HexagonGame::changeLevel(string mId, bool mFirstTime)
	{
		checkAndSaveScore();		
		newGame(mId, mFirstTime, difficultyMult);
	}
	void HexagonGame::addMessage(string mMessage, float mDuration)
	{
		Text* text = new Text(mMessage, getFont("imagine"), 40 / getZoomFactor());
		text->setPosition(Vector2f(getWidth() / 2, getHeight() / 6));
		text->setOrigin(text->getGlobalBounds().width / 2, 0);

		messageTimeline.push_back(new Do{ [&, text, mMessage]{ playSound("beep"); messageTextPtr = text; }});
		messageTimeline.push_back(new Wait{mDuration});
		messageTimeline.push_back(new Do{ [=]{ messageTextPtr = nullptr; delete text; }});
	}
	void HexagonGame::clearMessage()
	{
		if(messageTextPtr == nullptr) return;

		delete messageTextPtr;
		messageTextPtr = nullptr;
	}

	void HexagonGame::setLevelData(LevelData mLevelSettings, bool mMusicFirstPlay)
	{
		levelData = mLevelSettings;
		styleData = getStyleData(levelData.getStyleId());
		musicData = getMusicData(levelData.getMusicId());
		musicData.setFirstPlay(mMusicFirstPlay);
	}

	bool HexagonGame::isKeyPressed(Keyboard::Key mKey) 		{ return window.isKeyPressed(mKey); }
	bool HexagonGame::isButtonPressed(Mouse::Button mButton){ return window.isButtonPressed(mButton); }
	void HexagonGame::playLevelMusic() { if(!getNoMusic()) musicData.playRandomSegment(musicPtr); }
	void HexagonGame::stopLevelMusic() { if(!getNoMusic()) if(musicPtr != nullptr) musicPtr->stop(); }
}
