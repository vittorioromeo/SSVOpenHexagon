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
	HexagonGame::HexagonGame(GameWindow& mGameWindow) :
		window(mGameWindow)
	{				
		pm = new PatternManager(this);

		recreateTextures();

		game.addUpdateFunc(	 [&](float frameTime) { update(frameTime); });
		game.addDrawFunc(	 [&](){ gameTexture.clear(Color::Black); }, -2);

		if(!getNoBackground())
			game.addDrawFunc([&](){ drawBackground(); }, 			   -1);

		game.addDrawFunc(	 [&](){ manager.draw(); }, 					0);
		game.addDrawFunc(	 [&](){ gameTexture.display(); }, 			1);
		game.addDrawFunc(	 [&](){ drawOnWindow(gameSprite); }, 		2);
		game.addDrawFunc(	 [&](){ drawText(); }, 				3);
	}
	HexagonGame::~HexagonGame() { delete pm; }

	void HexagonGame::recreateTextures()
	{
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(window.getWidth() / 2, window.getHeight() / 2);
	}

	void HexagonGame::newGame(string mId, bool mFirstPlay)
	{
		clearMessages();

		setLevelData(getLevelData(mId), mFirstPlay);

		stopAllSounds();
		playSound("play");
		
		pm->resetAdj();

		stopLevelMusic();
		playLevelMusic();

		rotationDirection = getRnd(0, 100) > 50 ? true : false;

		scripts.clear();

		timeStop = 0;
		randomSideChangesEnabled = true;
		incrementEnabled = true;
		maxPulse = 85;
		minPulse = 75;
		pulseSpeedBackwards = 1;
		pulseSpeed = 1;

		hasDied = false;
		mustRestart = false;
		currentTime = 0;
		incrementTime = 0;
		setSides(levelData.getSides());
		radius = minPulse;

		manager.clear();
		createPlayer(manager, this, centerPos);

		scriptsTimeline = Timeline{};
		messagesTimeline = Timeline{};
		timeline = Timeline{};
	}
	void HexagonGame::death()
	{
		playSound("death");
		playSound("game_over");

		hasDied = true;
		stopLevelMusic();
		checkAndSaveScore();
	}

	void HexagonGame::drawOnTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void HexagonGame::drawOnWindow(Drawable &mDrawable) { window.renderWindow.draw(mDrawable); }

	void HexagonGame::update(float mFrameTime)
	{
		if(!hasDied)
		{
			manager.update(mFrameTime);

			updateLevelEvents(mFrameTime);

			if(timeStop <= 0)
			{
				currentTime += mFrameTime / 60.0f;
				incrementTime += mFrameTime / 60.0f;
			}
			else timeStop -= 1 * mFrameTime;

			updateIncrement();			
			updateLevel(mFrameTime);
			updateRadius(mFrameTime);
			if(!getBlackAndWhite()) styleData.update(mFrameTime);
		}
		else setRotationSpeed(getRotationSpeed() / 1.001f);

		updateKeys();
		if(!getNoRotation()) updateRotation(mFrameTime);

		if(mustRestart) newGame(levelData.getId(), false);
	}
	inline void HexagonGame::updateIncrement()
	{
		if(!incrementEnabled) return;
		if(incrementTime < levelData.getIncrementTime()) return;

		incrementTime = 0;
		incrementDifficulty();
	}
	inline void HexagonGame::updateLevelEvents(float mFrameTime)
	{
		scriptsTimeline.update(mFrameTime);
		if(scriptsTimeline.isFinished()) clearAndResetTimeline(scriptsTimeline);

		for(ScriptData& pattern : scripts) pattern.update(mFrameTime);

		if(!getScripting()) return;

		messagesTimeline.update(mFrameTime);
		if(messagesTimeline.isFinished()) clearAndResetTimeline(messagesTimeline);

		executeEvents(levelData.getRoot()["events"], currentTime);
	}
	inline void HexagonGame::updateLevel(float mFrameTime)
	{
		timeline.update(mFrameTime);

		if(timeline.isFinished())
		{
			timeline.clear();
			levelData.getRandomPattern()(pm);
			timeline.reset();
		}
	}
	inline void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = getRotationSpeed() * 10 * mFrameTime;
		if(rotationDirection) nextRotation *= -1;
		if(fastSpin > 0)
		{
			nextRotation += (getSmootherStep(0, 85, fastSpin) / 3.5f) * getSign(nextRotation) * mFrameTime * 17.0f;
			fastSpin -= mFrameTime;
		}

		gameSprite.rotate(nextRotation);
	}
	inline void HexagonGame::updateRadius(float mFrameTime)
	{
		radiusTimer += pulseSpeed * mFrameTime;
		if(radiusTimer >= 25)
		{
			radiusTimer = 0;
			radius = maxPulse;
		}

		if(radius > minPulse) radius -= pulseSpeedBackwards * mFrameTime;
	}
	inline void HexagonGame::updateKeys()
	{
		if(isKeyPressed(Keyboard::R)) mustRestart = true;
		else if(isKeyPressed(Keyboard::Escape))	goToMenu();
	}

	void HexagonGame::drawText()
	{
		ostringstream s;
		s << "time: " << toStr(currentTime).substr(0, 5) << endl;
		if(hasDied) s << "press r to restart" << endl;

		vector<Vector2f> offsets{{-1,-1},{-1,1},{1,-1},{1,1}};

		Text timeText(s.str(), getFont("imagine"), 25 / getZoomFactor());
		timeText.setPosition(15, 3);
		timeText.setColor(getColorMain());
		for(auto offset : offsets)
		{
			Text timeOffsetText(s.str(), getFont("imagine"), timeText.getCharacterSize());
			timeOffsetText.setPosition(timeText.getPosition() + offset);
			timeOffsetText.setColor(getColorB());
			drawOnWindow(timeOffsetText);
		}
		drawOnWindow(timeText);

		for (Text* textPtr : messageTextPtrs)
		{
			for(auto offset : offsets)
			{
				Text textPtrOffset{textPtr->getString(), getFont("imagine"), textPtr->getCharacterSize()};
				textPtrOffset.setPosition(textPtr->getPosition() + offset);
				textPtrOffset.setOrigin(textPtrOffset.getGlobalBounds().width / 2, 0);
				textPtrOffset.setColor(getColorB());
				drawOnWindow(textPtrOffset);
			}
			
			textPtr->setColor(getColorMain());
			drawOnWindow(*textPtr);
		}
	}
	void HexagonGame::drawBackground()
	{
		float div{360.f / getSides() * 1.0001f};
		float distance{1500};

		VertexArray vertices{PrimitiveType::Triangles, 3};

		for(int i{0}; i < getSides(); i++)
		{
			float angle { div * i };
			Color currentColor { styleData.getCurrentA() };

			if (i % 2 == 0)
			{
				currentColor = styleData.getCurrentB();
				if (i == getSides() - 1) currentColor = getColorDarkened(currentColor, 1.4f);
			}

			Vector2f p1 = getOrbit(centerPos, angle + div * 0.5f, distance);
			Vector2f p2 = getOrbit(centerPos, angle - div * 0.5f, distance);

			vertices.append(Vertex{centerPos, currentColor});
			vertices.append(Vertex{p1, currentColor});
			vertices.append(Vertex{p2, currentColor});
		}

		gameTexture.draw(vertices);
	}

	void HexagonGame::playLevelMusic() { if(!getNoMusic()) musicData.playRandomSegment(musicPtr); }
	void HexagonGame::stopLevelMusic() { if(!getNoMusic()) if(musicPtr != nullptr) musicPtr->stop(); }

	void HexagonGame::incrementDifficulty()
	{
		playSound("level_up");

		setSpeedMultiplier(getSpeedMultiplier() + levelData.getSpeedIncrement());
		setRotationSpeed(getRotationSpeed() + levelData.getRotationSpeedIncrement());
		setDelayMultiplier(getDelayMultiplier() + levelData.getDelayIncrement());
		rotationDirection = !rotationDirection;
		fastSpin = levelData.getFastSpin();
		
		if(randomSideChangesEnabled) timeline.add(new Do([&]{ randomSideChange(); }));
	}
	void HexagonGame::randomSideChange()
	{
		if(manager.getComponentPtrsById("wall").size() > 0)
		{
			timeline.add(new Wait(10));
			timeline.add(new Do([&]{ randomSideChange(); }));
			return;
		}
		setSides(getRnd(levelData.getSidesMin(), levelData.getSidesMax() + 1));
	}

	void HexagonGame::checkAndSaveScore()
	{
		if(getScore(levelData.getId()) < currentTime) setScore(levelData.getId(), currentTime);
		saveCurrentProfile();
	}
	void HexagonGame::goToMenu()
	{
		stopAllSounds();
		playSound("beep");

		checkAndSaveScore();
		window.setGame(&mgPtr->getGame());
		mgPtr->init();
	}
	void HexagonGame::changeLevel(string mId)
	{
		checkAndSaveScore();
		newGame(mId, true);
	}
	void HexagonGame::addMessage(string mMessage, float mDuration)
	{
		Text* text = new Text(mMessage, getFont("imagine"), 40 / getZoomFactor());
		text->setPosition(Vector2f(getWidth() / 2, getHeight() / 6));
		text->setOrigin(text->getGlobalBounds().width / 2, 0);

		messagesTimeline.add(new Do{ [&, text, mMessage]{ messageTextPtrs.push_back(text); }});
		messagesTimeline.add(new Wait{mDuration});
		messagesTimeline.add(new Do{ [=]{ messageTextPtrs.clear(); delete text; }});
	}
	void HexagonGame::clearMessages()
	{
		for (Text* textPtr : messageTextPtrs) delete textPtr;
		messageTextPtrs.clear();
	}

	void HexagonGame::setLevelData(LevelData mLevelSettings, bool mMusicFirstPlay)
	{
		levelData = mLevelSettings;
		styleData = getStyleData(levelData.getStyleId());
		musicData = getMusicData(levelData.getMusicId());
		musicData.setFirstPlay(mMusicFirstPlay);
	}

	bool HexagonGame::isKeyPressed(Keyboard::Key mKey) 	{ return window.isKeyPressed(mKey); }

	Game& HexagonGame::getGame()						{ return game; }
	float HexagonGame::getRadius() 						{ return radius; }
	Color HexagonGame::getColorMain() 					{ return getBlackAndWhite() ? Color::White : styleData.getCurrentMain(); }
	Color HexagonGame::getColorB() 						{ return getBlackAndWhite() ? Color::Black : styleData.getCurrentB(); }

	float HexagonGame::getSpeedMultiplier() { return levelData.getSpeedMultiplier(); }
	float HexagonGame::getDelayMultiplier() { return levelData.getDelayMultiplier(); }
	float HexagonGame::getRotationSpeed() 	{ return levelData.getRotationSpeed(); }
	int HexagonGame::getSides() 			{ return levelData.getSides(); }

	void HexagonGame::setSpeedMultiplier(float mSpeedMultiplier) { levelData.setSpeedMultiplier(mSpeedMultiplier); }
	void HexagonGame::setDelayMultiplier(float mDelayMultiplier) { levelData.setDelayMultiplier(mDelayMultiplier); }
	void HexagonGame::setRotationSpeed(float mRotationSpeed) 	 { levelData.setRotationSpeed(mRotationSpeed); }
	void HexagonGame::setSides(int mSides)
	{
		playSound("beep");
		if (mSides < 3) mSides = 3;
		levelData.setValueInt("sides", mSides);
	}

	void HexagonGame::executeEvents(Json::Value& mRoot, float mTime)
	{
		for (Json::Value& eventRoot : mRoot)
		{
			if(eventRoot["time"].asFloat() >  mTime) continue;
			if(eventRoot["executed"].asBool()) continue;
			eventRoot["executed"] = true;
			string type{eventRoot["type"].asString()};
			float duration{eventRoot["duration"].asFloat()};
			string valueName{eventRoot["value_name"].asString()};
			float value{eventRoot["value"].asFloat()};
			string message{eventRoot["message"].asString()};
			string id{eventRoot["id"].asString()};

			if 		(type == "level_change")			changeLevel(id);
			else if (type == "menu") 					goToMenu();
			else if (type == "message_add")				{ if(getShowMessages()) addMessage(message, duration); }
			else if (type == "message_clear") 			clearMessages();
			else if (type == "time_stop")				timeStop = duration;
			else if (type == "timeline_wait") 			timeline.add(new Wait(duration));
			else if (type == "timeline_clear") 			clearAndResetTimeline(timeline);
			else if (type == "value_float_set") 		levelData.setValueFloat(valueName, value);
			else if (type == "value_float_add") 		levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) + value);
			else if (type == "value_float_subtract") 	levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) - value);
			else if (type == "value_float_multiply") 	levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) * value);
			else if (type == "value_float_divide") 		levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) / value);
			else if (type == "value_int_set") 			levelData.setValueInt(valueName, value);
			else if (type == "value_int_add") 			levelData.setValueInt(valueName, levelData.getValueFloat(valueName) + value);
			else if (type == "value_int_subtract")		levelData.setValueInt(valueName, levelData.getValueFloat(valueName) - value);
			else if (type == "value_int_multiply") 		levelData.setValueInt(valueName, levelData.getValueFloat(valueName) * value);
			else if (type == "value_int_divide") 		levelData.setValueInt(valueName, levelData.getValueFloat(valueName) / value);
			else if (type == "music_set")				{ if(getChangeMusic()) { stopLevelMusic(); musicData = getMusicData(id); musicData.playRandomSegment(musicPtr); } }
			else if (type == "music_set_segment")		{ if(getChangeMusic()) { stopLevelMusic(); musicData = getMusicData(id); musicData.playSegment(musicPtr, eventRoot["segment_index"].asInt()); } }
			else if (type == "music_set_seconds")		{ if(getChangeMusic()) { stopLevelMusic(); musicData = getMusicData(id); musicData.playSeconds(musicPtr, eventRoot["seconds"].asInt()); } }
			else if (type == "style_set")				{ if(getChangeStyles()) styleData = getStyleData(id); }
			else if (type == "side_changing_stop")		randomSideChangesEnabled = false;
			else if (type == "side_changing_start")		randomSideChangesEnabled = true;
			else if (type == "increment_stop")			incrementEnabled = false;
			else if (type == "increment_start")			incrementEnabled = true;
			else if (type == "pulse_max_set")			maxPulse = value;
			else if (type == "pulse_min_set")			minPulse = value;
			else if (type == "pulse_speed_set")			pulseSpeed = value;
			else if (type == "pulse_speed_b_set")		pulseSpeedBackwards = value;
			else if (type == "script_exec")				scripts.push_back(getScriptData(id, this));
			else if (type == "script_queue")			queueScript(getScriptData(id, this));
			else										log("unknown script command: " + type);
		}
	}

	void HexagonGame::queueScript(ScriptData mScript)
	{
		// TO IMPLEMENT
	}
}
