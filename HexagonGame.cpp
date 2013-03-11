// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Global/Assets.h"
#include "Global/Factory.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "MenuGame.h"
#include "Online/Online.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;

namespace hg
{
	HexagonGame::HexagonGame(GameWindow& mGameWindow) : window(mGameWindow)
	{
		initFlashEffect();

		game.onUpdate += [&](float mFrameTime) { update(mFrameTime); };
		game.onDraw += [&]{ draw(); };
		
		using k = Keyboard::Key;
		game.addInput({{k::Left}}, 		[&](float){ inputMovement = -1; });
		game.addInput({{k::Right}}, 	[&](float){ inputMovement = 1; });
		game.addInput({{k::LShift}}, 	[&](float){ inputFocused = true; });
		game.addInput({{k::Escape}}, 	[&](float){ goToMenu(); });
		game.addInput({{k::R}}, 		[&](float){ status.mustRestart = true; });
		game.addInput({{k::Space}}, 	[&](float){ if(status.hasDied) status.mustRestart = true; });
		game.addInput({{k::Return}}, 	[&](float){ if(status.hasDied) status.mustRestart = true; });
		game.addInput({{k::F12}}, 		[&](float){ mustTakeScreenshot = true; }, Input::Trigger::Types::SINGLE);

		using b = Mouse::Button;
		game.addInput({{b::Left}}, 		[&](float){ inputMovement = -1; });
		game.addInput({{b::Right}}, 	[&](float){ inputMovement = 1; });
		game.addInput({{b::Middle}}, 	[&](float){ inputFocused = true; });
		game.addInput({{b::XButton1}},	[&](float){ status.mustRestart = true; });
		game.addInput({{b::XButton2}},	[&](float){ status.mustRestart = true; });
	}

	void HexagonGame::newGame(string mId, bool mFirstPlay, float mDifficultyMult)
	{
		firstPlay = mFirstPlay;
		setLevelData(getLevelData(mId), mFirstPlay);
		difficultyMult = mDifficultyMult;

		// Audio cleanup
		stopAllSounds();
		playSound("go.ogg");
		stopLevelMusic();
		playLevelMusic();

		// Events cleanup
		clearMessage();

		for(auto eventPtr : eventPtrs) delete eventPtr;
		eventPtrs.clear();

		while(!eventPtrQueue.empty()) { delete eventPtrQueue.front(); eventPtrQueue.pop(); }
		eventPtrQueue = queue<EventData*>{};

		// Game status cleanup
		status = HexagonGameStatus{};
		restartId = mId;
		restartFirstTime = false;
		setSides(levelData.getSides());

		// Manager cleanup
		manager.clear();
		factory.createPlayer();

		// Timeline cleanup
		timeline.clear();
		messageTimeline.clear();
		effectTimelineManager.clear();

		// LUA context cleanup
		if(!mFirstPlay) runLuaFunction<void>("onUnload");
		lua = Lua::LuaContext{};
		initLua();
		runLuaFile(levelData.getValueString("lua_file"));
		runLuaFunction<void>("onLoad");

		// Random rotation direction
		if(getRnd(0, 100) > 50) setRotationSpeed(getRotationSpeed() * -1);

		// Reset zoom
		overlayCamera.setView({{getWidth() / 2.f, getHeight() / 2.f}, sf::Vector2f(getWidth(), getHeight())});
		backgroundCamera.setView({{0, 0}, {getWidth() * getZoomFactor(), getHeight() * getZoomFactor()}});
		backgroundCamera.setRotation(0);
		
		// 3D Cameras cleanup
		depthCameras.clear();
		unsigned int depth{styleData.get3DDepth()};
		if(depth > get3DMaxDepth()) depth = get3DMaxDepth();
		for(unsigned int i{0}; i < depth; ++i) depthCameras.push_back({window, {}});
	}
	void HexagonGame::death()
	{
		playSound("death.ogg");
		playSound("gameOver.ogg");

		if(getInvincible()) return;

		status.flashEffect = 255;
		shakeCamera(effectTimelineManager, overlayCamera);
		shakeCamera(effectTimelineManager, backgroundCamera);
		for(auto& depthCamera : depthCameras) shakeCamera(effectTimelineManager, depthCamera);
		status.hasDied = true;
		stopLevelMusic();
		checkAndSaveScore();

		if(getAutoRestart()) status.mustRestart = true;
	}

	void HexagonGame::incrementDifficulty()
	{
		playSound("levelUp.ogg");

		setRotationSpeed(levelData.getRotationSpeed() + levelData.getRotationSpeedIncrement() * getSign(getRotationSpeed()));
		setRotationSpeed(levelData.getRotationSpeed() * -1);
		
		if(status.fastSpin < 0 && abs(getRotationSpeed()) > levelData.getValueFloat("rotation_speed_max"))
			setRotationSpeed(levelData.getValueFloat("rotation_speed_max") * getSign(getRotationSpeed()));

		status.fastSpin = levelData.getFastSpin();
		timeline.insert<Do>(timeline.getCurrentIndex() + 1, [&]{ sideChange(getRnd(levelData.getSidesMin(), levelData.getSidesMax() + 1)); });
	}
	void HexagonGame::sideChange(int mSideNumber)
	{
		if(manager.getComponents("wall").size() > 0)
		{
			timeline.insert<Do>(timeline.getCurrentIndex() + 1, [&]{ clearAndResetTimeline(timeline); });
			timeline.insert<Do>(timeline.getCurrentIndex() + 1, [&, mSideNumber]{ sideChange(mSideNumber); });
			timeline.insert<Wait>(timeline.getCurrentIndex() + 1, 1);
			return;
		}

		runLuaFunction<void>("onIncrement");
		setSpeedMultiplier(levelData.getSpeedMultiplier() + levelData.getSpeedIncrement());
		setDelayMultiplier(levelData.getDelayMultiplier() + levelData.getDelayIncrement());

		if(status.randomSideChangesEnabled) setSides(mSideNumber);
	}

	void HexagonGame::checkAndSaveScore()
	{
		if(getInvincible()) return;

		string localValidator{getScoreValidator(levelData.getId(), difficultyMult)};
		if(getScore(localValidator) < status.currentTime) setScore(localValidator, status.currentTime);
		saveCurrentProfile();

		if(status.scoreInvalid || !isEligibleForScore()) return;

		string validator{Online::getValidator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
		Online::startSendScore(getCurrentProfile().getName(), validator, status.currentTime);
	}
	void HexagonGame::goToMenu(bool mSendScores)
	{
		stopAllSounds();
		playSound("beep.ogg");

		if(mSendScores && !status.hasDied) checkAndSaveScore();
		runLuaFunction<void>("onUnload");
		window.setGameState(mgPtr->getGame());
		mgPtr->init();
	}
	void HexagonGame::changeLevel(string mId, bool mFirstTime) { newGame(mId, mFirstTime, difficultyMult); }
	void HexagonGame::addMessage(string mMessage, float mDuration)
	{
		Text* text = new Text(mMessage, getFont("imagine.ttf"), 40 / getZoomFactor());
		text->setPosition(Vector2f(getWidth() / 2, getHeight() / 6));
		text->setOrigin(text->getGlobalBounds().width / 2, 0);

		messageTimeline.append<Do>([&, text, mMessage]{ playSound("beep.ogg"); messageTextPtr = text; });
		messageTimeline.append<Wait>(mDuration);
		messageTimeline.append<Do>([=]{ messageTextPtr = nullptr; delete text; });
	}
	void HexagonGame::clearMessage()
	{
		if(messageTextPtr == nullptr) return;
		delete messageTextPtr; messageTextPtr = nullptr;
	}

	void HexagonGame::setLevelData(LevelData mLevelSettings, bool mMusicFirstPlay)
	{
		levelData = mLevelSettings;
		styleData = getStyleData(levelData.getStyleId());
		musicData = getMusicData(levelData.getMusicId());
		musicData.setFirstPlay(mMusicFirstPlay);
	}

	void HexagonGame::playLevelMusic() { if(!getNoMusic()) musicData.playRandomSegment(musicPtr); }
	void HexagonGame::stopLevelMusic() { if(!getNoMusic()) if(musicPtr != nullptr) musicPtr->stop(); }
}
