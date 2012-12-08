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

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <SFML/Audio.hpp>
#include <SSVStart.h>
#include "Data/StyleData.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Utils/Utils.h"
#include "MenuGame.h"

namespace hg
{
	MenuGame::MenuGame(GameWindow& mGameWindow) : window(mGameWindow), state(StateType::PROFILE_SELECTION)
	{
		recreateTextures();

		game.addUpdateFunc([&](float frameTime) { update(frameTime); });
		game.addDrawFunc([&]{ draw(); }, 0);

		levelDataIds = getMenuLevelDataIdsByPack(getPackPaths()[packIndex]);
		setIndex(0);
		
		if(getProfilesSize() == 0) state = StateType::PROFILE_CREATION;
		else if(getProfilesSize() == 1)
		{
			setCurrentProfile(getFirstProfileName());
			state = StateType::LEVEL_SELECTION;
		}
	}

	void MenuGame::recreateTextures()
	{		
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(window.getWidth() / 2, window.getHeight() / 2);

		menuTexture.create(getSizeX(), getSizeY(), 32);
		menuTexture.setView(View{Vector2f(window.getWidth()/2, 810/2), Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		menuTexture.setSmooth(true);
		menuSprite.setTexture(menuTexture.getTexture(), false);
		menuSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		menuSprite.setPosition(getWidth() / 2, getHeight() / 2);
	}

	void MenuGame::init()
	{
		stopAllMusic();
		stopAllSounds();

		playSound("open_hexagon");
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
		if(inputDelay <= 0)
		{
			if(window.isKeyPressed(Keyboard::LAlt) && window.isKeyPressed(Keyboard::Return))
			{
				setFullscreen(window, !window.getFullscreen());
				recreateTextures();
				hgPtr->recreateTextures();
				inputDelay = 25;
			}
			else if(window.isKeyPressed(Keyboard::Escape)) inputDelay = 25;
		}
		else
		{
			inputDelay -= 1 * mFrameTime;
			if(inputDelay < 1.0f && window.isKeyPressed(Keyboard::Escape)) window.stop();
		}

		if(state == StateType::PROFILE_CREATION)
		{
			Event e;
			window.pollEvent(e);

			if(e.type == Event::TextEntered)
			{
				if (e.text.unicode > 48 && e.text.unicode < 126) if (profileCreationName.size() < 16) profileCreationName.append(toStr((char)(e.text.unicode)));
				if (e.text.unicode == 8) if(!profileCreationName.empty()) profileCreationName.erase(profileCreationName.end() - 1);
				if (e.text.unicode == 13) if(!profileCreationName.empty())
				{
					createProfile(profileCreationName);
					setCurrentProfile(profileCreationName);
					inputDelay = 30;
					state = StateType::LEVEL_SELECTION;
				}
			}
		}
		else if(state == StateType::PROFILE_SELECTION)
		{
			vector<string> profileNames{getProfileNames()};
			profileCreationName = profileNames[profileIndex % profileNames.size()];

			if(inputDelay <= 0)
			{
				if(window.isKeyPressed(Keyboard::Left))
				{
					playSound("beep");
					profileIndex--;
					
					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Right))
				{
					playSound("beep");
					profileIndex++;

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Return))
				{
					playSound("beep");
					setCurrentProfile(profileCreationName);
					state = StateType::LEVEL_SELECTION;

					inputDelay = 30;
				}
				else if(window.isKeyPressed(Keyboard::F1))
				{
					playSound("beep");
					profileCreationName = "";
					state = StateType::PROFILE_CREATION;

					inputDelay = 14;
				}
			}
		}
		else if (state == StateType::LEVEL_SELECTION)
		{
			styleData.update(mFrameTime);
			gameSprite.rotate(levelData.getRotationSpeed() * 10 * mFrameTime);

			if(inputDelay <= 0)
			{
				if(window.isKeyPressed(Keyboard::Right))
				{
					playSound("beep");
					setIndex(currentIndex + 1);

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Left))
				{
					playSound("beep");
					setIndex(currentIndex - 1);

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Up))
				{
					playSound("beep");
					difficultyMult += 0.10f;
					if(difficultyMult > 2.6f) difficultyMult = 2.6f;

					inputDelay = 9;
				}
				else if(window.isKeyPressed(Keyboard::Down))
				{
					playSound("beep");
					difficultyMult -= 0.10;
					if(difficultyMult < 0.2f) difficultyMult = 0.2f;

					inputDelay = 9;
				}
				else if(window.isKeyPressed(Keyboard::Return))
				{
					playSound("beep");
					window.setGame(&hgPtr->getGame());
					hgPtr->newGame(levelDataIds[currentIndex], true, difficultyMult);

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::F2) || window.isKeyPressed(Keyboard::J))
				{
					playSound("beep");
					profileCreationName = "";
					state = StateType::PROFILE_SELECTION;

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::F3) || window.isKeyPressed(Keyboard::K))
				{
					playSound("beep");
					setPulse(!getPulse());

					inputDelay = 14;
				}
				else if (window.isKeyPressed(Keyboard::F4) || window.isKeyPressed(Keyboard::L))
				{
					playSound("beep");

					packIndex = (packIndex + 1) % getPackPaths().size();
					levelDataIds = getMenuLevelDataIdsByPack(getPackPaths()[packIndex]);
					setIndex(0);

					inputDelay = 14;
				}
			}
		}
	}
	void MenuGame::draw()
	{
		menuTexture.clear(Color{0,0,0,0});

		if(state == StateType::LEVEL_SELECTION)
		{
			gameTexture.clear(styleData.getColors()[0]);
			styleData.drawBackground(gameTexture, Vector2f{0,0}, 6);
			drawLevelSelection();
		}
		else if(state == StateType::PROFILE_CREATION)
		{
			gameTexture.clear(Color::Black);
			drawProfileCreation();
		}
		else if(state == StateType::PROFILE_SELECTION)
		{
			gameTexture.clear(Color::Black);
			drawProfileSelection();
		}

		gameTexture.display();
		menuTexture.display();

		drawOnWindow(gameSprite);
		drawOnWindow(menuSprite);
	}

	void MenuGame::positionAndDrawCenteredText(Text& mText, Color mColor, float mElevation, bool mBold)
	{
		mText.setOrigin(mText.getGlobalBounds().width / 2, 0);
		if(mBold) mText.setStyle(Text::Bold);
		mText.setColor(mColor);
		mText.setPosition(window.getWidth() / 2, mElevation);
		drawOnMenuTexture(mText);
	}

	void MenuGame::drawLevelSelection()
	{
		Color mainColor{styleData.getMainColor()};
		MusicData musicData{getMusicData(levelData.getMusicId())};

		positionAndDrawCenteredText(title1, mainColor, 45, true);
		positionAndDrawCenteredText(title2, mainColor, 80, true);
		positionAndDrawCenteredText(title3, mainColor, 214, true);
		positionAndDrawCenteredText(title4, mainColor, 250, true);

		levelTime.setString("best time: " + toStr(getScore(getScoreValidator(levelData.getId(), getPulse(), difficultyMult))));
		positionAndDrawCenteredText(levelTime, mainColor, 768 - 425, false);

		cProfText.setString("(J)(F2) profile: " + getCurrentProfile().getName());
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 375, false);
		cProfText.setString("(K)(F3) pulse: " + (getPulse() ? toStr("enabled") : toStr("disabled")));
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 355, false);

		string packName{levelData.getPackPath().substr(6, levelData.getPackPath().size() - 7)};
		cProfText.setString("(L)(F4) level pack: " + packName + " (" + toStr(packIndex + 1) + "/" + toStr(getPackPaths().size()) + ")");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 335, false);
		cProfText.setString("(up/down) difficulty multiplier: " + toStr(difficultyMult));
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 315, false);

		levelName.setString(levelData.getName());
		positionAndDrawCenteredText(levelName, mainColor, 768 - 255 - 20*(countNewLines(levelData.getName())), false);

		levelDesc.setString(levelData.getDescription());
		positionAndDrawCenteredText(levelDesc, mainColor, 768 - 180 + 45*(countNewLines(levelData.getName())), false);

		levelAuth.setString("author: " + levelData.getAuthor());
		positionAndDrawCenteredText(levelAuth, mainColor, 768 - 85, false);
		levelMusc.setString("music: " + musicData.getName() + " by " + musicData.getAuthor() + " (" + musicData.getAlbum() + ")");
		positionAndDrawCenteredText(levelMusc, mainColor, 768 - 70, false);

		levelMusc.setString("(" + toStr(currentIndex + 1) + "/" + toStr(levelDataIds.size()) + ")");
		positionAndDrawCenteredText(levelMusc, mainColor, 768 - 40, false);
	}
	void MenuGame::drawProfileCreation()
	{
		Color mainColor{Color::White};

		positionAndDrawCenteredText(title1, mainColor, 45, true);
		positionAndDrawCenteredText(title2, mainColor, 80, true);
		positionAndDrawCenteredText(title3, mainColor, 240, true);
		positionAndDrawCenteredText(title4, mainColor, 270, true);

		cProfText.setString("profile creation");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 395, false);

		cProfText.setString("insert profile name");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 375, false);

		cProfText.setString("press enter when done");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 335, false);

		cProfText.setString("keep esc pressed to exit");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 315, false);

		levelName.setString(profileCreationName);
		positionAndDrawCenteredText(levelName, mainColor, 768 - 245 - 40, false);
	}
	void MenuGame::drawProfileSelection()
	{
		Color mainColor{Color::White};

		positionAndDrawCenteredText(title1, mainColor, 45, true);
		positionAndDrawCenteredText(title2, mainColor, 80, true);
		positionAndDrawCenteredText(title3, mainColor, 240, true);
		positionAndDrawCenteredText(title4, mainColor, 270, true);

		cProfText.setString("profile selection");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 395, false);

		cProfText.setString("press left/right to browse profiles");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 375, false);

		cProfText.setString("press enter to select profile");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 355, false);

		cProfText.setString("press f1 to create a new profile");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 335, false);

		levelName.setString(profileCreationName);
		positionAndDrawCenteredText(levelName, mainColor, 768 - 245 - 40, false);
	}
	
	void MenuGame::drawOnGameTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void MenuGame::drawOnMenuTexture(Drawable &mDrawable) { menuTexture.draw(mDrawable); }
	void MenuGame::drawOnWindow(Drawable &mDrawable) { window.draw(mDrawable); }

	Game& MenuGame::getGame() { return game; }
}

