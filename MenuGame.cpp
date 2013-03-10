// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <string>
#include "Global/Assets.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "MenuGame.h"
#include "Online/Online.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace ssvs::Input;
using namespace sses;
using namespace ssvms;

namespace hg
{
	MenuGame::MenuGame(HexagonGame& mHexagonGame, GameWindow& mGameWindow) : hexagonGame(mHexagonGame), window(mGameWindow)
	{
		initAssets();

		game.onUpdate += [&](float mFrameTime) { update(mFrameTime); };
		game.onDraw += [&]{ draw(); };

		levelDataIds = getLevelIdsByPack(getPackPaths()[packIndex]);
		setIndex(0);

		if(getProfilesSize() == 0) state = States::PROFILE_NEW;
		else if(getProfilesSize() == 1) { setCurrentProfile(getFirstProfileName()); state = States::MAIN; }

		initOptionsMenu();
		initInput();
	}

	void MenuGame::init() { stopAllMusic(); stopAllSounds(); playSound("openHexagon.ogg"); }
	void MenuGame::initAssets()
	{
		getAssetManager().getTexture("titleBar.png").setSmooth(true);
		getAssetManager().getTexture("creditsBar1.png").setSmooth(true);
		getAssetManager().getTexture("creditsBar2.png").setSmooth(true);
		getAssetManager().getTexture("bottomBar.png").setSmooth(true);

		titleBar.setOrigin({0, 0});
		titleBar.setScale({0.5f, 0.5f});
		titleBar.setPosition(overlayCamera.getConvertedCoords({20, 20}));

		versionText.setString(toStr(getVersion()));
		versionText.setColor(Color::White);
		versionText.setPosition(titleBar.getPosition() + Vector2f{titleBar.getGlobalBounds().width - 67, titleBar.getGlobalBounds().top});

		creditsBar1.setOrigin({1024, 0});
		creditsBar1.setScale({0.373f, 0.373f});
		creditsBar1.setPosition(overlayCamera.getConvertedCoords(Vector2i(getWidth() - 20.f, 20.f)));

		creditsBar2.setOrigin({1024, 116});
		creditsBar2.setScale({0.373f, 0.373f});
		creditsBar2.setPosition(overlayCamera.getConvertedCoords(Vector2i(getWidth() - 20.f, 160.f / getZoomFactor())));

		float scaleFactor{getWidth() * getZoomFactor() / 2048.f};
		bottomBar.setOrigin({0, 112.f});
		bottomBar.setScale({scaleFactor, scaleFactor});
		bottomBar.setPosition(overlayCamera.getConvertedCoords(Vector2i(0, getHeight())));
	}
	void MenuGame::initOptionsMenu()
	{
		namespace i = ssvms::Items;
		auto& main(optionsMenu.createCategory("options"));
		auto& gfx(optionsMenu.createCategory("graphics"));
		auto& sfx(optionsMenu.createCategory("audio"));
		auto& play(optionsMenu.createCategory("gameplay"));
		auto& debug(optionsMenu.createCategory("debug"));

		main.create<i::Goto>("gameplay", play);
		main.create<i::Goto>("graphics", gfx);
		main.create<i::Goto>("audio", sfx);
		main.create<i::Goto>("debug", debug);
		main.create<i::Toggle>("online", [&]{ return getOnline(); }, [&]{ setOnline(true); }, [&]{ setOnline(false); });
		main.create<i::Single>("back", [&]{ state = States::MAIN; });

		gfx.create<i::Toggle>("rotation",	[&]{ return !getNoRotation(); }, 	[&]{ setNoRotation(false); }, 	[&]{ setNoRotation(true); });
		gfx.create<i::Toggle>("background",	[&]{ return !getNoBackground(); }, 	[&]{ setNoBackground(false); }, [&]{ setNoBackground(true); });
		gfx.create<i::Toggle>("b&w colors", [&]{ return getBlackAndWhite(); }, 	[&]{ setBlackAndWhite(true); }, [&]{ setBlackAndWhite(false); });
		gfx.create<i::Toggle>("3D effect",	[&]{ return get3D(); }, 			[&]{ set3D(true); }, 			[&]{ set3D(false); });
		gfx.create<i::Toggle>("pulse", 		[&]{ return getPulse(); }, 			[&]{ setPulse(true); }, 		[&]{ setPulse(false); });
		gfx.create<i::Single>("go windowed", 	[&]{ setFullscreen(window, false); });
		gfx.create<i::Single>("go fullscreen", 	[&]{ setFullscreen(window, true); });
		gfx.create<i::Goto>("back", main);

		sfx.create<i::Toggle>("sounds",	[&]{ return !getNoSound(); }, 	[&]{ setNoSound(false); }, 	[&]{ setNoSound(true); });
		sfx.create<i::Toggle>("music",	[&]{ return !getNoMusic(); },	[&]{ setNoMusic(false); }, 	[&]{ setNoMusic(true); });
		sfx.create<i::Goto>("back", main);

		play.create<i::Toggle>("autorestart", [&]{ return getAutoRestart(); }, [&]{ setAutoRestart(true); }, [&]{ setAutoRestart(false); });
		play.create<i::Goto>("back", main);

		debug.create<i::Toggle>("invincible", [&]{ return getInvincible(); }, [&]{ setInvincible(true); }, [&]{ setInvincible(false); });
		debug.create<i::Goto>("back", main);
	}
	void MenuGame::initInput()
	{
		using k = Keyboard::Key;
		using t = Trigger::Types;
		using s = States;
		game.addInput({{k::Left}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) 		{  --profileIndex; }
			else if(state == s::MAIN) 		{ setIndex(currentIndex - 1); }
			else if(state == s::OPTIONS) 	{ optionsMenu.decreaseCurrentItem(); }
		}, t::SINGLE);
		game.addInput({{k::Right}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) 		{ ++profileIndex; }
			else if(state == s::MAIN) 		{ setIndex(currentIndex + 1); }
			else if(state == s::OPTIONS) 	{ optionsMenu.increaseCurrentItem(); }
		}, t::SINGLE);
		game.addInput({{k::Up}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::MAIN) 			{ ++difficultyMultIndex; }
			else if(state == s::OPTIONS) 	{ optionsMenu.selectPreviousItem(); }
		}, t::SINGLE);
		game.addInput({{k::Down}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::MAIN) 			{ --difficultyMultIndex; }
			else if(state == s::OPTIONS)	{ optionsMenu.selectNextItem(); }
		}, t::SINGLE);
		game.addInput({{k::Return}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) { setCurrentProfile(profileNewName); state = s::MAIN; }
			else if(state == s::MAIN)
			{
				window.setGameState(hexagonGame.getGame());
				hexagonGame.newGame(levelDataIds[currentIndex], true, difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]);
			}
			else if(state == s::OPTIONS) optionsMenu.executeCurrentItem(); 
		}, t::SINGLE);
		game.addInput({{k::F1}}, [&](float) { playSound("beep.ogg"); if(state == s::PROFILES) { profileNewName = ""; state = s::PROFILE_NEW; } }, t::SINGLE);
		game.addInput({{k::F2}, {k::J}}, [&](float) { playSound("beep.ogg"); if(state == s::MAIN ) { profileNewName = ""; state = s::PROFILES; } }, t::SINGLE);
		game.addInput({{k::F3}, {k::K}}, [&](float) { playSound("beep.ogg"); if(state == s::MAIN) state = s::OPTIONS; }, t::SINGLE);
		game.addInput({{k::F4}, {k::L}}, [&](float)
		{
			playSound("beep.ogg"); if(state == s::MAIN) { auto p(getPackPaths()); packIndex = (packIndex + 1) % p.size(); levelDataIds = getLevelIdsByPack(p[packIndex]); setIndex(0); }
		}, t::SINGLE);
		game.addInput({{k::Escape}}, [&](float) { playSound("beep.ogg"); if(state == s::OPTIONS) state = s::MAIN; }, t::SINGLE);
		game.addInput({{k::Escape}}, [&](float mFrameTime) { if(state != s::OPTIONS) exitTimer += mFrameTime; });
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

	string MenuGame::getLeaderboard()
	{
		float difficultyMult{difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]};
		string validator{Online::getValidator(levelData.getId(), levelData.getJsonRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
		Json::Value root{Online::getScores(validator)};

		using RecordPair = pair<string, float>;
		vector<RecordPair> recordPairs;
		for(auto itr(root.begin()); itr != root.end(); ++itr)
		{
			Json::Value& record(*itr);
			string name{record["n"].asString()};
			float score{record["s"].asFloat()};

			recordPairs.push_back({name, score});
		}

		sort(begin(recordPairs), end(recordPairs), [&](const RecordPair& mA, const RecordPair& mB){ return mA.second > mB.second; });

		string result{""};
		for(unsigned int i{0}; i < recordPairs.size(); ++i)
		{
			if(i > 4) break;
			auto& recordPair(recordPairs[i]);
			result.append("(" + toStr(i + 1) +") " + recordPair.first + ": " + toStr(recordPair.second) + "\n");
		}
		return result;
	}

	void MenuGame::update(float mFrameTime)
	{
		if(!window.isKeyPressed(Keyboard::Escape)) exitTimer = 0;
		if(exitTimer > 20) window.stop();

		if(inputDelay <= 0)
		{
			if(window.isKeyPressed(Keyboard::LAlt) && window.isKeyPressed(Keyboard::Return)) { setFullscreen(window, !window.getFullscreen()); inputDelay = 25; }
		}
		else inputDelay -= 1 * mFrameTime;

		if(state == States::PROFILE_NEW)
		{
			Event e; window.pollEvent(e);
			if(e.type == Event::TextEntered)
			{
				if(e.text.unicode > 47 && e.text.unicode < 126 && profileNewName.size() < 16) {char c{static_cast<char>(e.text.unicode)}; if(isalnum(c)) profileNewName.append(toStr(c)); }
				else if(e.text.unicode == 8 && !profileNewName.empty()) profileNewName.erase(profileNewName.end() - 1);
				else if(e.text.unicode == 13 && !profileNewName.empty()) { createProfile(profileNewName); setCurrentProfile(profileNewName); state = States::MAIN; inputDelay = 30; }
			}
		}
		else if(state == States::PROFILES) { profileNewName = getProfileNames()[profileIndex % getProfileNames().size()]; }
		else if(state == States::MAIN) { styleData.update(mFrameTime); backgroundCamera.rotate(levelData.getRotationSpeed() * 10 * mFrameTime); }
	}
	void MenuGame::draw()
	{
		window.clear(Color{0, 0, 0, 0});

		if(state == States::MAIN)
		{
			window.clear(styleData.getColors()[0]);
			backgroundCamera.apply(); styleData.drawBackground(window.getRenderWindow(), Vector2f{0,0}, 6);
			overlayCamera.apply(); drawLevelSelection(); render(bottomBar);
		}
		else if(state == States::PROFILE_NEW) 	{ window.clear(Color::Black); overlayCamera.apply(); drawProfileCreation(); }
		else if(state == States::PROFILES) 		{ window.clear(Color::Black); overlayCamera.apply(); drawProfileSelection(); }
		else if(state == States::OPTIONS) 		{ window.clear(Color::Black); overlayCamera.apply(); drawOptions(); }

		overlayCamera.apply(); render(titleBar); render(creditsBar1); render(creditsBar2); render(versionText);
	}

	void MenuGame::renderText(const string& mString, Text& mText, sf::Vector2f mPosition, unsigned int mSize)
	{
		unsigned int originalSize{mText.getCharacterSize()};
		if(mSize != 0) mText.setCharacterSize(mSize);

		mText.setString(mString);

		if(state != States::MAIN || getBlackAndWhite()) mText.setColor(Color::White);
		else mText.setColor(styleData.getMainColor());

		mText.setPosition(overlayCamera.getConvertedCoords(Vector2i(mPosition)).x, mPosition.y + 160);
		render(mText);

		mText.setCharacterSize(originalSize);
	}

	void MenuGame::drawLevelSelection()
	{
		MusicData musicData{getMusicData(levelData.getMusicId())};
		PackData packData{getPackData(levelData.getPackPath().substr(6, levelData.getPackPath().size() - 7))};
		string packName{packData.getName()}, packNames{""}; for(string packName : getPackNames()) packNames.append(packName + "\n"); // TODO!!!!

		if(getOnline())
		{
			string serverMessage{"connecting to server..."};
			float serverVersion{Online::getServerVersion()};
			if(serverVersion == getVersion()) serverMessage = "you have the latest version";
			if(serverVersion < getVersion()) serverMessage = "your version is newer (beta)";
			if(serverVersion > getVersion()) serverMessage = "update available (" + toStr(serverVersion) + ")";
			renderText(serverMessage, cProfText, {20, 0}, 13);

			if(!isEligibleForScore()) renderText("you are not eligible for scoring", cProfText, {20, 11}, 11);

			renderText("profile: " + getCurrentProfile().getName(), cProfText, {20, 10 + 5});
			renderText("pack: " + packName + " (" + toStr(packIndex + 1) + "/" + toStr(getPackPaths().size()) + ")", cProfText, {20, 30 + 5});
			renderText("local best: " + toStr(getScore(getScoreValidator(levelData.getId(), difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]))), cProfText, {20, 50 + 5});
			if(difficultyMultipliers.size() > 1) renderText("difficulty: " + toStr(difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]), cProfText, {20, 70 + 5});

			renderText(getLeaderboard(), cProfText, {20, 100});
		}
		else renderText("online disabled", cProfText, {20, 0}, 13);

		renderText(levelData.getName(), levelName, {20, 50 + 120 + 25});
		renderText(levelData.getDescription(), levelDesc, {20, 50 + 195 + 25 + 60.f * (countNewLines(levelData.getName()))});
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
		renderText(profileNewName, levelName, 							{20, 768 - 245 - 40});
	}
	void MenuGame::drawProfileSelection()
	{
		renderText("profile selection", cProfText, 						{20, 768 - 395});
		renderText("press left/right to browse profiles", cProfText, 	{20, 768 - 375});
		renderText("press enter to select profile", cProfText, 			{20, 768 - 355});
		renderText("press f1 to create a new profile", cProfText, 		{20, 768 - 335});
		renderText(profileNewName, levelName, 							{20, 768 - 245 - 40});
	}
	void MenuGame::drawOptions()
	{
		renderText(optionsMenu.getCurrentCategory().getName(), levelDesc, {20, 20});

		vector<ItemBase*>& currentItems(optionsMenu.getCurrentItems());
		for(int i{0}; i < static_cast<int>(currentItems.size()); ++i)
		{
			string name, itemName{currentItems[i]->getName()};
			if(i == optionsMenu.getCurrentIndex()) name.append(">> ");
			name.append(itemName);

			int extraSpacing{0};
			if(itemName == "back") extraSpacing = 20;
			renderText(name, cProfText, {20, 60.f + i * 20 + extraSpacing});
		}
	}

	void MenuGame::render(Drawable &mDrawable) { window.draw(mDrawable); }

	GameState& MenuGame::getGame() { return game; }
}
