#include "HexagonGame.h"
#include "CPlayer.h"
#include "CWall.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include "SSVStart.h"
#include "PatternManager.h"
#include <fstream>
#include "Config.h"
#include "Factory.h"
#include <SFML/Audio.hpp>
#include "Assets.h"
#include "StyleData.h"

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	HexagonGame::HexagonGame() :
		window { (unsigned int)getWindowSizeX(), (unsigned int)getWindowSizeY(), getPixelMultiplier(), getLimitFps() }
	{				
		window.isFrameTimeStatic = getStaticFrameTime();
		window.staticFrameTime = getStaticFrameTimeValue();
		window.renderWindow.setVerticalSyncEnabled(getVsync());

		pm = new PatternManager(this);

		font.loadFromFile("imagine.ttf");
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(getWindowSizeX() / 2, getWindowSizeY() / 2);

		game.addUpdateFunc(	 [&](float frameTime) { update(frameTime); });
		game.addDrawFunc(	 [&](){ gameTexture.clear(Color::Black); }, -2);

		if(!getNoBackground())
			game.addDrawFunc([&](){ drawBackground(); }, 			   -1);

		game.addDrawFunc(	 [&](){ manager.draw(); }, 					0);
		game.addDrawFunc(	 [&](){ gameTexture.display(); }, 			1);
		game.addDrawFunc(	 [&](){ drawOnWindow(gameSprite); }, 		2);
		game.addDrawFunc(	 [&](){ drawDebugText(); }, 				3);

		setLevelData(getLevelData("tutorial"));

		newGame();
		
		window.setGame(&game);
		window.run();
	}
	HexagonGame::~HexagonGame() { delete pm; }

	void HexagonGame::newGame()
	{
		stopAllSounds();
		playSound("play");
		
		pm->resetAdj();

		stopLevelMusic();
		playLevelMusic();

		rotationDirection = getRnd(0, 100) > 50 ? true : false;

		hasDied = false;
		mustRestart = false;
		currentTime = 0;
		incrementTime = 0;
		sides = levelData.getSidesStart();
		radius = minRadius;

		manager.clear();
		createPlayer(manager, this, centerPos);

		timeline = Timeline{};

		speedMult 		= levelData.getSpeedMultiplier();
		rotationSpeed 	= levelData.getRotationSpeed();
		delayMult 		= levelData.getDelayMultiplier();


	}
	void HexagonGame::death()
	{
		playSound("death");
		playSound("gameOver");
		stopLevelMusic();
		hasDied = true;
	}

	void HexagonGame::drawOnTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void HexagonGame::drawOnWindow(Drawable &mDrawable) { window.renderWindow.draw(mDrawable); }

	void HexagonGame::update(float mFrameTime)
	{
		if(!hasDied)
		{
			manager.update(mFrameTime);
			currentTime += mFrameTime / 60.0f;
			incrementTime += mFrameTime / 60.0f;

			updateIncrement();
			updateLevel(mFrameTime);
			updateRadius(mFrameTime);
			if(!getBlackAndWhite()) updateColor(mFrameTime);
		}
		else rotationSpeed /= 1.001f;

		updateDebugKeys(mFrameTime);
		if(!getNoRotation()) updateRotation(mFrameTime);

		if(mustRestart) newGame();
	}
	inline void HexagonGame::updateIncrement()
	{
		if(incrementTime < levelData.getIncrementTime()) return;

		incrementTime = 0;
		incrementDifficulty();
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
		auto nextRotation = rotationSpeed * 10 * mFrameTime;
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
	inline void HexagonGame::updateDebugKeys(float mFrameTime)
	{
		if(Keyboard::isKeyPressed(Keyboard::R)) mustRestart = true;

		if(Keyboard::isKeyPressed(Keyboard::Num6)) { setLevelData(getLevelData("tutorial")); newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num1)) { setLevelData(getLevelData("easy")); newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num2)) { setLevelData(getLevelData("normal")); newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num3)) { setLevelData(getLevelData("hard")); newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num4)) { setLevelData(getLevelData("lunatic")); newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num5)) { setLevelData(getLevelData("extra")); newGame(); }

		if(Keyboard::isKeyPressed(Keyboard::Q)) sides++;
		if(Keyboard::isKeyPressed(Keyboard::W)) sides--;

		if(Keyboard::isKeyPressed(Keyboard::Z)) speedMult += 0.1f * mFrameTime;
		if(Keyboard::isKeyPressed(Keyboard::X))	speedMult -= 0.1f * mFrameTime;

		if(Keyboard::isKeyPressed(Keyboard::C)) rotationSpeed += 0.03f * mFrameTime;
		if(Keyboard::isKeyPressed(Keyboard::V))	rotationSpeed -= 0.03f * mFrameTime;
	}

	void HexagonGame::drawDebugText()
	{
		ostringstream s;
		s 	<< "time: " << toStr(currentTime).substr(0, 5) << endl
			<< "level: " << toStr(levelData.getName()) << endl;

		Text t { s.str(), font, 20 };
		t.setPosition(10, 0);
		t.setColor(Color::White);
		window.renderWindow.draw(t);
	}
	void HexagonGame::drawBackground()
	{
		float div { 360.f / sides * 1.0001f };
		float distance { 1500 };

		VertexArray vertices{PrimitiveType::Triangles, 3};

		for(int i {0}; i < sides; i++)
		{
			float angle { div * i };
			Color currentColor { styleData.getCurrentA() };

			if (i % 2 == 0)
			{
				currentColor = styleData.getCurrentB();
				if (i == sides - 1) currentColor = getColorDarkened(currentColor, 1.4f);
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
		playSound("levelUp");

		speedMult +=		levelData.getSpeedIncrement();
		rotationSpeed +=	levelData.getRotationSpeedIncrement();
		rotationDirection = !rotationDirection;
		delayMult += 		levelData.getDelayIncrement();
		fastSpin = 			levelData.getFastSpin();

		sides = getRnd(levelData.getSidesMin(), levelData.getSidesMax() + 1);

		timeline.clear();
		timeline.reset();
		pm->resetAdj();
		timeline.add(new Wait(60));
	}

	void HexagonGame::setLevelData(LevelData mLevelSettings)
	{
		levelData = mLevelSettings;
		styleData = getStyleData(levelData.getStyleId());
		musicData = getMusicData(levelData.getMusicId());
	}

	Vector2f HexagonGame::getCenterPos() 	{ return centerPos; }
	int HexagonGame::getSides() 			{ return sides; }
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
}

