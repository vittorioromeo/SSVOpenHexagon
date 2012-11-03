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
	HexagonGame::HexagonGame()
	{
		pm = new PatternManager(this);

		int windowSizeX { 1024 };
		int windowSizeY { 768 };
		int sizeX { 1300 };
		int sizeY { 1300 };

		font.loadFromFile("C:/Windows/Fonts/imagine.ttf"); // TODO: fix paths
		gameTexture.create(sizeX, sizeY, 32);
		gameTexture.setView(View(Vector2f(windowSizeX / 2, windowSizeY / 2), Vector2f(sizeX, sizeY)));
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(sizeX / 2, sizeY / 2);
		gameSprite.setPosition(windowSizeX / 2, windowSizeY / 2);
		window.renderWindow.setVerticalSyncEnabled(true);
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
		createPlayer();

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

		window.setGame(&game);
		window.run();
	}

	void HexagonGame::update(float mFrameTime)
	{
		currentTime += mFrameTime / 60.0f;
		updateDebugKeys(mFrameTime);
		updateLevel(mFrameTime);
		updateColor(mFrameTime);

		gameSprite.rotate(rotationSpeed * 10 * mFrameTime);
	}
	inline void HexagonGame::updateLevel(float mFrameTime)
	{
		timeline.update(mFrameTime);

		if(timeline.isFinished())
		{
			pm->zigZag(3);
			pm->mirrorSpin(6);
			pm->alternateBarrageDiv(3);
			pm->alternateBarrageDiv(2);
			pm->spin(6);
			timeline.reset();
		}
	}
	inline void HexagonGame::updateColor(float mFrameTime)
	{
		hue += 1.f * mFrameTime;
		color = hue2color(hue / 255.0f);
		if (hue > 254) hue = 0;
	}
	inline void HexagonGame::updateDebugKeys(float mFrameTime)
	{
		if(Keyboard::isKeyPressed(Keyboard::Up)) radius += 1 * mFrameTime;
		if(Keyboard::isKeyPressed(Keyboard::Down)) radius -= 1 * mFrameTime;

		if(Keyboard::isKeyPressed(Keyboard::Q)) sides++;
		if(Keyboard::isKeyPressed(Keyboard::W)) sides--;

		if(Keyboard::isKeyPressed(Keyboard::A)) thickness += 1 * mFrameTime;
		if(Keyboard::isKeyPressed(Keyboard::S))	thickness -= 1 * mFrameTime;

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
		  << "thickness: " << toStr(thickness) << endl
		  << "speed multiplier: " << toStr(speedMult) << endl
		  << "rotation: " << toStr(rotationSpeed) << endl;

		Text t { s.str(), font, 20 };
		t.setPosition(10, 0);
		t.setColor(Color::White);
		window.renderWindow.draw(t);
	}
	void HexagonGame::drawBackground()
	{
		bool whiteScheme { false }; // TODO: controlled by class

		float div { 360.f / sides };
		float distance { 1500 };

		Color darkColor1  { darkenColor(color, 2.7f) };
		Color darkColor2  { darkenColor(darkColor1, 1.5f) };
		Color lightColor1 { Color(235,235,235,255) };
		Color lightColor2 { Color(190 + color.r / 5, 190 + color.g / 5, 190 + color.b / 5, 255) };
		Color color1, color2;

		if(whiteScheme)
		{
			color1 = lightColor1;
			color2 = lightColor2;
		}
		else
		{
			color1 = darkColor1;
			color2 = darkColor2;
		}

		if((int)(hue / 29) % 2 == 0) swap(color1, color2); // TODO: add its own variable for swapping

		for(int i { 0 }; i < sides; i++)
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

			VertexArray vertices{ PrimitiveType::Triangles, 3 };
			vertices.append(Vertex{ centerPos, currentColor });
			vertices.append(Vertex{ p1, currentColor });
			vertices.append(Vertex{ p2, currentColor });
			gameTexture.draw(vertices);
		}
	}

	void HexagonGame::drawOnTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void HexagonGame::drawOnWindow(Drawable &mDrawable) { window.renderWindow.draw(mDrawable); }

	Vector2f HexagonGame::getCenterPos() { return centerPos; }
	int HexagonGame::getSides() { return sides; }
	float HexagonGame::getRadius() { return radius; }
	Color HexagonGame::getColor() { return color; }
}
