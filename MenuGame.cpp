// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

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

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;

namespace hg
{
	MenuGame::MenuGame(HexagonGame& mHexagonGame, GameWindow& mGameWindow) : hexagonGame(mHexagonGame), window(mGameWindow)
	{
		overlayCamera.move({0, 20});

		game.onUpdate += [&](float mFrameTime) { update(mFrameTime); };
		game.onDraw += [&]{ draw(); };

		levelDataIds = getMenuLevelDataIdsByPack(getPackPaths()[packIndex]);
		setIndex(0);
		
		if(getProfilesSize() == 0) state = StateType::PROFILE_CREATION;
		else if(getProfilesSize() == 1)
		{
			setCurrentProfile(getFirstProfileName());
			state = StateType::LEVEL_SELECTION;
		}
	}

	void MenuGame::init()
	{
		stopAllMusic();
		stopAllSounds();

		playSound("openHexagon.ogg");
	}

	void MenuGame::setIndex(int mIndex)
	{
		currentIndex = mIndex;

		if(currentIndex > (int)(levelDataIds.size() - 1)) currentIndex = 0;
		else if(currentIndex < 0) currentIndex = levelDataIds.size() - 1;

		levelData = getLevelData(levelDataIds[currentIndex]);
		styleData = getStyleData(levelData.getStyleId());
		difficultyMultipliers = levelData.getDifficultyMultipliers();
		difficultyMultIndex = find(begin(difficultyMultipliers), end(difficultyMultipliers), 1) - begin(difficultyMultipliers);
	}

	void MenuGame::update(float mFrameTime)
	{
		if(inputDelay <= 0)
		{
			if(window.isKeyPressed(Keyboard::LAlt) && window.isKeyPressed(Keyboard::Return))
			{
				setFullscreen(window, !window.getFullscreen());
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
				if (e.text.unicode > 47 && e.text.unicode < 126) if (profileCreationName.size() < 16) profileCreationName.append(toStr((char)(e.text.unicode)));
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
					playSound("beep.ogg");
					profileIndex--;
					
					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Right))
				{
					playSound("beep.ogg");
					profileIndex++;

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Return))
				{
					playSound("beep.ogg");
					setCurrentProfile(profileCreationName);
					state = StateType::LEVEL_SELECTION;

					inputDelay = 30;
				}
				else if(window.isKeyPressed(Keyboard::F1))
				{
					playSound("beep.ogg");
					profileCreationName = "";
					state = StateType::PROFILE_CREATION;

					inputDelay = 14;
				}
			}
		}
		else if (state == StateType::LEVEL_SELECTION)
		{
			styleData.update(mFrameTime);

			backgroundCamera.rotate(levelData.getRotationSpeed() * 10 * mFrameTime);

			if(inputDelay <= 0)
			{
				if(window.isKeyPressed(Keyboard::Right))
				{
					playSound("beep.ogg");
					setIndex(currentIndex + 1);

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Left))
				{
					playSound("beep.ogg");
					setIndex(currentIndex - 1);

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::Up))
				{
					playSound("beep.ogg");
					difficultyMultIndex++;
					inputDelay = 12;
				}
				else if(window.isKeyPressed(Keyboard::Down))
				{
					playSound("beep.ogg");
					difficultyMultIndex--;
					inputDelay = 12;
				}
				else if(window.isKeyPressed(Keyboard::Return))
				{
					playSound("beep.ogg");
					window.setGame(&hexagonGame.getGame());
					hexagonGame.newGame(levelDataIds[currentIndex], true, difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]);

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::F2) || window.isKeyPressed(Keyboard::J))
				{
					playSound("beep.ogg");
					profileCreationName = "";
					state = StateType::PROFILE_SELECTION;

					inputDelay = 14;
				}
				else if(window.isKeyPressed(Keyboard::F3) || window.isKeyPressed(Keyboard::K))
				{
					playSound("beep.ogg");
					setPulse(!getPulse());

					inputDelay = 14;
				}
				else if (window.isKeyPressed(Keyboard::F4) || window.isKeyPressed(Keyboard::L))
				{
					playSound("beep.ogg");

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
		backgroundCamera.apply();

		window.clear(Color{0, 0, 0, 0});
		
		if(state == StateType::LEVEL_SELECTION)
		{
			window.clear(styleData.getColors()[0]);
			styleData.drawBackground(window.getRenderWindow(), Vector2f{0,0}, 6);

			overlayCamera.apply();
			drawLevelSelection();
			backgroundCamera.apply();
		}
		else if(state == StateType::PROFILE_CREATION)
		{
			window.clear(Color::Black);

			overlayCamera.apply();
			drawProfileCreation();
			backgroundCamera.apply();
		}
		else if(state == StateType::PROFILE_SELECTION)
		{
			window.clear(Color::Black);

			overlayCamera.apply();
			drawProfileSelection();
			backgroundCamera.apply();
		}

		backgroundCamera.unapply();
	}

	void MenuGame::positionAndDrawCenteredText(Text& mText, Color mColor, float mElevation, bool mBold)
	{
		mText.setOrigin(mText.getGlobalBounds().width / 2, 0);
		if(mBold) mText.setStyle(Text::Bold);
		mText.setColor(mColor);
		mText.setPosition(getWidth() / 2, mElevation);
		drawOnWindow(mText);
	}

	void MenuGame::drawLevelSelection()
	{		
		Color mainColor{styleData.getMainColor()};
		MusicData musicData{getMusicData(levelData.getMusicId())};

		positionAndDrawCenteredText(title1, mainColor, 45, true);
		positionAndDrawCenteredText(title2, mainColor, 80, true);
		positionAndDrawCenteredText(title3, mainColor, 214, true);
		positionAndDrawCenteredText(title4, mainColor, 250, true);

		levelTime.setString("best time: " + toStr(getScore(getScoreValidator(levelData.getId(), difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]))));
		positionAndDrawCenteredText(levelTime, mainColor, 768 - 425, false);

		cProfText.setString("(J)(F2) profile: " + getCurrentProfile().getName());
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 375, false);
		cProfText.setString("(K)(F3) pulse: " + (getPulse() ? toStr("enabled") : toStr("disabled")));
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 355, false);

		PackData packData{getPackData(levelData.getPackPath().substr(6, levelData.getPackPath().size() - 7))};
		string packName{packData.getName()};
		cProfText.setString("(L)(F4) level pack: " + packName + " (" + toStr(packIndex + 1) + "/" + toStr(getPackPaths().size()) + ")");
		positionAndDrawCenteredText(cProfText, mainColor, 768 - 335, false);

		if(difficultyMultipliers.size() > 1)
		{
			cProfText.setString("(up/down) difficulty multiplier: " + toStr(difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]));
			positionAndDrawCenteredText(cProfText, mainColor, 768 - 315, false);
		}
		
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

	void MenuGame::drawOnWindow(Drawable &mDrawable) { window.draw(mDrawable); }

	GameState& MenuGame::getGame() { return game; }
}
