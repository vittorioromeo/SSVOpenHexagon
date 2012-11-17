#include "MenuGame.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include "SSVStart.h"
#include <fstream>
#include "Config.h"
#include <SFML/Audio.hpp>
#include "Assets.h"
#include "StyleData.h"

namespace hg
{
	MenuGame::MenuGame(GameWindow& mGameWindow) : window(mGameWindow)
	{
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(getWindowSizeX() / 2, getWindowSizeY() / 2);

		menuTexture.create(getSizeX(), getSizeY(), 32);
		menuTexture.setView(View{Vector2f(window.renderWindow.getSize().x/2, 810/2), Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		menuTexture.setSmooth(true);
		menuSprite.setTexture(menuTexture.getTexture(), false);
		menuSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		menuSprite.setPosition(getWindowSizeX() / 2, getWindowSizeY() / 2);

		game.addUpdateFunc([&](float frameTime) { update(frameTime); });
		game.addDrawFunc([&]{ draw(); }, 0);

		levelDataIds = getAllLevelDataIds();
		setIndex(0);
	}

	void MenuGame::init()
	{
		stopAllMusic();
		stopAllSounds();

		playSound("openHexagon");
	}

	void MenuGame::setIndex(int mIndex)
	{
		currentIndex = mIndex;

		if(currentIndex > (int)(levelDataIds.size() - 1)) currentIndex = 0;
		else if(currentIndex < 0) currentIndex = levelDataIds.size() - 1;

		levelData = getLevelData(levelDataIds[currentIndex]);
		styleData = getStyleData(levelData.getStyleId());
	}

	void MenuGame::update(float mFrameTime)
	{
		styleData.update(mFrameTime);
		gameSprite.rotate(levelData.getRotationSpeed() * 10 * mFrameTime);

		if(inputDelay <= 0)
		{
			if(Keyboard::isKeyPressed(Keyboard::Right))
			{
				playSound("beep");
				setIndex(currentIndex + 1);

				inputDelay = 14;
			}
			else if(Keyboard::isKeyPressed(Keyboard::Left))
			{
				playSound("beep");
				setIndex(currentIndex - 1);

				inputDelay = 14;
			}
			else if(Keyboard::isKeyPressed(Keyboard::Return))
			{
				playSound("beep");
				window.setGame(&hexagonGamePtr->getGame());
				hexagonGamePtr->startFromMenu(getLevelData(levelDataIds[currentIndex]));

				inputDelay = 14;
			}
			else if(Keyboard::isKeyPressed(Keyboard::Escape)) inputDelay = 25;
		}
		else
		{
			inputDelay -= 1 * mFrameTime;

			if(inputDelay < 1.0f && Keyboard::isKeyPressed(Keyboard::Escape)) window.stop();
		}
	}
	void MenuGame::draw()
	{
		gameTexture.clear(styleData.getCurrentA());
		menuTexture.clear(Color{0,0,0,0});

		drawBackground();
		drawText();

		gameTexture.display();
		menuTexture.display();

		drawOnWindow(gameSprite);
		drawOnWindow(menuSprite);
	}
	void MenuGame::drawBackground()
	{
		Vector2f centerPos{0,0};
		int sides{6};
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
	void MenuGame::drawText()
	{
		Color mainColor{styleData.getCurrentMain()};

		title1.setOrigin(title1.getGlobalBounds().width / 2, 0);
		title1.setStyle(Text::Bold);
		title1.setColor(mainColor);
		title1.setPosition(window.renderWindow.getSize().x / 2, 45);
		drawOnMenuTexture(title1);

		title2.setOrigin(title2.getGlobalBounds().width / 2, 0);
		title2.setStyle(Text::Bold);
		title2.setColor(mainColor);
		title2.setPosition(window.renderWindow.getSize().x / 2, 80);
		drawOnMenuTexture(title2);

		title3.setOrigin(title3.getGlobalBounds().width / 2, 0);
		title3.setStyle(Text::Bold);
		title3.setColor(mainColor);
		title3.setPosition(window.renderWindow.getSize().x / 2, 240);
		drawOnMenuTexture(title3);

		levelTime.setString("best time: " + toStr(getScore(levelData.getId())));
		levelTime.setOrigin(levelTime.getGlobalBounds().width / 2, 0);
		levelTime.setColor(mainColor);
		levelTime.setPosition(window.renderWindow.getSize().x / 2, 768 - 425);
		drawOnMenuTexture(levelTime);

		levelName.setString(levelData.getName());
		levelName.setOrigin(levelName.getGlobalBounds().width / 2, 0);
		levelName.setColor(mainColor);
		levelName.setPosition(window.renderWindow.getSize().x / 2, 768 - 295 - 40*(countNewLines(levelData.getName())));
		drawOnMenuTexture(levelName);

		levelDesc.setString(levelData.getDescription());
		levelDesc.setOrigin(levelDesc.getGlobalBounds().width / 2, 0);
		levelDesc.setColor(mainColor);
		levelDesc.setPosition(window.renderWindow.getSize().x / 2, 768 - 220 + 25*(countNewLines(levelData.getName())) + 16*(countNewLines(levelData.getDescription())));
		drawOnMenuTexture(levelDesc);

		levelAuth.setString("author: " + levelData.getAuthor());
		levelAuth.setOrigin(levelAuth.getGlobalBounds().width / 2, 0);
		levelAuth.setColor(mainColor);
		levelAuth.setPosition(window.renderWindow.getSize().x / 2, 768 - 75);
		drawOnMenuTexture(levelAuth);

		MusicData musicData{getMusicData(levelData.getMusicId())};

		levelMusc.setString("music: " + musicData.getName() + " by " + musicData.getAuthor() + " (" + musicData.getAlbum() + ")");
		levelMusc.setOrigin(levelMusc.getGlobalBounds().width / 2, 0);
		levelMusc.setColor(mainColor);
		levelMusc.setPosition(window.renderWindow.getSize().x / 2, 768 - 60);
		drawOnMenuTexture(levelMusc);
	}
	
	void MenuGame::drawOnGameTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void MenuGame::drawOnMenuTexture(Drawable &mDrawable) { menuTexture.draw(mDrawable); }
	void MenuGame::drawOnWindow(Drawable &mDrawable) { window.renderWindow.draw(mDrawable); }

	Game& MenuGame::getGame() { return game; }
}

