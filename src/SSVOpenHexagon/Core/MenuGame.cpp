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
		auto& assetManager(getAssetManager());
		assetManager.get<Texture>("titleBar.png").setSmooth(true);
		assetManager.get<Texture>("creditsBar1.png").setSmooth(true);
		assetManager.get<Texture>("creditsBar2.png").setSmooth(true);
		assetManager.get<Texture>("creditsBar2b.png").setSmooth(true);
		assetManager.get<Texture>("creditsBar2c.png").setSmooth(true);
		assetManager.get<Texture>("creditsBar2d.png").setSmooth(true);
		assetManager.get<Texture>("bottomBar.png").setSmooth(true);

		refreshCamera();
	}
	void MenuGame::initOptionsMenu()
	{
		namespace i = ssvms::Items;
		auto& main(optionsMenu.createCategory("options"));
		auto& resolution(optionsMenu.createCategory("resolution"));
		auto& gfx(optionsMenu.createCategory("graphics"));
		auto& sfx(optionsMenu.createCategory("audio"));
		auto& play(optionsMenu.createCategory("gameplay"));
		auto& debug(optionsMenu.createCategory("debug"));
		auto& friends(optionsMenu.createCategory("friends"));

		main.create<i::Goto>("friends", friends);
		main.create<i::Single>("change profile", [&]{ enteredString = ""; state = States::PROFILES; });
		main.create<i::Single>("new profile", [&]{ enteredString = ""; state = States::PROFILE_NEW; });

		main.create<i::Goto>("gameplay", play);
		main.create<i::Goto>("resolution", resolution);
		main.create<i::Goto>("graphics", gfx);
		main.create<i::Goto>("audio", sfx);
		main.create<i::Goto>("debug", debug);
		main.create<i::Toggle>("online", [&]{ return getOnline(); }, [&]{ setOnline(true); }, [&]{ setOnline(false); });
		main.create<i::Toggle>("official mode", [&]{ return getOfficial(); }, [&]{ setOfficial(true); }, [&]{ setOfficial(false); });
		main.create<i::Single>("back", [&]{ state = States::MAIN; });

		resolution.create<i::Single>("auto", [&]{ setCurrentResolutionAuto(window); });
		for(const auto& vm : VideoMode::getFullscreenModes())
			if(vm.bitsPerPixel == 32)
				resolution.create<i::Single>(toStr(vm.width) + "x" + toStr(vm.height), [&]{ setCurrentResolution(window, vm.width, vm.height); refreshCamera(); });
		resolution.create<i::Single>("go windowed", 	[&]{ setFullscreen(window, false); });
		resolution.create<i::Single>("go fullscreen", 	[&]{ setFullscreen(window, true); });
		resolution.create<i::Goto>("back", main);


		gfx.create<i::Toggle>("rotation",	[&]{ return !getNoRotation(); }, 	[&]{ setNoRotation(false); }, 	[&]{ setNoRotation(true); });
		gfx.create<i::Toggle>("background",	[&]{ return !getNoBackground(); }, 	[&]{ setNoBackground(false); }, [&]{ setNoBackground(true); });
		gfx.create<i::Toggle>("b&w colors", [&]{ return getBlackAndWhite(); }, 	[&]{ setBlackAndWhite(true); }, [&]{ setBlackAndWhite(false); });
		gfx.create<i::Toggle>("3D effect",	[&]{ return get3D(); }, 			[&]{ set3D(true); }, 			[&]{ set3D(false); });
		gfx.create<i::Toggle>("pulse", 		[&]{ return getPulse(); }, 			[&]{ setPulse(true); }, 		[&]{ setPulse(false); });
		gfx.create<i::Toggle>("flash", 		[&]{ return getFlash(); }, 			[&]{ setFlash(true); }, 		[&]{ setFlash(false); });
		gfx.create<i::Goto>("back", main);

		sfx.create<i::Toggle>("sounds",	[&]{ return !getNoSound(); }, 	[&]{ setNoSound(false); }, 	[&]{ setNoSound(true); });
		sfx.create<i::Toggle>("music",	[&]{ return !getNoMusic(); },	[&]{ setNoMusic(false); }, 	[&]{ setNoMusic(true); });
		sfx.create<i::Slider>("sounds volume", [&]{ return toStr(getSoundVolume()); }, [&]{ setSoundVolume(getClamped(getSoundVolume() + 5, 0, 100)); refreshVolumes(); }, [&]{ setSoundVolume(getClamped(getSoundVolume() - 5, 0, 100)); refreshVolumes(); });
		sfx.create<i::Slider>("music volume", [&]{ return toStr(getMusicVolume()); }, [&]{ setMusicVolume(getClamped(getMusicVolume() + 5, 0, 100)); refreshVolumes(); }, [&]{ setMusicVolume(getClamped(getMusicVolume() - 5, 0, 100)); refreshVolumes(); });
		sfx.create<i::Goto>("back", main);

		play.create<i::Toggle>("autorestart", [&]{ return getAutoRestart(); }, [&]{ setAutoRestart(true); }, [&]{ setAutoRestart(false); });
		play.create<i::Goto>("back", main);

		debug.create<i::Toggle>("invincible", [&]{ return getInvincible(); }, [&]{ setInvincible(true); }, [&]{ setInvincible(false); });
		debug.create<i::Goto>("back", main);

		friends.create<i::Single>("add friend", [&]{ enteredString = ""; state = States::ADD_FRIEND; });
		friends.create<i::Single>("clear friends", [&]{ getCurrentProfile().clearTrackedNames(); });
		friends.create<i::Goto>("back", main);
	}
	void MenuGame::initInput()
	{
		using k = Keyboard::Key;
		using t = Trigger::Type;
		using s = States;
		game.addInput(getTriggerRotateCCW(), [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) 		{  --profileIndex; }
			else if(state == s::MAIN) 		{ setIndex(currentIndex - 1); }
			else if(state == s::OPTIONS) 	{ optionsMenu.decreaseCurrentItem(); }
		}, t::Single);
		game.addInput(getTriggerRotateCW(), [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) 		{ ++profileIndex; }
			else if(state == s::MAIN) 		{ setIndex(currentIndex + 1); }
			else if(state == s::OPTIONS) 	{ optionsMenu.increaseCurrentItem(); }
		}, t::Single);
		game.addInput({{k::Up}, {k::W}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::MAIN) 			{ ++difficultyMultIndex; refreshScores(); }
			else if(state == s::OPTIONS) 	{ optionsMenu.selectPreviousItem(); }
		}, t::Single);
		game.addInput({{k::Down}, {k::S}}, [&](float)
		{
			playSound("beep.ogg");
			if(state == s::MAIN) 			{ --difficultyMultIndex; refreshScores(); }
			else if(state == s::OPTIONS)	{ optionsMenu.selectNextItem(); }
		}, t::Single);
		game.addInput(getTriggerRestart(), [&](float)
		{
			playSound("beep.ogg");
			if(state == s::PROFILES) { setCurrentProfile(enteredString); state = s::MAIN; refreshScores(); }
			else if(state == s::MAIN)
			{
				window.setGameState(hexagonGame.getGame());
				hexagonGame.newGame(levelDataIds[currentIndex], true, difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]);
			}
			else if(state == s::OPTIONS) optionsMenu.executeCurrentItem();
			else if(state == s::PROFILE_NEW) { if(!enteredString.empty()) { createProfile(enteredString); setCurrentProfile(enteredString); state = s::MAIN; refreshScores(); enteredString = ""; } }
			else if(state == s::ADD_FRIEND) { if(!enteredString.empty() && !contains(getCurrentProfile().getTrackedNames(), enteredString)) { getCurrentProfile().addTrackedName(enteredString); state = s::MAIN; refreshScores(); enteredString = ""; } }
		}, t::Single);
		game.addInput({{k::F1}}, [&](float) { playSound("beep.ogg"); if(state == s::PROFILES) { enteredString = ""; state = s::PROFILE_NEW; } }, t::Single);
		game.addInput({{k::F2}, {k::J}}, [&](float) { playSound("beep.ogg"); if(state == s::MAIN ) { enteredString = ""; state = s::PROFILES; } }, t::Single);
		game.addInput({{k::F3}, {k::K}}, [&](float) { playSound("beep.ogg"); if(state == s::MAIN) state = s::OPTIONS; }, t::Single);
		game.addInput({{k::F4}, {k::L}}, [&](float)
		{
			playSound("beep.ogg"); if(state == s::MAIN) { auto p(getPackPaths()); packIndex = (packIndex + 1) % p.size(); levelDataIds = getLevelIdsByPack(p[packIndex]); setIndex(0); }
		}, t::Single);
		game.addInput(getTriggerExit(), [&](float) { playSound("beep.ogg"); if(state == s::OPTIONS) state = s::MAIN; refreshScores(); }, t::Single);
		game.addInput(getTriggerExit(), [&](float mFrameTime) { if(state != s::OPTIONS) exitTimer += mFrameTime; });
		game.addInput(getTriggerExit(), [&](float) { if(state == s::ADD_FRIEND) state = s::MAIN; });
		game.addInput(getTriggerScreenshot(), [&](float){ mustTakeScreenshot = true; }, t::Single);
		game.addInput({{k::LAlt, k::Return}}, [&](float){ setFullscreen(window, !window.getFullscreen()); refreshCamera(); }, t::Single);
		game.addInput({{k::BackSpace}}, [&](float){ if((state == s::PROFILE_NEW || state == s::ADD_FRIEND) && !enteredString.empty()) enteredString.erase(enteredString.end() - 1); }, t::Single);
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

		if(Online::isOverloaded()) { wasOverloaded = true; return; }
		Online::startGetScores(currentLeaderboard, currentPlayerScore, getCurrentProfile().getName(), friendsScores, getCurrentProfile().getTrackedNames(), validator, difficultyMult);
	}
	void MenuGame::updateLeaderboard()
	{
		if(currentLeaderboard == "NULL") { leaderboardString = "no scores"; return; }
		if(currentLeaderboard == "" || currentPlayerScore == "") { leaderboardString = "refreshing..."; return; }

		unsigned int leaderboardRecordCount{8};
		ssvuj::Value root{getRootFromString(currentLeaderboard)};

		using RecordPair = pair<string, float>;
		vector<RecordPair> recordPairs;

		int playerPosition{-1};

		for(auto itr(begin(root)); itr != end(root); ++itr)
		{
			ssvuj::Value& record(*itr);
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
				ssvuj::Value playerScoreRoot{getRootFromString(currentPlayerScore)};
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

	void MenuGame::updateFriends()
	{
		if(state != States::MAIN) return;
		if(getCurrentProfile().getTrackedNames().empty()) { friendsString = "you have no friends! :(\nadd them in the options menu"; return; }
		if(friendsScores.empty())
		{
			friendsString = "";
			for(const auto& n : getCurrentProfile().getTrackedNames()) friendsString.append("(?)" + n + "\n");
			return;
		}
		using ScoreTuple = tuple<int, string, float>;
		vector<ScoreTuple> tuples;
		for(const auto& s : friendsScores)
		{
			if(s == "NULL") continue;
			auto value(getRootFromString(s));
			tuples.emplace_back(as<int>(value, "p"), as<string>(value, "n"), as<float>(value, "s"));
		}
		sort(begin(tuples), end(tuples), [&](const ScoreTuple& mA, const ScoreTuple& mB){ return get<0>(mA) < get<0>(mB); });
		friendsString.clear();
		for(const auto& t : tuples) friendsString.append("(" + toStr(get<0>(t)) + ") " + get<1>(t) + ": " + toStr(get<2>(t)) + "\n");
	}

	template<typename T> float getGlobalLeft(const T& mElement) { return mElement.getGlobalBounds().left; }
	template<typename T> float getGlobalRight(const T& mElement) { return mElement.getGlobalBounds().left + mElement.getGlobalBounds().width; }
	template<typename T> float getGlobalTop(const T& mElement) { return mElement.getGlobalBounds().top; }
	template<typename T> float getGlobalBottom(const T& mElement) { return mElement.getGlobalBounds().top + mElement.getGlobalBounds().height; }
	template<typename T> float getGlobalWidth(const T& mElement) { return mElement.getGlobalBounds().width; }
	template<typename T> float getGlobalHeight(const T& mElement) { return mElement.getGlobalBounds().height; }

	template<typename T> float getLocalLeft(const T& mElement) { return mElement.getLocalBounds().left; }
	template<typename T> float getLocalRight(const T& mElement) { return mElement.getLocalBounds().left + mElement.getLocalBounds().width; }
	template<typename T> float getLocalTop(const T& mElement) { return mElement.getLocalBounds().top; }
	template<typename T> float getLocalBottom(const T& mElement) { return mElement.getLocalBounds().top + mElement.getLocalBounds().height; }
	template<typename T> float getLocalWidth(const T& mElement) { return mElement.getLocalBounds().width; }
	template<typename T> float getLocalHeight(const T& mElement) { return mElement.getLocalBounds().height; }

	void MenuGame::refreshCamera()
	{
		float fw = (1024.f / getWidth());
		float fh = (768.f / getHeight());
		float fmin = max(fw, fh);
		float w = getWidth() * fmin;
		float h = getHeight() * fmin;
		overlayCamera.setView(View{FloatRect(0, 0, w, h)});
		titleBar.setOrigin({0, 0});
		titleBar.setScale({0.5f, 0.5f});
		titleBar.setPosition({20.f, 20.f});

		versionText.setString(toStr(getVersion()));
		versionText.setColor(Color::White);
		versionText.setOrigin({getLocalRight(versionText), 0.f});
		versionText.setPosition({getGlobalRight(titleBar) - 15.f, getGlobalTop(titleBar) + 15.f});

		creditsBar1.setOrigin({getLocalWidth(creditsBar1), 0.f});
		creditsBar1.setScale({0.373f, 0.373f});
		creditsBar1.setPosition({w - 20.f, 20.f});

		creditsBar2.setOrigin({getLocalWidth(creditsBar2), 0});
		creditsBar2.setScale({0.373f, 0.373f});
		creditsBar2.setPosition({w - 20.f, 17.f + getGlobalBottom(creditsBar1)});

		float scaleFactor{w / 1024.f};
		bottomBar.setOrigin({0, 56.f});
		bottomBar.setScale({scaleFactor, scaleFactor});
		bottomBar.setPosition(Vec2f(0, h));
	}

	void MenuGame::update(float mFrameTime)
	{
		currentCreditsId += mFrameTime;
		creditsBar2.setTexture(getAssetManager().get<Texture>(creditsIds[static_cast<int>(currentCreditsId / 100) % creditsIds.size()]));

		if(wasOverloaded == true && Online::isFree()) { wasOverloaded = false; refreshScores(); }

		updateLeaderboard();
		updateFriends();

		if(!window.isKeyPressed(Keyboard::Escape)) exitTimer = 0;
		if(exitTimer > 20) window.stop();

		if(state == States::PROFILE_NEW || state == States::ADD_FRIEND) { for(const auto& c : enteredChars) if(enteredString.size() < 16 && isalnum(c)) { playSound("beep.ogg"); enteredString.append(toStr(c)); } }
		else if(state == States::PROFILES) { enteredString = getProfileNames()[profileIndex % getProfileNames().size()]; }
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
		else if(state == States::PROFILE_NEW || state == States::ADD_FRIEND) drawProfileCreation();
		else if(state == States::PROFILES) drawProfileSelection();
		else if(state == States::OPTIONS) drawOptions();

		render(titleBar); render(creditsBar1); render(creditsBar2); render(versionText);
		if(mustTakeScreenshot) { window.getRenderWindow().capture().saveToFile("screenshot.png"); mustTakeScreenshot = false; }
	}

	Text& MenuGame::renderText(const string& mString, Text& mText, Vec2f mPosition, unsigned int mSize)
	{
		unsigned int originalSize{mText.getCharacterSize()};
		if(mSize != 0) mText.setCharacterSize(mSize);

		if(mText.getString() != mString) mText.setString(mString);

		if(state != States::MAIN || getBlackAndWhite()) mText.setColor(Color::White);
		else mText.setColor(styleData.getMainColor());

		mText.setPosition(mPosition);
		render(mText); mText.setCharacterSize(originalSize);

		return mText;
	}

	void MenuGame::drawLevelSelection()
	{
		float fw = (1024.f / getWidth());
		float fh = (768.f / getHeight());
		float fmin = max(fw, fh);
		float w = getWidth() * fmin;
		float h = getHeight() * fmin;

		MusicData musicData{getMusicData(levelData.getMusicId())};
		PackData packData{getPackData(levelData.getPackPath().substr(6, levelData.getPackPath().size() - 7))};
		const string& packName{packData.getName()};

		if(getOnline())
		{
			string versionMessage{"connecting to server..."};
			float serverVersion{Online::getServerVersion()};

			if(serverVersion == -1) versionMessage = "error connecting to server";
			else if(serverVersion == getVersion()) versionMessage = "you have the latest version";
			else if(serverVersion < getVersion()) versionMessage = "your version is newer (beta)";
			else if(serverVersion > getVersion()) versionMessage = "update available (" + toStr(serverVersion) + ")";
			renderText(versionMessage, cProfText, {20, 0}, 13);


			Text& profile = renderText("profile: " + getCurrentProfile().getName(), cProfText, Vec2f{20.f, getGlobalBottom(titleBar)}, 18);
			Text& pack = renderText("pack: " + packName + " (" + toStr(packIndex + 1) + "/" + toStr(getPackPaths().size()) + ")", cProfText, {20.f, getGlobalBottom(profile) - 7.f}, 18);
			Text& lbest = renderText("local best: " + toStr(getScore(getLocalValidator(levelData.getId(), difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]))), cProfText, {20.f, getGlobalBottom(pack) - 7.f}, 18);
			if(difficultyMultipliers.size() > 1) renderText("difficulty: " + toStr(difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]), cProfText, {20.f, getGlobalBottom(lbest) - 7.f}, 18);

			if(wasOverloaded || Online::isOverloaded()) { leaderboardString = friendsString = "too many requests, wait..."; }

			renderText(leaderboardString, cProfText, {20.f, getGlobalBottom(lbest)}, 15);
			Text& smsg = renderText("server message: " + Online::getServerMessage(), levelAuth, {20.f, getGlobalTop(bottomBar) - 20.f}, 14);
			friendsText.setOrigin({getLocalWidth(friendsText), 0.f});
			renderText("friends:\n" + friendsString, friendsText, {w - 20.f, getGlobalBottom(titleBar)}, 18);
			//renderText("packs:\n" + packNames, packsText, {overlayCamera.getConvertedCoords(Vec2i(getWidth() - 20.f - friendsText.getGlobalBounds().width, getHeight() - 20.f - friendsText.getGlobalBounds().height))});

			if(!isEligibleForScore()) renderText("not eligible for scoring: " + getUneligibilityReason(), cProfText, {20.f, getGlobalTop(smsg) - 20.f}, 11);
		}
		else renderText("online disabled", cProfText, {20, 0}, 13);

		Text& lname = renderText(levelData.getName(), levelName, {20.f, h / 2.f});
		Text& ldesc = renderText(levelData.getDescription(), levelDesc, {20.f, getGlobalBottom(lname) - 5.f});
		Text& lauth = renderText("author: " + levelData.getAuthor(), levelAuth, {20.f, getGlobalBottom(ldesc) + 25.f});
		renderText("music: " + musicData.getName() + " by " + musicData.getAuthor() + " (" + musicData.getAlbum() + ")", levelMusc, {20.f, getGlobalBottom(lauth) - 5.f});
		renderText("(" + toStr(currentIndex + 1) + "/" + toStr(levelDataIds.size()) + ")", levelMusc, {20.f, getGlobalTop(lname) - 25.f});

		string packNames{"Installed packs:\n"};
		for(const auto& n : getPackNames()) { if(packData.getId() == n) packNames += ">>> "; packNames.append(n + "\n"); }
		packsText.setString(packNames);
		packsText.setOrigin(packsText.getGlobalBounds().width, packsText.getGlobalBounds().height);
		packsText.setPosition({w - 20.f, getGlobalTop(bottomBar) - 15.f});
		packsText.setColor(styleData.getMainColor());
		render(packsText);
	}
	void MenuGame::drawProfileCreation()
	{
		renderText(state == States::ADD_FRIEND ? "add friend" : "profile creation", cProfText, {20, 768 - 395});
		renderText("insert profile name", cProfText, 					{20, 768 - 375});
		renderText("press enter when done", cProfText, 					{20, 768 - 335});
		renderText("keep esc pressed to exit", cProfText, 				{20, 768 - 315});
		renderText(enteredString, levelName, 							{20, 768 - 245 - 40});
	}
	void MenuGame::drawProfileSelection()
	{
		renderText("profile selection", cProfText, 						{20, 768 - 395});
		renderText("press left/right to browse profiles", cProfText, 	{20, 768 - 375});
		renderText("press enter to select profile", cProfText, 			{20, 768 - 355});
		renderText("press f1 to create a new profile", cProfText, 		{20, 768 - 335});
		renderText(enteredString, levelName, 							{20, 768 - 245 - 40});
	}
	void MenuGame::drawOptions()
	{
		float fw = (1024.f / getWidth());
		float fh = (768.f / getHeight());
		float fmin = max(fw, fh);
		float w = getWidth() * fmin;
		float h = getHeight() * fmin;

		renderText(optionsMenu.getCurrentCategory().getName(), levelDesc, {20.f, getGlobalBottom(titleBar)});

		float currentX{0.f}, currentY{0.f};
		vector<ItemBase*>& currentItems(optionsMenu.getCurrentItems());
		for(int i{0}; i < static_cast<int>(currentItems.size()); ++i)
		{
			currentY += 19;
			if(i != 0 && i % 21 == 0) { currentY = 0; currentX += 180; }
			string name, itemName{currentItems[i]->getName()};
			if(i == optionsMenu.getCurrentIndex()) name.append(">> ");
			name.append(itemName);

			int extraSpacing{0};
			if(itemName == "back") extraSpacing = 20;
			renderText(name, cProfText, {20.f + currentX, getGlobalBottom(titleBar) + 20.f + currentY + extraSpacing});
		}

		if(getOfficial()) renderText("official mode on - some options cannot be changed", cProfText, {20, h - 30.f});
		else if(getOfficial()) renderText("official mode off - your scores won't be sent to the server", cProfText, {20, h - 30.f});
	}

	void MenuGame::render(Drawable &mDrawable) { window.draw(mDrawable); }

	GameState& MenuGame::getGame() { return game; }
}
