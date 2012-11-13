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

		sbDeath.loadFromFile("pldead00.wav");
		sDeath.setBuffer(sbDeath);

		pm = new PatternManager(this);

		font.loadFromFile("imagine.ttf");
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(getWindowSizeX() / 2, getWindowSizeY() / 2);
		window.renderWindow.setVerticalSyncEnabled(true);

		game.addUpdateFunc(	 [&](float frameTime) { update(frameTime); });
		game.addDrawFunc(	 [&](){ gameTexture.clear(Color::Black); }, -2);

		if(!getNoBackground())
			game.addDrawFunc([&](){ drawBackground(); }, 			   -1);

		game.addDrawFunc(	 [&](){ manager.draw(); }, 					0);
		game.addDrawFunc(	 [&](){ gameTexture.display(); }, 			1);
		game.addDrawFunc(	 [&](){ drawOnWindow(gameSprite); }, 		2);
		game.addDrawFunc(	 [&](){ drawDebugText(); }, 				3);

		initLevelSettings();
		if(!getNoMusic()) initMusicData();

		newGame();

		window.setGame(&game);
		window.run();
	}
	HexagonGame::~HexagonGame() { delete pm; }

	void HexagonGame::newGame()
	{
		stopLevelMusic();
		playLevelMusic();

		rotationDirection = rnd(0, 100) > 50 ? true : false;

		hasDied = false;
		mustRestart = false;
		currentTime = 0;
		incrementTime = 0;
		sides = 6;
		radius = minRadius;
		backType = (BackType)rnd(0, 3);

		manager.clear();
		createPlayer(manager, this, centerPos);

		timeline = Timeline{};

		speedMult 				= getLevelSettings().getSpeedMultiplier();
		rotationSpeed 			= getLevelSettings().getRotationSpeed();
		delayMult 				= getLevelSettings().getDelayMultiplier();
	}
	void HexagonGame::death()
	{
		stopLevelMusic();
		hasDied = true;
		sDeath.play();
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
			if(!getBlackAndWhite()) updateColor(mFrameTime); else color = Color::White;
		}
		else rotationSpeed /= 1.001f;

		updateDebugKeys(mFrameTime);
		if(!getNoRotation()) updateRotation(mFrameTime);

		if(mustRestart) newGame();
	}
	inline void HexagonGame::updateIncrement()
	{
		if(incrementTime < getLevelSettings().getIncrementTime()) return;

		incrementTime = 0;
		incrementDifficulty();
	}
	inline void HexagonGame::updateLevel(float mFrameTime)
	{
		timeline.update(mFrameTime);

		if(timeline.isFinished())
		{
			timeline.clear();
			getLevelSettings().getRandomPattern()();
			timeline.reset();
		}
	}
	inline void HexagonGame::updateColor(float mFrameTime)
	{
		color = hue2color(hue / 255.0f);

		colorSwap += 1.f * mFrameTime;
		if (colorSwap > 100) colorSwap = 0;

		hue += hueIncrement * mFrameTime;

		if(backType == BackType::DARK)
		{
			hueIncrement = 1;
			if (hue > 254) hue = 0;
		}
		else if(backType == BackType::LIGHT)
		{
			if(hue < 149) hue = 150;
			if(hue <= 150) hueIncrement = 1;
			if(hue >= 255) hueIncrement = -1;
		}
		else if(backType == BackType::GRAY)
		{
			color = Color::White;

			if(hue < 149) hue = 150;
			if(hue <= 150) hueIncrement = 1;
			if(hue >= 255) hueIncrement = -1;
		}
	}
	inline void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = rotationSpeed * 10 * mFrameTime;
		if(rotationDirection) nextRotation *= -1;
		if(fastSpin > 0)
		{
			nextRotation += (smootherstep(0, 85, fastSpin) / 3.5f) * sign(nextRotation) * mFrameTime * 17.0f;
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
			radius = rnd(82, 92);
		}

		if(radius > minRadius) radius -= 0.7f * mFrameTime;
	}
	inline void HexagonGame::updateDebugKeys(float mFrameTime)
	{
		if(Keyboard::isKeyPressed(Keyboard::R)) mustRestart = true;

		if(Keyboard::isKeyPressed(Keyboard::Num1)) { levelPtr = &levels[rnd(0, levels.size())]; newGame(); }

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
			<< "level: " << toStr(getLevelSettings().getName()) << endl;
		//	<< "sides: " << toStr(sides) << endl
		//	<< "speed multiplier: " << toStr(speedMult) << endl
		//	<< "rotation: " << toStr(rotationSpeed) << endl
		//	<< "hue: " << toStr(hue) << endl;

		Text t { s.str(), font, 20 };
		t.setPosition(10, 0);
		t.setColor(Color::White);
		window.renderWindow.draw(t);
	}
	void HexagonGame::drawBackground()
	{
		float div { 360.f / sides * 1.0001f };
		float distance { 1500 };

		Color darkColor1  { darkenColor(color, 2.7f) };
		Color darkColor2  { darkenColor(darkColor1, 1.5f) };
		Color lightColor1 { Color(235,235,235,255) };
		Color lightColor2 { Color(190 + color.r / 5, 190 + color.g / 5, 190 + color.b / 5, 255) };
		Color color1, color2;

		if(backType == BackType::DARK)
		{
			color1 = darkColor1;
			color2 = darkColor2;
		}
		else if(backType == BackType::LIGHT)
		{
			color1 = lightColor1;
			color2 = lightColor2;
		}
		else if (backType == BackType::GRAY)
		{
			color1 = darkenColor(Color(255,255,255,255), hue / 30.0f);
			color2 = darkenColor(Color(200,200,200,255), hue / 30.0f);
		}

		if(colorSwap > 50) swap(color1, color2);

		VertexArray vertices{PrimitiveType::Triangles, 3};

		for(int i {0}; i < sides; i++)
		{
			float angle { div * i };
			Color currentColor { color1 };

			if (i % 2 == 0)
			{
				currentColor = color2;
				if (i == sides - 1) currentColor = darkenColor(currentColor, 1.4f);
			}

			Vector2f p1 = orbit(centerPos, angle + div * 0.5f, distance);
			Vector2f p2 = orbit(centerPos, angle - div * 0.5f, distance);

			vertices.append(Vertex{centerPos, currentColor});
			vertices.append(Vertex{p1, currentColor});
			vertices.append(Vertex{p2, currentColor});
		}

		gameTexture.draw(vertices);
	}

	void HexagonGame::initLevelSettings()
	{
		for(auto path : getAllJsonPaths("Levels/"))
		{
			Json::Value root;
			Json::Reader reader;
			ifstream test(path, std::ifstream::binary);

			bool parsingSuccessful = reader.parse( test, root, false );
			if (!parsingSuccessful) cout << reader.getFormatedErrorMessages() << endl;

			LevelSettings level{loadLevelFromJson(pm, root)};
			levels.push_back(level);
		}

		levelPtr = &levels[1];
	}
	void HexagonGame::initMusicData()
	{
		for(auto path : getAllJsonPaths("Music/"))
		{
			Json::Value root;
			Json::Reader reader;
			ifstream test(path, std::ifstream::binary);

			bool parsingSuccessful = reader.parse( test, root, false );
			if (!parsingSuccessful) cout << reader.getFormatedErrorMessages() << endl;

			MusicData musicData{loadMusicFromJson(root)};
			musicMap.insert(make_pair(musicData.getId(), musicData));
		}
	}

	void HexagonGame::playLevelMusic()
	{
		if(!getNoMusic()) musicMap.find(levelPtr->getMusicId())->second.playRandomSegment(musicPtr);
	}
	void HexagonGame::stopLevelMusic()
	{
		if(!getNoMusic()) if(musicPtr != nullptr) musicPtr->stop();
	}

	void HexagonGame::incrementDifficulty()
	{
		speedMult += getLevelSettings().getSpeedIncrement();
		rotationSpeed += getLevelSettings().getRotationSpeedIncrement();
		rotationDirection = !rotationDirection;
		delayMult += getLevelSettings().getDelayIncrement();
		fastSpin = getLevelSettings().getFastSpin();

		if (sides != 6) sides = 6;
		else  			sides = rnd(getLevelSettings().getSidesMin(), getLevelSettings().getSidesMax() + 1);

		timeline.clear();
		timeline.reset();
		timeline.add(new Wait(40));
	}
	LevelSettings& HexagonGame::getLevelSettings() { return *levelPtr; }

	Vector2f HexagonGame::getCenterPos() 	{ return centerPos; }
	int HexagonGame::getSides() 			{ return sides; }
	float HexagonGame::getRadius() 			{ return radius; }
	Color HexagonGame::getColor() 			{ return color; }
}

