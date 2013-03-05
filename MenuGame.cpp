// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <string>
#include "Global/Assets.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "MenuGame.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;
using namespace ssvms;

namespace hg
{
	MenuGame::MenuGame(HexagonGame& mHexagonGame, GameWindow& mGameWindow) : hexagonGame(mHexagonGame), window(mGameWindow)
	{
		// Title bar
		getAssetManager().getTexture("titleBar.png").setSmooth(true);
		titleBar.setOrigin({0, 0});
		titleBar.setScale({0.5f, 0.5f});
		titleBar.setPosition(overlayCamera.getConvertedCoords({20, 20}));

		// Credits bar 1
		getAssetManager().getTexture("creditsBar1.png").setSmooth(true);
		creditsBar1.setOrigin({1024, 0});
		creditsBar1.setScale({0.373f, 0.373f});
		creditsBar1.setPosition(overlayCamera.getConvertedCoords(Vector2i(getWidth() - 20.f, 20.f)));
		
		// Credits bar 2
		getAssetManager().getTexture("creditsBar2.png").setSmooth(true);
		creditsBar2.setOrigin({1024, 116});
		creditsBar2.setScale({0.373f, 0.373f});
		creditsBar2.setPosition(overlayCamera.getConvertedCoords(Vector2i(getWidth() - 20.f, 160.f / getZoomFactor())));

		// Bottom bar
		getAssetManager().getTexture("bottomBar.png").setSmooth(true);
		float scaleFactor{getWidth() * getZoomFactor() / 2048.f};
		bottomBar.setOrigin({0, 112.f});
		bottomBar.setScale({scaleFactor, scaleFactor});
		bottomBar.setPosition(overlayCamera.getConvertedCoords(Vector2i(0, getHeight())));

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

		// Options menu
		Category& options(optionsMenu.createCategory("options"));
		options.createItem<Items::Toggle>("screen rotation", 	[&]{ return !getNoRotation(); }, 	[&]{ setNoRotation(false); }, 	[&]{ setNoRotation(true); });
		options.createItem<Items::Toggle>("display background", [&]{ return !getNoBackground(); }, 	[&]{ setNoBackground(false); }, [&]{ setNoBackground(true); });
		options.createItem<Items::Toggle>("b&w colors", 		[&]{ return getBlackAndWhite(); }, 	[&]{ setBlackAndWhite(true); }, [&]{ setBlackAndWhite(false); });
		options.createItem<Items::Toggle>("sounds", 			[&]{ return !getNoSound(); }, 		[&]{ setNoSound(false); }, 		[&]{ setNoSound(true); });
		options.createItem<Items::Toggle>("music", 				[&]{ return !getNoMusic(); }, 		[&]{ setNoMusic(false); }, 		[&]{ setNoMusic(true); });
		options.createItem<Items::Toggle>("3D effect",			[&]{ return get3D(); }, 			[&]{ set3D(true); }, 			[&]{ set3D(false); });
		options.createItem<Items::Toggle>("pulse effect", 		[&]{ return getPulse(); }, 			[&]{ setPulse(true); }, 		[&]{ setPulse(false); });
		options.createItem<Items::Toggle>("invincibility",		[&]{ return getInvincible(); },		[&]{ setInvincible(true); }, 	[&]{ setInvincible(false); });
		options.createItem<Items::Single>("go windowed", 	[&]{ setFullscreen(window, false); });
		options.createItem<Items::Single>("go fullscreen", 	[&]{ setFullscreen(window, true); });
		options.createItem<Items::Single>("back", 			[&]{ state = StateType::LEVEL_SELECTION; });
	}

	void MenuGame::init() { stopAllMusic(); stopAllSounds(); playSound("openHexagon.ogg"); }

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
			Event e; window.pollEvent(e);

			if(e.type == Event::TextEntered)
			{
				if(e.text.unicode > 47 && e.text.unicode < 126) if(profileCreationName.size() < 16) profileCreationName.append(toStr((char)(e.text.unicode)));
				if(e.text.unicode == 8) if(!profileCreationName.empty()) profileCreationName.erase(profileCreationName.end() - 1);
				if(e.text.unicode == 13) if(!profileCreationName.empty())
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
				if(window.isKeyPressed(Keyboard::Left)) { playSound("beep.ogg"); profileIndex--; inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Right)) { playSound("beep.ogg"); profileIndex++; inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Return)) { playSound("beep.ogg"); setCurrentProfile(profileCreationName); state = StateType::LEVEL_SELECTION; inputDelay = 30; }
				else if(window.isKeyPressed(Keyboard::F1)) { playSound("beep.ogg"); profileCreationName = ""; state = StateType::PROFILE_CREATION; inputDelay = 14; }
			}
		}
		else if(state == StateType::LEVEL_SELECTION)
		{
			styleData.update(mFrameTime);

			backgroundCamera.rotate(levelData.getRotationSpeed() * 10 * mFrameTime);

			if(inputDelay <= 0)
			{
				if(window.isKeyPressed(Keyboard::Right)) { playSound("beep.ogg"); setIndex(currentIndex + 1); inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Left)){ playSound("beep.ogg"); setIndex(currentIndex - 1); inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Up)) { playSound("beep.ogg"); difficultyMultIndex++; inputDelay = 12; }
				else if(window.isKeyPressed(Keyboard::Down)) { playSound("beep.ogg"); difficultyMultIndex--; inputDelay = 12; }
				else if(window.isKeyPressed(Keyboard::Return))
				{
					playSound("beep.ogg");
					window.setGameState(hexagonGame.getGame());
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
				else if(window.isKeyPressed(Keyboard::F3) || window.isKeyPressed(Keyboard::K)) { playSound("beep.ogg"); state = StateType::OPTIONS; inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::F4) || window.isKeyPressed(Keyboard::L))
				{
					playSound("beep.ogg");

					packIndex = (packIndex + 1) % getPackPaths().size();
					levelDataIds = getMenuLevelDataIdsByPack(getPackPaths()[packIndex]);
					setIndex(0);

					inputDelay = 14;
				}
			}
		}
		else if(state == StateType::OPTIONS)
		{
			if(inputDelay <= 0)
			{
				if(window.isKeyPressed(Keyboard::Left)) { playSound("beep.ogg"); optionsMenu.decreaseCurrentItem(); inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Right)) { playSound("beep.ogg"); optionsMenu.increaseCurrentItem(); inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Up)) { playSound("beep.ogg"); optionsMenu.selectPreviousItem(); inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Down)) { playSound("beep.ogg"); optionsMenu.selectNextItem(); inputDelay = 14; }
				else if(window.isKeyPressed(Keyboard::Return)) { playSound("beep.ogg"); optionsMenu.executeCurrentItem(); inputDelay = 14; }
			}
		}
	}
	void MenuGame::draw()
	{
		window.clear(Color{0, 0, 0, 0});
		
		if(state == StateType::LEVEL_SELECTION)
		{
			window.clear(styleData.getColors()[0]);
			backgroundCamera.apply();
			styleData.drawBackground(window.getRenderWindow(), Vector2f{0,0}, 6);

			overlayCamera.apply(); drawLevelSelection(); render(bottomBar);
		}
		else if(state == StateType::PROFILE_CREATION) { window.clear(Color::Black); overlayCamera.apply(); drawProfileCreation(); }
		else if(state == StateType::PROFILE_SELECTION) { window.clear(Color::Black); overlayCamera.apply(); drawProfileSelection(); }
		else if(state == StateType::OPTIONS) { window.clear(Color::Black); overlayCamera.apply(); drawOptions(); }

		overlayCamera.apply(); render(titleBar); render(creditsBar1); render(creditsBar2); 
	}

	void MenuGame::renderText(const string& mString, Text& mText, sf::Vector2f mPosition)
	{
		mText.setString(mString);

		if(state != StateType::LEVEL_SELECTION || getBlackAndWhite()) mText.setColor(Color::White);
		else mText.setColor(styleData.getMainColor());

		mText.setPosition(overlayCamera.getConvertedCoords(Vector2i(mPosition)).x, mPosition.y + 160);
		render(mText);
	}

	void MenuGame::drawLevelSelection()
	{		
		MusicData musicData{getMusicData(levelData.getMusicId())};
		PackData packData{getPackData(levelData.getPackPath().substr(6, levelData.getPackPath().size() - 7))};
		string packName{packData.getName()}, packNames{""}; for(string packName : getPackNames()) packNames.append(packName + "\n"); // TODO!!!!

		renderText("profile: " + getCurrentProfile().getName(), cProfText, {20, 0});
		renderText("pack: " + packName + " (" + toStr(packIndex + 1) + "/" + toStr(getPackPaths().size()) + ")", cProfText, {20, 20});
		renderText("best time: " + toStr(getScore(getScoreValidator(levelData.getId(), difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]))), cProfText, {20, 40});
		if(difficultyMultipliers.size() > 1) renderText("difficulty: " + toStr(difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]), cProfText, {20, 60});
		renderText(levelData.getName(), levelName, {20, 50 + 120});
		renderText(levelData.getDescription(), levelDesc, {20, 50 + 195 + 60.f * (countNewLines(levelData.getName()))});
		renderText("author: " + levelData.getAuthor(), levelAuth, {20, -30 + 500});
		renderText("music: " + musicData.getName() + " by " + musicData.getAuthor() + " (" + musicData.getAlbum() + ")", levelMusc, {20, -30 + 515});
		renderText("(" + toStr(currentIndex + 1) + "/" + toStr(levelDataIds.size()) + ")", levelMusc, {20, -30 + 530});
	}
	void MenuGame::drawProfileCreation()
	{
		renderText("profile creation", cProfText, 						{20, 768 - 395});
		renderText("insert profile name", cProfText, 					{20, 768 - 375});
		renderText("press enter when done", cProfText, 					{20, 768 - 335});
		renderText("keep esc pressed to exit", cProfText, 				{20, 768 - 315});
		renderText(profileCreationName, levelName, 						{20, 768 - 245 - 40});
	}
	void MenuGame::drawProfileSelection()
	{
		renderText("profile selection", cProfText, 						{20, 768 - 395});
		renderText("press left/right to browse profiles", cProfText, 	{20, 768 - 375});
		renderText("press enter to select profile", cProfText, 			{20, 768 - 355});
		renderText("press f1 to create a new profile", cProfText, 		{20, 768 - 335});
		renderText(profileCreationName, levelName, 						{20, 768 - 245 - 40});
	}
	void MenuGame::drawOptions()
	{
		renderText(optionsMenu.getCurrentCategory().getName(), levelDesc, {20, 20});

		vector<ItemBase*>& currentItems(optionsMenu.getCurrentItems());
		for(int i{0}; i < static_cast<int>(currentItems.size()); ++i)
		{
			string name, itemName{currentItems[i]->getName()};
			if(i == optionsMenu.getCurrentItemsIndex()) name.append(">> ");
			name.append(itemName);

			int extraSpacing{0};
			if(itemName == "back") extraSpacing = 20;
			renderText(name, cProfText, {20, 60.f + i * 20 + extraSpacing});
		}
	}

	void MenuGame::render(Drawable &mDrawable) { window.draw(mDrawable); }

	GameState& MenuGame::getGame() { return game; }
}
