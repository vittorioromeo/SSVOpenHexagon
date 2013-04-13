// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <string>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Core/MenuGame.h"
#include "SSVOpenHexagon/Online/Online.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Input;
using namespace sses;
using namespace ssvms;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{
	MenuGame::MenuGame(HexagonGame& mHexagonGame, GameWindow& mGameWindow) : hexagonGame(mHexagonGame), window(mGameWindow)
	{
		initAssets();

		game.onUpdate += [&](float mFrameTime) { update(mFrameTime); };
		game.onDraw += [&]{ draw(); };

		game.getEventDelegate(Event::EventType::TextEntered) += [&](const Event& mEvent){ enteredChars.push_back(mEvent.text.unicode); };

		levelDataIds = getLevelIdsByPack(getPackPaths()[packIndex]);
		setIndex(0);

		if(getProfilesSize() == 0) state = States::PROFILE_NEW;
		else if(getProfilesSize() == 1) { setCurrentProfile(getFirstProfileName()); state = States::MAIN; }

		initOptionsMenu(); initInput();
	}

	void MenuGame::init() { stopAllMusic(); stopAllSounds(); playSound("openHexagon.ogg"); refreshScores(); }
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
		versionText.setPosition(titleBar.getPosition() + Vector2f{titleBar.getGlobalBounds().width - 83, titleBar.getGlobalBounds().top});

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
		main.create<i::Toggle>("official mode", [&]{ return getOfficial(); }, [&]{ setOfficial(true); }, [&]{ setOfficial(false); });
		main.create<i::Single>("back", [&]{ state = States::MAIN; });

		gfx.create<i::Toggle>("rotation",	[&]{ return !getNoRotation(); }, 	[&]{ setNoRotation(false); }, 	[&]{ setNoRotation(true); });
		gfx.create<i::Toggle>("background",	[&]{ return !getNoBackground(); }, 	[&]{ setNoBackground(false); }, [&]{ setNoBackground(true); });
		gfx.create<i::Toggle>("b&w colors", [&]{ return getBlackAndWhite(); }, 	[&]{ setBlackAndWhite(true); }, [&]{ setBlackAndWhite(false); });
		gfx.create<i::Toggle>("3D effect",	[&]{ return get3D(); }, 			[&]{ set3D(true); }, 			[&]{ set3D(false); });
		gfx.create<i::Toggle>("pulse", 		[&]{ return getPulse(); }, 			[&]{ setPulse(true); }, 		[&]{ setPulse(false); });
		gfx.create<i::Toggle>("flash", 		[&]{ return getFlash(); }, 			[&]{ setFlash(true); }, 		[&]{ setFlash(false); });
		gfx.create<i::Single>("go windowed", 	[&]{ setFullscreen(window, false); });
		gfx.create<i::Single>("go fullscreen", 	[&]{ setFullscreen(window, true); });
		gfx.create<i::Goto>("back", main);

		sfx.create<i::Toggle>("sounds",	[&]{ return !getNoSound(); }, 	[&]{ setNoSound(false); }, 	[&]{ setNoSound(true); });
		sfx.create<i::Toggle>("music",	[&]{ return !getNoMusic(); },	[&]{ setNoMusic(false); }, 	[&]{ setNoMusic(true); });
		sfx.create<i::Slider>("sounds volume", [&]{ return toStr(getSoundVolume()); },
		[&]{ setSoundVolume(getClamped(getSoundVolume() + 5, 0, 100)); refreshVolumes(); }, [&]{ setSoundVolume(getClamped(getSoundVolume() - 5, 0, 100)); refreshVolumes(); });
		sfx.create<i::Slider>("music volume", [&]{ return toStr(getMusicVolume()); },
		[&]{ setMusicVolume(getClamped(getMusicVolume() + 5, 0, 100)); refreshVolumes(); }, [&]{ setMusicVolume(getClamped(getMusicVolume() - 5, 0, 100)); refreshVolumes(); });
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
		game.addInput({{k::Left}, {k::A}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) 		{  --profileIndex; }
			else if(state == s::MAIN) 		{ setIndex(currentIndex - 1); }
			else if(state == s::OPTIONS) 	{ optionsMenu.decreaseCurrentItem(); }
		}, t::SINGLE);
		game.addInput({{k::Right}, {k::D}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) 		{ ++profileIndex; }
			else if(state == s::MAIN) 		{ setIndex(currentIndex + 1); }
			else if(state == s::OPTIONS) 	{ optionsMenu.increaseCurrentItem(); }
		}, t::SINGLE);
		game.addInput({{k::Up}, {k::W}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::MAIN) 			{ ++difficultyMultIndex; refreshScores(); }
			else if(state == s::OPTIONS) 	{ optionsMenu.selectPreviousItem(); }
		}, t::SINGLE);
		game.addInput({{k::Down}, {k::S}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::MAIN) 			{ --difficultyMultIndex; refreshScores(); }
			else if(state == s::OPTIONS)	{ optionsMenu.selectNextItem(); }
		}, t::SINGLE);
		game.addInput({{k::Return}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) { setCurrentProfile(profileNewName); state = s::MAIN; refreshScores(); }
			else if(state == s::MAIN)
			{
				window.setGameState(hexagonGame.getGame());
				hexagonGame.newGame(levelDataIds[currentIndex], true, difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]);
			}
			else if(state == s::OPTIONS) optionsMenu.executeCurrentItem();
			else if(state == s::PROFILE_NEW) if(!profileNewName.empty()) { createProfile(profileNewName); setCurrentProfile(profileNewName); state = s::MAIN; refreshScores(); }
		}, t::SINGLE);
		game.addInput({{k::F1}}, [&](float) { playSound("beep.ogg"); if(state == s::PROFILES) { profileNewName = ""; state = s::PROFILE_NEW; } }, t::SINGLE);
		game.addInput({{k::F2}, {k::J}}, [&](float) { playSound("beep.ogg"); if(state == s::MAIN ) { profileNewName = ""; state = s::PROFILES; } }, t::SINGLE);
		game.addInput({{k::F3}, {k::K}}, [&](float) { playSound("beep.ogg"); if(state == s::MAIN) state = s::OPTIONS; }, t::SINGLE);
		game.addInput({{k::F4}, {k::L}}, [&](float)
		{
			playSound("beep.ogg"); if(state == s::MAIN) { auto p(getPackPaths()); packIndex = (packIndex + 1) % p.size(); levelDataIds = getLevelIdsByPack(p[packIndex]); setIndex(0); }
		}, t::SINGLE);
		game.addInput({{k::Escape}}, [&](float) { playSound("beep.ogg"); if(state == s::OPTIONS) state = s::MAIN; refreshScores(); }, t::SINGLE);
		game.addInput({{k::Escape}}, [&](float mFrameTime) { if(state != s::OPTIONS) exitTimer += mFrameTime; });
		game.addInput({{k::F12}}, [&](float){ mustTakeScreenshot = true; }, t::SINGLE);
		game.addInput({{k::LAlt, k::Return}}, [&](float){ setFullscreen(window, !window.getFullscreen()); }, t::SINGLE);
		game.addInput({{k::BackSpace}}, [&](float){ if(state == s::PROFILE_NEW && !profileNewName.empty()) profileNewName.erase(profileNewName.end() - 1); }, t::SINGLE);
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

		refreshScores();
	}

	void MenuGame::refreshScores()
	{
		if(state != States::MAIN) return;
		float difficultyMult{difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]};
		string validator{Online::getValidator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath())};
		Online::startGetScores(currentLeaderboard, currentPlayerScore, getCurrentProfile().getName(), validator, difficultyMult);
	}
	void MenuGame::updateLeaderboard()
	{
		if(currentLeaderboard == "NULL") { leaderboardString = "no scores"; return; }
		if(currentLeaderboard == "" || currentPlayerScore == "") { leaderboardString = "refreshing..."; return; }

		unsigned int leaderboardRecordCount{8};
		Json::Value root{getRootFromString(currentLeaderboard)};

		using RecordPair = pair<string, float>;
		vector<RecordPair> recordPairs;

		int playerPosition{-1};

		for(auto itr(root.begin()); itr != root.end(); ++itr)
		{
			Json::Value& record(*itr);
			string name{toLower(as<string>(record, "n"))};
			float score{as<float>(record, "s")};
			recordPairs.push_back({name, score});
		}

		sort(begin(recordPairs), end(recordPairs), [&](const RecordPair& mA, const RecordPair& mB){ return mA.second > mB.second; });

		bool foundPlayer{false};
		for(unsigned int i{0}; i < recordPairs.size(); ++i)
		{
			if(recordPairs[i].first != getCurrentProfile().getName()) continue;
			playerPosition = i + 1;
			foundPlayer = true;
			break;
		}

		string result{""};
		for(unsigned int i{0}; i < recordPairs.size(); ++i)
		{
			if(currentPlayerScore != "NULL" && currentPlayerScore != "" && !foundPlayer && i == leaderboardRecordCount -1)
			{
				Json::Value playerScoreRoot{getRootFromString(currentPlayerScore)};
				result.append("...(" + toStr(as<int>(playerScoreRoot, "p")) + ") " + getCurrentProfile().getName() + ": " + toStr(as<float>(playerScoreRoot, "s")) + "\n");
				break;
			}

			if(i <= leaderboardRecordCount)
			{
				if(playerPosition == -1 || i < leaderboardRecordCount)
				{
					auto& recordPair(recordPairs[i]);
					if(recordPair.first == getCurrentProfile().getName()) result.append(" >> ");
					result.append("(" + toStr(i + 1) +") " + recordPair.first + ": " + toStr(recordPair.second) + "\n");
				}
			}
			else break;
		}

		leaderboardString = result;
	}

	void MenuGame::update(float mFrameTime)
	{
		updateLeaderboard();

		if(!window.isKeyPressed(Keyboard::Escape)) exitTimer = 0;
		if(exitTimer > 20) window.stop();

		if(state == States::PROFILE_NEW) { for(auto& c : enteredChars) if(profileNewName.size() < 16 && isalnum(c)) { playSound("beep.ogg"); profileNewName.append(toStr(c)); } }
		else if(state == States::PROFILES) { profileNewName = getProfileNames()[profileIndex % getProfileNames().size()]; }
		else if(state == States::MAIN) { styleData.update(mFrameTime); backgroundCamera.rotate(levelData.getRotationSpeed() * 10 * mFrameTime); }

		enteredChars.clear();
	}
	void MenuGame::draw()
	{
		styleData.computeColors();
		window.clear(state != States::MAIN ? Color::Black : styleData.getColors()[0]);

		backgroundCamera.apply();
		if(state == States::MAIN) styleData.drawBackground(window.getRenderWindow(), {0, 0}, levelData.getSides());

		overlayCamera.apply();
		if(state == States::MAIN) { drawLevelSelection(); render(bottomBar); }
		else if(state == States::PROFILE_NEW) drawProfileCreation();
		else if(state == States::PROFILES) drawProfileSelection();
		else if(state == States::OPTIONS) drawOptions();

		render(titleBar); render(creditsBar1); render(creditsBar2); render(versionText);
		if(mustTakeScreenshot) { window.getRenderWindow().capture().saveToFile("screenshot.png"); mustTakeScreenshot = false; }
	}

	void MenuGame::renderText(const string& mString, Text& mText, sf::Vector2f mPosition, unsigned int mSize)
	{
		unsigned int originalSize{mText.getCharacterSize()};
		if(mSize != 0) mText.setCharacterSize(mSize);

		mText.setString(mString);

		if(state != States::MAIN || getBlackAndWhite()) mText.setColor(Color::White);
		else mText.setColor(styleData.getMainColor());

		mText.setPosition(overlayCamera.getConvertedCoords(Vector2i(mPosition)).x, mPosition.y + 160);
		render(mText); mText.setCharacterSize(originalSize);
	}

	void MenuGame::drawLevelSelection()
	{
		MusicData musicData{getMusicData(levelData.getMusicId())};
		PackData packData{getPackData(levelData.getPackPath().substr(6, levelData.getPackPath().size() - 7))};
		string packName{packData.getName()}; //, packNames{""}; for(string packName : getPackNames()) packNames.append(packName + "\n"); // TODO!!!!

		if(getOnline())
		{
			string versionMessage{"connecting to server..."};
			float serverVersion{Online::getServerVersion()};

			if(serverVersion == -1) versionMessage = "error connecting to server";
			else if(serverVersion == getVersion()) versionMessage = "you have the latest version";
			else if(serverVersion < getVersion()) versionMessage = "your version is newer (beta)";
			else if(serverVersion > getVersion()) versionMessage = "update available (" + toStr(serverVersion) + ")";
			renderText(versionMessage, cProfText, {20, 0}, 13);

			if(!isEligibleForScore()) renderText("not eligible for scoring: " + getUneligibilityReason(), cProfText, {20, 11}, 11);

			renderText("profile: " + getCurrentProfile().getName(), cProfText, {20, 10 + 5 + 3});
			renderText("pack: " + packName + " (" + toStr(packIndex + 1) + "/" + toStr(getPackPaths().size()) + ")", cProfText, {20, 30 + 3});
			renderText("local best: " + toStr(getScore(getLocalValidator(levelData.getId(), difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]))), cProfText, {20, 45 + 3});
			if(difficultyMultipliers.size() > 1) renderText("difficulty: " + toStr(difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]), cProfText, {20, 60 + 3});

			renderText(leaderboardString, cProfText, {20, 100}, 20);
			renderText("server message: " + Online::getServerMessage(), levelAuth, {20, -30 + 525}, 13);
		}
		else renderText("online disabled", cProfText, {20, 0}, 13);

		renderText(levelData.getName(), levelName, {20, 50 + 120 + 25 + 45});
		renderText(levelData.getDescription(), levelDesc, {20, 50 + 195 + 25 + 28 + 60.f * (getNewLinesCount(levelData.getName()))});
		renderText("author: " + levelData.getAuthor(), levelAuth, {20, -30 + 500 - 35});
		renderText("music: " + musicData.getName() + " by " + musicData.getAuthor() + " (" + musicData.getAlbum() + ")", levelMusc, {20, -30 + 515 - 35});
		renderText("(" + toStr(currentIndex + 1) + "/" + toStr(levelDataIds.size()) + ")", levelMusc, {20, -30 + 530 - 35});
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

		if(getOfficial()) renderText("official mode on - some options cannot be changed", cProfText, {20, 500});
		else if(getOfficial()) renderText("official mode off - your scores won't be sent to the server", cProfText, {20, 500});
	}

	void MenuGame::render(Drawable &mDrawable) { window.draw(mDrawable); }

	GameState& MenuGame::getGame() { return game; }
}
