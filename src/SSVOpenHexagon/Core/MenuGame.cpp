// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

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
	MenuGame::MenuGame(HGAssets& mAssets, HexagonGame& mHexagonGame, GameWindow& mGameWindow) : assets(mAssets), hexagonGame(mHexagonGame), window(mGameWindow)
	{
		refreshCamera();
		initAssets();

		game.onUpdate += [&](float mFrameTime) { update(mFrameTime); };
		game.onDraw += [&]{ draw(); };
		game.getEventDelegate(Event::EventType::TextEntered) += [&](const Event& mEvent){ enteredChars.push_back(mEvent.text.unicode); };

		levelDataIds = assets.getLevelIdsByPack(assets.getPackPaths()[packIndex]);
		setIndex(0);

		initMenus(); initInput();
	}

	void MenuGame::init() { assets.stopMusics(); assets.stopSounds(); assets.playSound("openHexagon.ogg"); }
	void MenuGame::initAssets()
	{
		for(const auto& t : {"titleBar.png", "creditsBar1.png", "creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png", "creditsBar2d.png", "bottomBar.png"})
			assets().get<Texture>(t).setSmooth(true);

		refreshCamera();
	}

	void operator|(ItemBase& mLhs, const pair<MenuController&, MenuController::Predicate>& mRhs) { mRhs.first.enableItemIf(mLhs, mRhs.second); }

	void MenuGame::initMenus()
	{
		auto onlyIf = [&](MenuController::Predicate mPred){ return pair<MenuController&, MenuController::Predicate>(menuController, mPred); };
		auto onlyIfPlayingLocally =			onlyIf([&]{ return assets.pIsPlayingLocally(); });
		auto onlyIfNotPlayingLocally =		onlyIf([&]{ return !assets.pIsPlayingLocally(); });
		auto onlyIfNotOfficial =			onlyIf([&]{ return Config::getOfficial(); });
		auto onlyIfDisconnected =			onlyIf([&]{ return Online::getConnectionStatus() == Online::ConnectStat::Disconnected; });
		auto onlyIfConnectedAndUnlogged =	onlyIf([&]{ return Online::getConnectionStatus() == Online::ConnectStat::Connected && Online::getLoginStatus() == Online::LoginStat::Unlogged; });
		auto onlyIfConnectedAndLogged =		onlyIf([&]{ return Online::getConnectionStatus() == Online::ConnectStat::Connected && Online::getLoginStatus() == Online::LoginStat::Logged; });
		auto onlyIfUnlogged =				onlyIf([&]{ return Online::getLoginStatus() == Online::LoginStat::Unlogged; });
		auto onlyIfSoundEnabled =			onlyIf([&]{ return !Config::getNoSound(); });
		auto onlyIfMusicEnabled =			onlyIf([&]{ return !Config::getNoMusic(); });

		namespace i = ssvms::Items;

		auto& wlcm(welcomeMenu.createCategory("welcome"));
		wlcm.create<i::Single>("connect",		[&]{ Online::tryConnectToServer(); }) | onlyIfDisconnected;
		wlcm.create<i::Single>("login",			[&]{ assets.pSaveCurrent(); assets.pSetPlayingLocally(false); enteredString = ""; state = States::LRUser; }) | onlyIfConnectedAndUnlogged;
		wlcm.create<i::Single>("logout",		[&]{ Online::logout(); }) | onlyIfConnectedAndLogged;
		wlcm.create<i::Single>("play locally",	[&]{ assets.pSaveCurrent(); assets.pSetPlayingLocally(true); enteredString = ""; state = States::LocalProfileSelect; }) | onlyIfUnlogged;
		wlcm.create<i::Single>("exit game",		[&]{ window.stop(); });

		auto& main(optionsMenu.createCategory("options"));
		auto& resolution(optionsMenu.createCategory("resolution"));
		auto& gfx(optionsMenu.createCategory("graphics"));
		auto& sfx(optionsMenu.createCategory("audio"));
		auto& play(optionsMenu.createCategory("gameplay"));
		auto& debug(optionsMenu.createCategory("debug"));
		auto& friends(optionsMenu.createCategory("friends"));

		main.create<i::Goto>("friends", friends) | onlyIfNotPlayingLocally;
		main.create<i::Single>("change local profile", [&]{ enteredString = ""; state = States::LocalProfileSelect; }) | onlyIfPlayingLocally;
		main.create<i::Single>("new local profile", [&]{ enteredString = ""; state = States::LocalProfileNew; }) | onlyIfPlayingLocally;
		main.create<i::Single>("login screen", [&]{ state = States::Welcome; });
		main.create<i::Goto>("gameplay", play);
		main.create<i::Goto>("resolution", resolution);
		main.create<i::Goto>("graphics", gfx);
		main.create<i::Goto>("audio", sfx);
		main.create<i::Goto>("debug", debug) | onlyIfNotOfficial;
		main.create<i::Toggle>("online", &Config::getOnline, &Config::setOnline);
		main.create<i::Toggle>("official mode", &Config::getOfficial, &Config::setOfficial);
		main.create<i::Single>("exit game", [&]{ window.stop(); });
		main.create<i::Single>("back", [&]{ state = States::Main; });

		resolution.create<i::Single>("auto", [&]{ Config::setCurrentResolutionAuto(window); });
		for(const auto& vm : VideoMode::getFullscreenModes()) if(vm.bitsPerPixel == 32) resolution.create<i::Single>(toStr(vm.width) + "x" + toStr(vm.height), [&]{ Config::setCurrentResolution(window, vm.width, vm.height); refreshCamera(); });
		resolution.create<i::Single>("go windowed", 	[&]{ Config::setFullscreen(window, false); });
		resolution.create<i::Single>("go fullscreen", 	[&]{ Config::setFullscreen(window, true); });
		resolution.create<i::GoBack>("back");

		gfx.create<i::Toggle>("3D effects", &Config::get3D, &Config::set3D);
		gfx.create<i::Toggle>("3D effects", &Config::getNoRotation, &Config::setNoRotation) | onlyIfNotOfficial;
		gfx.create<i::Toggle>("no background", &Config::getNoBackground, &Config::setNoBackground) | onlyIfNotOfficial;
		gfx.create<i::Toggle>("b&w colors", &Config::getBlackAndWhite, &Config::setBlackAndWhite) | onlyIfNotOfficial;
		gfx.create<i::Toggle>("pulse", &Config::getPulse, &Config::setPulse) | onlyIfNotOfficial;

		gfx.create<i::Toggle>("flash", &Config::getFlash, &Config::setFlash);
		gfx.create<i::Toggle>("vsync", &Config::getVsync, [&](bool mValue){ Config::setVsync(window, mValue); });
		gfx.create<i::Single>("go windowed", 	[&]{ Config::setFullscreen(window, false); });
		gfx.create<i::Single>("go fullscreen", 	[&]{ Config::setFullscreen(window, true); });
		gfx.create<i::Toggle>("limit fps", &Config::getLimitFPS, [&](bool mValue){ Config::setLimitFPS(mValue); refreshFPS(); });
		gfx.create<i::Slider>("max fps", &Config::getMaxFPS, [&](unsigned int mValue){ Config::setMaxFPS(mValue); refreshFPS(); }, 30u, 120u, 5u);
		gfx.create<i::Toggle>("show fps", &Config::getShowFPS, &Config::setShowFPS);
		gfx.create<i::GoBack>("back");

		sfx.create<i::Toggle>("no sound", &Config::getNoSound, &Config::setNoSound);
		sfx.create<i::Toggle>("no music", &Config::getNoMusic, &Config::setNoMusic);
		sfx.create<i::Slider>("sound volume", &Config::getSoundVolume, [&](unsigned int mValue){ Config::setSoundVolume(mValue); assets.refreshVolumes(); }, 0u, 100u, 5u) | onlyIfSoundEnabled;
		sfx.create<i::Slider>("music volume", &Config::getMusicVolume, [&](unsigned int mValue){ Config::setMusicVolume(mValue); assets.refreshVolumes(); }, 0u, 100u, 5u) | onlyIfMusicEnabled;
		sfx.create<i::Toggle>("sync music with difficulty", &Config::getMusicSpeedDMSync, &Config::setMusicSpeedDMSync) | onlyIfMusicEnabled;
		sfx.create<i::GoBack>("back");

		play.create<i::Toggle>("autorestart", &Config::getAutoRestart, &Config::setAutoRestart);
		play.create<i::GoBack>("back");

		debug.create<i::Toggle>("invincible", &Config::getInvincible, &Config::setInvincible);
		debug.create<i::GoBack>("back");

		friends.create<i::Single>("add friend", [&]{ enteredString = ""; state = States::FriendAdd; });
		friends.create<i::Single>("clear friends", [&]{ assets.pClearTrackedNames(); });
		friends.create<i::GoBack>("back");
	}
	void MenuGame::initInput()
	{
		using k = Keyboard::Key;
		using t = Trigger::Type;
		using s = States;
		game.addInput(Config::getTriggerRotateCCW(), [&](float)
		{
			assets.playSound("beep.ogg");
			if(state == s::LocalProfileSelect) 	{  --profileIndex; }
			else if(state == s::Main)			{ setIndex(currentIndex - 1); }
			else if(isInMenu())					{ getCurrentMenu()->decrease(); }
		}, t::Single);
		game.addInput(Config::getTriggerRotateCW(), [&](float)
		{
			assets.playSound("beep.ogg");
			if(state == s::LocalProfileSelect) 	{ ++profileIndex; }
			else if(state == s::Main)			{ setIndex(currentIndex + 1); }
			else if(isInMenu())					{ getCurrentMenu()->increase(); }
		}, t::Single);
		game.addInput({{k::Up}, {k::W}}, [&](float)
		{
			assets.playSound("beep.ogg");
			if(state == s::Main)				{ ++difficultyMultIndex; }
			else if(isInMenu())					{ getCurrentMenu()->previous(); }
		}, t::Single);
		game.addInput({{k::Down}, {k::S}}, [&](float)
		{
			assets.playSound("beep.ogg");
			if(state == s::Main)				{ --difficultyMultIndex; }
			else if(isInMenu())					{ getCurrentMenu()->next(); }
		}, t::Single);
		game.addInput(Config::getTriggerRestart(), [&](float)
		{
			assets.playSound("beep.ogg");
			if(state == s::LocalProfileSelect) { assets.pSetCurrent(enteredString); state = s::Main; }
			else if(state == s::Main)
			{
				window.setGameState(hexagonGame.getGame());
				hexagonGame.newGame(levelDataIds[currentIndex], true, difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]);
			}
			else if(isInMenu())						{ getCurrentMenu()->exec(); }
			else if(state == s::LocalProfileNew)	{ if(!enteredString.empty()) { assets.pCreate(enteredString); assets.pSetCurrent(enteredString); state = s::Main; enteredString = ""; } }
			else if(state == s::FriendAdd)			{ if(!enteredString.empty() && !contains(assets.pGetTrackedNames(), enteredString)) { assets.pAddTrackedName(enteredString); state = s::Main; enteredString = ""; } }
			else if(state == s::LRUser)				{ if(!enteredString.empty()) { lrUser = enteredString; state = s::LRPass; enteredString = ""; } }
			else if(state == s::LRPass)				{ if(!enteredString.empty()) { lrPass = enteredString; state = s::Logging; enteredString = ""; Online::tryLogin(lrUser, lrPass); } }
			else if(state == s::LREmail)			{ if(!enteredString.empty()) { lrEmail = enteredString; enteredString = ""; Online::trySendUserEmail(lrEmail); } }
		}, t::Single);
		game.addInput({{k::F1}}, [&](float)			{ assets.playSound("beep.ogg"); if(!assets.pIsPlayingLocally()) { state = s::Welcome; return; } if(state == s::LocalProfileSelect) { enteredString = ""; state = s::LocalProfileNew; } }, t::Single);
		game.addInput({{k::F2}, {k::J}}, [&](float) { assets.playSound("beep.ogg"); if(!assets.pIsPlayingLocally()) { state = s::Welcome; return; } if(state == s::Main) { enteredString = ""; state = s::LocalProfileSelect; } }, t::Single);
		game.addInput({{k::F3}, {k::K}}, [&](float) { assets.playSound("beep.ogg"); if(state == s::Main) state = s::Options; }, t::Single);
		game.addInput({{k::F4}, {k::L}}, [&](float)
		{
			assets.playSound("beep.ogg"); if(state == s::Main) { auto p(assets.getPackPaths()); packIndex = (packIndex + 1) % p.size(); levelDataIds = assets.getLevelIdsByPack(p[packIndex]); setIndex(0); }
		}, t::Single);
		game.addInput(Config::getTriggerExit(), [&](float)
		{
			assets.playSound("beep.ogg");
			if(isInMenu())
			{
				if(getCurrentMenu()->canGoBack()) getCurrentMenu()->goBack();
				else state = s::Main;
			}
			else if(state == s::FriendAdd || state == States::LocalProfileNew) state = s::Main;
		}, t::Single);

		game.addInput(Config::getTriggerExit(), [&](float mFrameTime) { if(state != s::Options) exitTimer += mFrameTime; });
		game.addInput(Config::getTriggerScreenshot(), [&](float){ mustTakeScreenshot = true; }, t::Single);
		game.addInput({{k::LAlt, k::Return}}, [&](float){ Config::setFullscreen(window, !window.getFullscreen()); refreshCamera(); }, t::Single);
		game.addInput({{k::BackSpace}}, [&](float){ if(isEnteringText() && !enteredString.empty()) enteredString.erase(enteredString.end() - 1); }, t::Single);
	}

	void MenuGame::setIndex(int mIndex)
	{
		currentIndex = mIndex;

		if(currentIndex > (int)(levelDataIds.size() - 1)) currentIndex = 0;
		else if(currentIndex < 0) currentIndex = levelDataIds.size() - 1;

		levelData = &assets.getLevelData(levelDataIds[currentIndex]);
		//levelStatus = levelData->createStatus();
		styleData = assets.getStyleData(levelData->styleId);
		difficultyMultipliers = levelData->difficultyMults;
		difficultyMultIndex = find(begin(difficultyMultipliers), end(difficultyMultipliers), 1) - begin(difficultyMultipliers);
	}

	void MenuGame::refreshFPS()
	{
		window.setVsync(Config::getVsync());
		if(Config::getLimitFPS()) window.setFPSLimit(Config::getMaxFPS());
	}

	void MenuGame::updateLeaderboard()
	{
		if(assets.pIsPlayingLocally()) { leaderboardString = "playing locally"; return; }

		currentLeaderboard = Online::getCurrentLeaderboard();
		if(currentLeaderboard == "NULL") { leaderboardString = "..."; return; }

		constexpr unsigned int leaderboardRecordCount{8};
		ssvuj::Value root{getRootFromString(currentLeaderboard)};
		if(as<string>(root, "id") != levelData->id) { leaderboardString = "..."; return; }

		currentPlayerScore = as<string>(root, "ps");

		using RecordPair = pair<string, float>;
		vector<RecordPair> recordPairs;

		int playerPosition{-1};

		for(auto itr(begin(root["r"])); itr != end(root["r"]); ++itr)
		{
			ssvuj::Value& record(*itr);
			string name{toLower(as<string>(record, 0))};
			float score{as<float>(record, 1)};
			recordPairs.push_back({name, score});
		}

		bool foundPlayer{false};
		for(unsigned int i{0}; i < recordPairs.size(); ++i)
		{
			if(recordPairs[i].first != assets.pGetName()) continue;
			playerPosition = i + 1;
			foundPlayer = true;
			break;
		}

		string result;
		for(unsigned int i{0}; i < recordPairs.size(); ++i)
		{
			if(currentPlayerScore != "NULL" && currentPlayerScore != "" && !foundPlayer && i == leaderboardRecordCount -1)
			{
				ssvuj::Value playerScoreRoot{getRootFromString(currentPlayerScore)};
				result.append("...(" + toStr(as<int>(playerScoreRoot, "p")) + ") " + assets.pGetName() + ": " + toStr(as<float>(playerScoreRoot, "s")) + "\n");
				break;
			}

			if(i <= leaderboardRecordCount)
			{
				if(playerPosition == -1 || i < leaderboardRecordCount)
				{
					auto& recordPair(recordPairs[i]);
					if(recordPair.first == assets.pGetName()) result.append(" >> ");
					result.append("(" + toStr(i + 1) +") " + recordPair.first + ": " + toStr(recordPair.second) + "\n");
				}
			}
			else break;
		}

		leaderboardString = result;
	}

	void MenuGame::updateFriends()
	{
		if(state != States::Main) return;

		if(assets.pIsPlayingLocally())			{ friendsString = "playing locally"; return; }
		if(assets.pGetTrackedNames().empty())	{ friendsString = "you have no friends! :(\nadd them in the options menu"; return; }

		const auto& fs(Online::getCurrentFriendScores());

		if(ssvuj::size(fs) == 0)
		{
			friendsString = "";
			for(const auto& n : assets.pGetTrackedNames()) friendsString.append("(?)" + n + "\n");
			return;
		}

		using ScoreTuple = tuple<int, string, float>;
		vector<ScoreTuple> tuples;
		for(const auto& n : assets.pGetTrackedNames())
		{
			if(!ssvuj::has(fs, n)) continue;

			const auto& score(ssvuj::as<float>(fs[n], 0));
			const auto& pos(ssvuj::as<unsigned int>(fs[n], 1));

			if(pos == 0) continue;
			tuples.emplace_back(pos, n, score);
		}

		sort(begin(tuples), end(tuples), [&](const ScoreTuple& mA, const ScoreTuple& mB){ return get<0>(mA) < get<0>(mB); });
		friendsString.clear();
		for(const auto& t : tuples) friendsString.append("(" + toStr(get<0>(t)) + ") " + get<1>(t) + ": " + toStr(get<2>(t)) + "\n");
	}

	template<typename T> float getGlobalLeft(const T& mElement)		{ return mElement.getGlobalBounds().left; }
	template<typename T> float getGlobalRight(const T& mElement)	{ return mElement.getGlobalBounds().left + mElement.getGlobalBounds().width; }
	template<typename T> float getGlobalTop(const T& mElement)		{ return mElement.getGlobalBounds().top; }
	template<typename T> float getGlobalBottom(const T& mElement)	{ return mElement.getGlobalBounds().top + mElement.getGlobalBounds().height; }
	template<typename T> float getGlobalWidth(const T& mElement)	{ return mElement.getGlobalBounds().width; }
	template<typename T> float getGlobalHeight(const T& mElement)	{ return mElement.getGlobalBounds().height; }

	template<typename T> float getLocalLeft(const T& mElement)		{ return mElement.getLocalBounds().left; }
	template<typename T> float getLocalRight(const T& mElement)		{ return mElement.getLocalBounds().left + mElement.getLocalBounds().width; }
	template<typename T> float getLocalTop(const T& mElement)		{ return mElement.getLocalBounds().top; }
	template<typename T> float getLocalBottom(const T& mElement)	{ return mElement.getLocalBounds().top + mElement.getLocalBounds().height; }
	template<typename T> float getLocalWidth(const T& mElement)		{ return mElement.getLocalBounds().width; }
	template<typename T> float getLocalHeight(const T& mElement)	{ return mElement.getLocalBounds().height; }

	void MenuGame::refreshCamera()
	{
		fw = (1024.f / Config::getWidth());
		fh = (768.f / Config::getHeight());
		fmin = max(fw, fh);
		w = Config::getWidth() * fmin;
		h = Config::getHeight() * fmin;
		overlayCamera.setView(View{FloatRect(0, 0, w, h)});
		titleBar.setOrigin({0, 0});
		titleBar.setScale({0.5f, 0.5f});
		titleBar.setPosition({20.f, 20.f});

		versionText.setString(toStr(Config::getVersion()));
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
		menuController.update();

		currentCreditsId += mFrameTime;
		creditsBar2.setTexture(assets().get<Texture>(creditsIds[static_cast<int>(currentCreditsId / 100) % creditsIds.size()]));

		if(state == States::Logging)
		{
			if(Online::getLoginStatus() == Online::LoginStat::Logged)
			{
				if(Online::getNewUserReg()) state = States::LREmail;
				else state = States::Main;
			}
			if(Online::getLoginStatus() == Online::LoginStat::TimedOut)	state = States::Welcome;
		}

		if(state == States::LREmail && !Online::getNewUserReg()) state = States::Main;

		if(!assets.pIsPlayingLocally() && Online::getConnectionStatus() != Online::ConnectStat::Connected) state = States::Welcome;

		updateLeaderboard();
		updateFriends();

		if(!window.isKeyPressed(Keyboard::Escape)) exitTimer = 0;
		if(exitTimer > 20) window.stop();

		if(isEnteringText())
		{
			unsigned int limit = (state == States::LREmail) ? 40 : 18;
			for(const auto& c : enteredChars) if(enteredString.size() < limit && (isalnum(c) || ispunct(c))) { assets.playSound("beep.ogg"); enteredString.append(toStr(c)); }
		}
		else if(state == States::LocalProfileSelect) { enteredString = assets.getLocalProfileNames()[profileIndex % assets.getLocalProfileNames().size()]; }
		else if(state == States::Main)
		{
			styleData.update(mFrameTime);
			backgroundCamera.rotate(levelStatus.rotationSpeed * 10.f * mFrameTime);

			if(!assets.pIsPlayingLocally())
			{
				float diffMult{difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]};
				Online::requestLeaderboardIfNeeded(levelData->id, diffMult);
			}
		}

		enteredChars.clear();
	}
	void MenuGame::draw()
	{
		styleData.computeColors();
		window.clear(state != States::Main ? Color::Black : styleData.getColors()[0]);

		backgroundCamera.apply();
		if(state == States::Main) styleData.drawBackground(window.getRenderWindow(), {0, 0}, levelStatus.sides);

		overlayCamera.apply();
		if(state == States::Main) { drawLevelSelection(); render(bottomBar); }
		else if(isEnteringText()) drawProfileCreation();
		else if(state == States::LocalProfileSelect) drawProfileSelection();
		else if(state == States::Options) drawOptions();
		else if(state == States::Welcome) drawWelcome();

		render(titleBar); render(creditsBar1); render(creditsBar2); render(versionText);
		if(mustTakeScreenshot) { window.getRenderWindow().capture().saveToFile("screenshot.png"); mustTakeScreenshot = false; }
	}

	Text& MenuGame::renderText(const string& mString, Text& mText, Vec2f mPosition, unsigned int mSize)
	{
		unsigned int originalSize{mText.getCharacterSize()};
		if(mSize != 0) mText.setCharacterSize(mSize);

		if(mText.getString() != mString) mText.setString(mString);

		if(state != States::Main || Config::getBlackAndWhite()) mText.setColor(Color::White);
		else mText.setColor(styleData.getMainColor());

		mText.setPosition(mPosition);
		render(mText); mText.setCharacterSize(originalSize);

		return mText;
	}
	Text& MenuGame::renderText(const string& mString, Text& mText, Vec2f mPosition, const Color& mColor, unsigned int mSize)
	{
		unsigned int originalSize{mText.getCharacterSize()};
		if(mSize != 0) mText.setCharacterSize(mSize);

		if(mText.getString() != mString) mText.setString(mString);

		mText.setColor(mColor);
		mText.setPosition(mPosition);
		render(mText); mText.setCharacterSize(originalSize);

		return mText;
	}

	void MenuGame::drawLevelSelection()
	{
		MusicData musicData{assets.getMusicData(levelData->musicId)};
		PackData packData{assets.getPackData(levelData->packPath.substr(6, levelData->packPath.size() - 7))};
		const string& packName{packData.name};

		if(Config::getOnline())
		{
			string versionMessage{"connecting to server..."};
			float serverVersion{Online::getServerVersion()};

			if(serverVersion == -1) versionMessage = "error connecting to server";
			else if(serverVersion == Config::getVersion()) versionMessage = "you have the latest version";
			else if(serverVersion < Config::getVersion()) versionMessage = "your version is newer (beta)";
			else if(serverVersion > Config::getVersion()) versionMessage = "update available (" + toStr(serverVersion) + ")";
			renderText(versionMessage, cProfText, {20, 0}, 13);

			Text& profile = renderText("profile: " + assets.pGetName(), cProfText, Vec2f{20.f, getGlobalBottom(titleBar)}, 18);
			Text& pack = renderText("pack: " + packName + " (" + toStr(packIndex + 1) + "/" + toStr(assets.getPackPaths().size()) + ")", cProfText, {20.f, getGlobalBottom(profile) - 7.f}, 18);

			string lbestStr;
			if(assets.pIsPlayingLocally()) lbestStr = "local best: " + toStr(assets.getLocalScore(getLocalValidator(levelData->id, difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()])));
			else { lbestStr = Online::getLoginStatus() == Online::LoginStat::Logged ? "logged in as: " + Online::getCurrentUsername() : "logging in..."; }

			Text& lbest = renderText(lbestStr, cProfText, {20.f, getGlobalBottom(pack) - 7.f}, 18);
			if(difficultyMultipliers.size() > 1) renderText("difficulty: " + toStr(difficultyMultipliers[difficultyMultIndex % difficultyMultipliers.size()]), cProfText, {20.f, getGlobalBottom(lbest) - 7.f}, 18);

			renderText(leaderboardString, cProfText, {20.f, getGlobalBottom(lbest)}, 15);
			Text& smsg = renderText("server message: " + Online::getServerMessage(), levelAuth, {20.f, getGlobalTop(bottomBar) - 20.f}, 14);
			friendsText.setOrigin({getLocalWidth(friendsText), 0.f});
			renderText("friends:\n" + friendsString, friendsText, {w - 20.f, getGlobalBottom(titleBar)}, 18);

			if(!Config::isEligibleForScore()) renderText("not eligible for scoring: " + Config::getUneligibilityReason(), cProfText, {20.f, getGlobalTop(smsg) - 20.f}, 11);

			if(!assets.pIsPlayingLocally() && Online::getLoginStatus() == Online::LoginStat::Logged)
			{
				const auto& us(Online::getUserStats());
				string userStats;
				userStats += "deaths: " + toStr(us.deaths) + "\n";
				userStats += "restarts: " + toStr(us.restarts) + "\n";
				userStats += "played: " + toStr(us.minutesSpentPlaying) + " min";
				renderText(userStats, levelMusc, {getGlobalRight(titleBar) + 10.f, getGlobalTop(titleBar)}, 13.5f);
			}
		}
		else renderText("online disabled", cProfText, {20, 0}, 13);

		Text& lname = renderText(levelData->name, levelName, {20.f, h / 2.f});
		Text& ldesc = renderText(levelData->description, levelDesc, {20.f, getGlobalBottom(lname) - 5.f});
		Text& lauth = renderText("author: " + levelData->author, levelAuth, {20.f, getGlobalBottom(ldesc) + 25.f});
		renderText("music: " + musicData.name + " by " + musicData.author + " (" + musicData.album + ")", levelMusc, {20.f, getGlobalBottom(lauth) - 5.f});
		renderText("(" + toStr(currentIndex + 1) + "/" + toStr(levelDataIds.size()) + ")", levelMusc, {20.f, getGlobalTop(lname) - 25.f});

		string packNames{"Installed packs:\n"};
		for(const auto& n : assets.getPackIds()) { if(packData.id == n) packNames += ">>> "; packNames.append(n + "\n"); }
		packsText.setString(packNames);
		packsText.setOrigin(packsText.getGlobalBounds().width, packsText.getGlobalBounds().height);
		packsText.setPosition({w - 20.f, getGlobalTop(bottomBar) - 15.f});
		packsText.setColor(styleData.getMainColor());
		render(packsText);


	}
	void MenuGame::drawProfileCreation()
	{
		string title;
		switch(state)
		{
			case States::LRUser:			title = "insert username"; break;
			case States::LRPass:			title = "insert password"; break;
			case States::LREmail:			title = "insert email"; break;
			case States::LocalProfileNew:	title = "create local profile"; break;
			case States::FriendAdd:			title = "add friend"; break;
			default: throw;
		}

		renderText(title, cProfText,						{20, 768 - 395});
		renderText("insert text", cProfText,				{20, 768 - 375});
		renderText("press enter when done", cProfText, 		{20, 768 - 335});
		renderText("keep esc pressed to exit", cProfText, 	{20, 768 - 315});
		renderText(state == States::LRPass ? std::string(enteredString.size(), '*') : enteredString, levelName, {20, 768 - 245 - 40}, (state == States::LREmail) ? 32 : 0);
	}
	void MenuGame::drawProfileSelection()
	{
		if(!assets.pIsPlayingLocally()) throw;
		renderText("local profile selection", cProfText, 				{20, 768 - 395});
		renderText("press left/right to browse profiles", cProfText, 	{20, 768 - 375});
		renderText("press enter to select profile", cProfText, 			{20, 768 - 355});
		renderText("press f1 to create a new profile", cProfText, 		{20, 768 - 335});
		renderText(enteredString, levelName, 							{20, 768 - 245 - 40});
	}
	void MenuGame::drawOptions()
	{
		renderText(optionsMenu.getCategory().getName(), levelDesc, {20.f, getGlobalBottom(titleBar)});

		float currentX{0.f}, currentY{0.f};
		auto& currentItems(optionsMenu.getItems());
		for(int i{0}; i < static_cast<int>(currentItems.size()); ++i)
		{
			currentY += 19;
			if(i != 0 && i % 21 == 0) { currentY = 0; currentX += 180; }
			string name, itemName{currentItems[i]->getName()};
			if(i == optionsMenu.getIndex()) name.append(">> ");
			name.append(itemName);

			int extraSpacing{0};
			if(itemName == "back") extraSpacing = 20;
			renderText(name, cProfText, {20.f + currentX, getGlobalBottom(titleBar) + 20.f + currentY + extraSpacing}, currentItems[i]->isEnabled() ? Color::White : Color{155, 155, 155, 255});
		}

		if(Config::getOfficial()) renderText("official mode on - some options cannot be changed", cProfText, {20, h - 30.f});
		else if(Config::getOfficial()) renderText("official mode off - your scores won't be sent to the server", cProfText, {20, h - 30.f});

		if(assets.pIsPlayingLocally()) renderText("local mode on - some options cannot be changed", cProfText, {20, h - 60.f});
	}

	void MenuGame::drawWelcome()
	{
		renderText(welcomeMenu.getCategory().getName(), levelDesc, {20.f, getGlobalBottom(titleBar)});

		float currentX{0.f}, currentY{0.f};
		auto& currentItems(welcomeMenu.getItems());
		for(int i{0}; i < static_cast<int>(currentItems.size()); ++i)
		{
			currentY += 19;
			if(i != 0 && i % 21 == 0) { currentY = 0; currentX += 180; }
			string name, itemName{currentItems[i]->getName()};
			if(i == welcomeMenu.getIndex()) name.append(">> ");
			name.append(itemName);

			int extraSpacing{0};
			if(itemName == "back") extraSpacing = 20;
			renderText(name, cProfText, {20.f + currentX, getGlobalBottom(titleBar) + 20.f + currentY + extraSpacing}, currentItems[i]->isEnabled() ? Color::White : Color{155, 155, 155, 255});
		}

		renderText(Online::getLoginStatus() == Online::LoginStat::Logged ? "logged in as: " + Online::getCurrentUsername() : "not logged in", cProfText, {20, h - 50.f});

		string connectionString;
		switch(Online::getConnectionStatus())
		{
			case Online::ConnectStat::Disconnected:	connectionString = "not connected to server"; break;
			case Online::ConnectStat::Connecting:	connectionString = "connecting to server..."; break;
			case Online::ConnectStat::Connected:	connectionString = "connected to server"; break;
		}
		renderText(connectionString, cProfText, {20, h - 30.f});
	}
}
