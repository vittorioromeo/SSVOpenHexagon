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

		recreate();

		game.addUpdateFunc(	 [&](float frameTime) { update(frameTime); });
		game.addDrawFunc(	 [&](){ gameTexture.clear(Color::Black); }, -2);

		if(!getNoBackground())
			game.addDrawFunc([&](){ drawBackground(); }, 			   -1);

		game.addDrawFunc(	 [&](){ manager.draw(); }, 					0);
		game.addDrawFunc(	 [&](){ gameTexture.display(); }, 			1);
		game.addDrawFunc(	 [&](){ drawOnWindow(gameSprite); }, 		2);
		game.addDrawFunc(	 [&](){ drawDebugText(); }, 				3);

		setLevelData(getLevelData("tutorial"));
	}
	HexagonGame::~HexagonGame() { delete pm; }

	void HexagonGame::recreate()
	{
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(window.getWidth() / 2, window.getHeight() / 2);
	}

	void HexagonGame::startFromMenu(LevelData mLevelData)
	{
		setLevelData(mLevelData);
		newGame();
	}
	void HexagonGame::newGame()
	{
		for (Text* textPtr : messageTextPtrs) delete textPtr;
		messageTextPtrs.clear();

		setLevelData(getLevelData(levelData.getId()));

		stopAllSounds();
		playSound("play");
		
		pm->resetAdj();

		stopLevelMusic();
		playLevelMusic();

		rotationDirection = getRnd(0, 100) > 50 ? true : false;

		timeStop = 0;
		hasDied = false;
		mustRestart = false;
		currentTime = 0;
		incrementTime = 0;
		setSides(levelData.getSides());
		radius = minRadius;

		manager.clear();
		createPlayer(manager, this, centerPos);

		messagesTimeline = Timeline{};
		timeline = Timeline{};
	}
	void HexagonGame::death()
	{
		playSound("death");
		playSound("game_over");
		stopLevelMusic();
		hasDied = true;
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
			if(!getBlackAndWhite()) updateColor(mFrameTime);
		}
		else setRotationSpeed(getRotationSpeed() / 1.001f);

		updateDebugKeys();
		if(!getNoRotation()) updateRotation(mFrameTime);

		if(mustRestart) newGame();
	}
	inline void HexagonGame::updateIncrement()
	{
		if(incrementTime < levelData.getIncrementTime()) return;

		incrementTime = 0;
		incrementDifficulty();
	}
	inline void HexagonGame::updateLevelEvents(float mFrameTime)
	{
		messagesTimeline.update(mFrameTime);

		if(messagesTimeline.isFinished())
		{
			messagesTimeline.clear();
			messagesTimeline.reset();
		}

		for (Json::Value& eventRoot : levelData.getEvents())
		{
			if(eventRoot["time"].asFloat() > currentTime) continue;
			if(eventRoot["executed"].asBool()) continue;

			eventRoot["executed"] = true;

			string type{eventRoot["type"].asString()};

			if(type == "event_time_stop")
			{
				float duration{eventRoot["duration"].asFloat()};
				timeStop = duration;
			}

			else if(type == "event_timeline_wait")
			{
				float duration{eventRoot["duration"].asFloat()};
				timeline.add(new Wait(duration));
			}
			else if(type == "event_timeline_clear")
			{
				timeline.clear();
				timeline.reset();
			}

			else if (type == "event_message_add")
			{
				float duration{eventRoot["duration"].asFloat()};
				string message{eventRoot["message"].asString()};
				
				Text* text = new Text{message, getFont("imagine"), 40 / getZoomFactor()};
				text->setPosition(Vector2f(getWidth() / 2, getHeight() / 6));
				text->setOrigin(text->getGlobalBounds().width / 2, 0);

				messagesTimeline.add(new Do{ [&, text, message] { messageTextPtrs.push_back(text); }});
				messagesTimeline.add(new Wait{duration});
				messagesTimeline.add(new Do{ [=] { messageTextPtrs.clear(); delete text; }});
			}

			else if (type == "event_level_change")
			{
				checkAndSaveScore();

				string id{eventRoot["id"].asString()};
				startFromMenu(getLevelData(id));
			}
			else if (type == "event_gotomenu")
			{
				goToMenu();
			}
			
			else if (type == "event_value_float_add")
			{
				string valueName{eventRoot["value_name"].asString()};
				float value{eventRoot["value"].asFloat()};
				levelData.setValueFloat(valueName, levelData.getValueFloat(valueName) + value);
			}
			else if (type == "event_value_float_set")
			{
				string valueName{eventRoot["value_name"].asString()};
				float value{eventRoot["value"].asFloat()};
				levelData.setValueFloat(valueName, value);
			}
			else if (type == "event_value_int_add")
			{
				string valueName{eventRoot["value_name"].asString()};
				int value{eventRoot["value"].asInt()};
				levelData.setValueInt(valueName, levelData.getValueInt(valueName) + value);
			}
			else if (type == "event_value_int_set")
			{
				string valueName{eventRoot["value_name"].asString()};
				int value{eventRoot["value"].asInt()};
				levelData.setValueInt(valueName, value);
			}
		}
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
	inline void HexagonGame::updateColor(float mFrameTime) { styleData.update(mFrameTime); }
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
		radiusTimer += 1.0f * mFrameTime;
		if(radiusTimer >= 25)
		{
			radiusTimer = 0;
			radius = getRnd(82, 92);
		}

		if(radius > minRadius) radius -= 0.7f * mFrameTime;
	}
	inline void HexagonGame::updateDebugKeys()
	{
		if(isKeyPressed(Keyboard::R)) mustRestart = true;
		else if(isKeyPressed(Keyboard::Escape))	goToMenu();
	}

	void HexagonGame::drawDebugText()
	{
		ostringstream s;
		s << "time: " << toStr(currentTime).substr(0, 5) << endl;
		if(hasDied) s << "press r to restart" << endl;

		vector<Vector2f> offsets{{-1,-1},{-1,1},{1,-1},{1,1}};

		for(auto offset : offsets)
		{
			Text t { s.str(), getFont("imagine"), 25 };
			t.setPosition(Vector2f{13, 3} + offset);
			t.setColor(getColorB());
			drawOnWindow(t);
		}

		Text t { s.str(), getFont("imagine"), 25 };
		t.setPosition(13, 3);
		t.setColor(getColorMain());
		drawOnWindow(t);

		for (Text* textPtr : messageTextPtrs)
		{
			for(auto offset : offsets)
			{
				Text t{textPtr->getString(), getFont("imagine"), textPtr->getCharacterSize()};
				t.setPosition(textPtr->getPosition() + offset);
				t.setOrigin(t.getGlobalBounds().width / 2, 0);
				t.setColor(getColorB());
				drawOnWindow(t);
			}
			
			textPtr->setColor(getColorMain());
			drawOnWindow(*textPtr);
		}
	}
	void HexagonGame::drawBackground()
	{
		float div { 360.f / getSides() * 1.0001f };
		float distance { 1500 };

		VertexArray vertices{PrimitiveType::Triangles, 3};

		for(int i {0}; i < getSides(); i++)
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
		rotationDirection = !rotationDirection;
		setDelayMultiplier(getDelayMultiplier() + levelData.getDelayIncrement());
		fastSpin = 			levelData.getFastSpin();
		
		timeline.add(new Do([&]{ changeSides(); }));
	}
	void HexagonGame::changeSides()
	{
		if(manager.getComponentPtrsById("wall").size() > 0)
		{
			timeline.add(new Wait(10));
			timeline.add(new Do([&]{ changeSides(); }));
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
		checkAndSaveScore();
		playSound("beep");
		window.setGame(&mgPtr->getGame());
		mgPtr->init();
	}

	void HexagonGame::setLevelData(LevelData mLevelSettings)
	{
		levelData = mLevelSettings;
		styleData = getStyleData(levelData.getStyleId());
		musicData = getMusicData(levelData.getMusicId());
	}

	Game& HexagonGame::getGame()			{ return game; }
	Vector2f HexagonGame::getCenterPos() 	{ return centerPos; }

	float HexagonGame::getRadius() 			{ return radius; }
	Color HexagonGame::getColorMain()
	{
		if(getBlackAndWhite()) return Color::White;
		return styleData.getCurrentMain();
	}
	Color HexagonGame::getColorB()
	{
		if(getBlackAndWhite()) return Color::Black;
		return styleData.getCurrentB();
	}
	bool HexagonGame::isKeyPressed(Keyboard::Key mKey) { return window.isKeyPressed(mKey); }

	float HexagonGame::getSpeedMultiplier() { return levelData.getSpeedMultiplier(); }
	float HexagonGame::getDelayMultiplier() { return levelData.getDelayMultiplier(); }
	float HexagonGame::getRotationSpeed() 	{ return levelData.getRotationSpeed(); }
	int HexagonGame::getSides() 			{ return levelData.getSides(); }

	void HexagonGame::setSpeedMultiplier(float mSpeedMultiplier) { levelData.setSpeedMultiplier(mSpeedMultiplier); }
	void HexagonGame::setDelayMultiplier(float mDelayMultiplier) { levelData.setDelayMultiplier(mDelayMultiplier); }
	void HexagonGame::setRotationSpeed(float mRotationSpeed) 	 { levelData.setRotationSpeed(mRotationSpeed); }
	void HexagonGame::setSides(int mSides)
	{
		if (mSides < 3) mSides = 3;
		levelData.setValueInt("sides", mSides);
	}
}

