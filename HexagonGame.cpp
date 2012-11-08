#include "HexagonGame.h"
#include "CPlayer.h"
#include "CWall.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include "SSVStart.h"
#include "PatternManager.h"

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	void HexagonGame::initLevelSettings()
	{
								//s_m   s_i    rs     rs_i   d_m    fspin
		LevelSettings easy 		{1.30f, 0.10f, 0.05f, 0.04f, 1.20f, 0.00f};
		LevelSettings normal 	{1.60f, 0.15f, 0.09f, 0.05f, 0.95f, 0.00f};
		LevelSettings hard 		{1.80f, 0.20f, 0.13f, 0.06f, 0.90f, 85.0f};
		LevelSettings lunatic 	{2.30f, 0.25f, 0.19f, 0.07f, 0.85f, 85.0f};

		easy.pfuncs.push_back( [&](){ pm->alternateBarrageDiv(2); } );
		easy.pfuncs.push_back( [&](){ pm->spin(2); } );
		easy.pfuncs.push_back( [&](){ pm->zigZag(2); } );
		easy.pfuncs.push_back( [&](){ pm->mirrorSpin(2); } );

		normal.pfuncs.push_back( [&](){ pm->alternateBarrageDiv(2); } );
		normal.pfuncs.push_back( [&](){ pm->spin(2); } );
		normal.pfuncs.push_back( [&](){ pm->zigZag(2); } );
		normal.pfuncs.push_back( [&](){ pm->mirrorSpin(2); } );

		hard.pfuncs.push_back( [&](){ pm->alternateBarrageDiv(2); } );
		hard.pfuncs.push_back( [&](){ pm->spin(2); } );
		hard.pfuncs.push_back( [&](){ pm->zigZag(2); } );
		hard.pfuncs.push_back( [&](){ pm->mirrorSpin(2); } );

		lunatic.pfuncs.push_back( [&](){ pm->alternateBarrageDiv(2); } );
		lunatic.pfuncs.push_back( [&](){ pm->spin(2); } );
		lunatic.pfuncs.push_back( [&](){ pm->zigZag(2); } );
		lunatic.pfuncs.push_back( [&](){ pm->mirrorSpin(2); } );

		levelMap.insert(make_pair(Level::EASY, easy));
		levelMap.insert(make_pair(Level::NORMAL, normal));
		levelMap.insert(make_pair(Level::HARD, hard));
		levelMap.insert(make_pair(Level::LUNATIC, lunatic));
	}
	LevelSettings& HexagonGame::getLevelSettings() { return levelMap.find(level)->second; }

	HexagonGame::HexagonGame()
	{
		pm = new PatternManager(this);

		font.loadFromFile("C:/Windows/Fonts/imagine.ttf"); // TODO: fix paths
		gameTexture.create(sizeX, sizeY, 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{sizeX, sizeY}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(sizeX / 2, sizeY / 2);
		gameSprite.setPosition(windowSizeX / 2, windowSizeY / 2);
		window.renderWindow.setVerticalSyncEnabled(true);

		game.addUpdateFunc([&](float frameTime)
		{
			manager.update(frameTime);
			update(frameTime);
		});
		game.addDrawFunc([&](){gameTexture.clear(Color::Black);}, -2);
		game.addDrawFunc([&](){drawBackground();}, -1);
		game.addDrawFunc([&](){manager.draw();}, 0);
		game.addDrawFunc([&](){gameTexture.display();}, 1);
		game.addDrawFunc([&](){drawOnWindow(gameSprite);}, 2);
		game.addDrawFunc([&](){drawDebugText();}, 3);

		initLevelSettings();
		newGame();

		window.setGame(&game);
		window.run();
	}
	HexagonGame::~HexagonGame() { delete pm; }

	Entity* HexagonGame::createPlayer()
	{
		Entity* result { new Entity };
		manager.addEntity(result);
		result->addComponent(new CPlayer { this, centerPos } );
		return result;
	}

	void HexagonGame::newGame()
	{
		currentTime = 0;
		incrementTime = 0;
		sides = 6;
		radius = minRadius;

		manager.clear();
		createPlayer();

		speedMult 				= getLevelSettings().speed;
		rotationSpeed 			= getLevelSettings().rotation;
		speedIncrement 			= getLevelSettings().speedInc;
		rotationSpeedIncrement 	= getLevelSettings().rotationInc;
		delayMult 				= getLevelSettings().delay;
	}
	void HexagonGame::death() { mustRestart = true; }

	void HexagonGame::update(float mFrameTime)
	{
		currentTime += mFrameTime / 60.0f;
		incrementTime += mFrameTime / 60.0f;

		updateIncrement();
		updateDebugKeys(mFrameTime);
		updateLevel(mFrameTime);
		updateColor(mFrameTime);
		updateRotation(mFrameTime);
		updateRadius(mFrameTime);

		if(mustRestart)
		{
			mustRestart = false;
			newGame();
			return;
		}
	}
	inline void HexagonGame::updateIncrement()
	{
		constexpr float maxIncrement = 15;

		if(incrementTime < maxIncrement) return;

		incrementTime = 0;

		speedMult += speedIncrement;
		rotationSpeed += rotationSpeedIncrement;
		rotationDirection = !rotationDirection;
		fastSpin = getLevelSettings().fastSpin;

		if (sides != 6) 					sides = 6;
		else if (level == Level::EASY) 		sides = rnd(3, 7);
		else if (level == Level::NORMAL) 	sides = rnd(4, 7);
		else if (level == Level::HARD)		sides = rnd(4, 8);
		else if (level == Level::LUNATIC)	sides = rnd(6, 8);
	}
	inline void HexagonGame::updateLevel(float mFrameTime)
	{
		timeline.update(mFrameTime);

		if(timeline.isFinished())
		{
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
		if(Keyboard::isKeyPressed(Keyboard::Num1)) { level = Level::EASY; death(); }
		if(Keyboard::isKeyPressed(Keyboard::Num2)) { level = Level::NORMAL; death(); }
		if(Keyboard::isKeyPressed(Keyboard::Num3)) { level = Level::HARD; death(); }
		if(Keyboard::isKeyPressed(Keyboard::Num4)) { level = Level::LUNATIC; death(); }

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
		s << "time: " << toStr(currentTime).substr(0, 5) << endl
		  << "sides: " << toStr(sides) << endl
		  << "speed multiplier: " << toStr(speedMult) << endl
		  << "rotation: " << toStr(rotationSpeed) << endl
		  << "hue: " << toStr(hue) << endl;

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

		if(colorSwap > 50) swap(color1, color2); // TODO: add its own variable for swapping

		VertexArray vertices{ PrimitiveType::Triangles, 3 };

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

			vertices.append(Vertex{ centerPos, currentColor });
			vertices.append(Vertex{ p1, currentColor });
			vertices.append(Vertex{ p2, currentColor });
		}

		gameTexture.draw(vertices);
	}

	void HexagonGame::drawOnTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void HexagonGame::drawOnWindow(Drawable &mDrawable) { window.renderWindow.draw(mDrawable); }

	Vector2f HexagonGame::getCenterPos() { return centerPos; }
	int HexagonGame::getSides() { return sides; }
	float HexagonGame::getRadius() { return radius; }
	Color HexagonGame::getColor() { return color; }
}
