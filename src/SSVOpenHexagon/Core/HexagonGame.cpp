// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Core/MenuGame.h"
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace ssvu;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;
using namespace hg::Utils;

namespace hg
{
	HexagonGame::HexagonGame(GameWindow& mGameWindow) : window(mGameWindow), fpsWatcher(window)
	{
		initFlashEffect();

		game.onUpdate += [&](float mFrameTime) { update(mFrameTime); };
		game.onDraw += [&]{ draw(); };

		add3StateInput(game, getTriggerRotateCCW(), getTriggerRotateCW(), inputMovement);
		add2StateInput(game, getTriggerFocus(), inputFocused);
		add2StateInput(game, getTriggerSwap(), inputSwap);
		game.addInput(getTriggerExit(),			[&](float){ goToMenu(); });
		game.addInput(getTriggerForceRestart(),	[&](float){ status.mustRestart = true; });
		game.addInput(getTriggerRestart(),		[&](float){ if(status.hasDied) status.mustRestart = true; });
		game.addInput(getTriggerScreenshot(),	[&](float){ mustTakeScreenshot = true; }, Input::Trigger::Type::Single);
	}

	void HexagonGame::newGame(const string& mId, bool mFirstPlay, float mDifficultyMult)
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
		messageText.setString("");
		eventTimeline.clear(); eventTimeline.reset();
		messageTimeline.clear(); messageTimeline.reset();

		// Game status cleanup
		status = HexagonGameStatus{};
		restartId = mId;
		restartFirstTime = false;
		setSides(levelData.sides);
		status.swapEnabled = levelData.swapEnabled;

		// Manager cleanup
		manager.clear();
		factory.createPlayer();

		// Timeline cleanup
		timeline.clear(); timeline.reset();
		effectTimelineManager.clear();
		mustChangeSides = false;

		// FPSWatcher reset
		fpsWatcher.reset();
		if(getOfficial()) fpsWatcher.enable();

		// LUA context cleanup
		if(!mFirstPlay) runLuaFunction<void>("onUnload");
		lua = Lua::LuaContext{};
		initLua();
		runLuaFile(levelData.luaScriptPath);
		runLuaFunction<void>("onLoad");

		// Random rotation direction
		if(getRnd(0, 100) > 50) setRotationSpeed(getRotationSpeed() * -1.f);

		// Reset zoom
		overlayCamera.setView({{getWidth() / 2.f, getHeight() / 2.f}, ssvs::Vec2f(getWidth(), getHeight())});
		backgroundCamera.setView({{0, 0}, {getWidth() * getZoomFactor(), getHeight() * getZoomFactor()}});
		backgroundCamera.setRotation(0);

		// 3D Cameras cleanup
		depthCameras.clear();
		unsigned int depth(styleData._3dDepth);
		if(depth > get3DMaxDepth()) depth = get3DMaxDepth();
		for(unsigned int i{0}; i < depth; ++i) depthCameras.push_back({window, {}});
	}
	void HexagonGame::death(bool mForce)
	{
		playSound("death.ogg", SoundPlayer::Mode::Abort);

		if(!mForce && (getInvincible() || status.tutorialMode)) return;
		playSound("gameOver.ogg", SoundPlayer::Mode::Abort);

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

		levelData.rotationSpeed += levelData.rotationSpeedIncrement * getSign(levelData.rotationSpeed);
		levelData.rotationSpeed *= -1.f;

		const auto& rotationSpeedMax(levelData.rotationSpeedMax);
		if(status.fastSpin < 0 && abs(levelData.rotationSpeed) > rotationSpeedMax) levelData.rotationSpeed = rotationSpeedMax * getSign(levelData.rotationSpeed);

		status.fastSpin = levelData.fastSpin;
	}

	void HexagonGame::sideChange(int mSideNumber)
	{
		runLuaFunction<void>("onIncrement");
		levelData.speedMultiplier += levelData.speedIncrement;
		levelData.delayMultiplier += levelData.delayIncrement;

		if(status.randomSideChangesEnabled) setSides(mSideNumber);
		mustChangeSides = false;
	}

	void HexagonGame::checkAndSaveScore()
	{
		if(getInvincible()) { log("Not saving score - invincibility on", "hg::HexagonGame::checkAndSaveScore()"); return; }

		string localValidator{getLocalValidator(levelData.id, difficultyMult)};
		if(getScore(localValidator) < status.currentTime) setScore(localValidator, status.currentTime);
		saveCurrentProfile();

		if(status.currentTime < 8) { log("Not sending score - less than 8 seconds", "hg::HexagonGame::checkAndSaveScore()"); return; }
		if(status.scoreInvalid || !isEligibleForScore()) { log("Not sending score - not eligible", "hg::HexagonGame::checkAndSaveScore()"); return; }

		string validator{Online::getValidator(levelData.packPath, levelData.id, levelData.levelRootPath, levelData.styleRootPath, levelData.luaScriptPath)};
		Online::startSendScore(toLower(getCurrentProfile().getName()), validator, difficultyMult, status.currentTime);
	}
	void HexagonGame::goToMenu(bool mSendScores)
	{
		stopAllSounds();
		playSound("beep.ogg");
		fpsWatcher.disable();

		if(mSendScores && !status.hasDied) checkAndSaveScore();
		runLuaFunction<void>("onUnload");
		window.setGameState(mgPtr->getGame());
		mgPtr->init();
	}
	void HexagonGame::changeLevel(const string& mId, bool mFirstTime) { newGame(mId, mFirstTime, difficultyMult); }
	void HexagonGame::addMessage(const string& mMessage, float mDuration)
	{
		messageTimeline.append<Do>([&, mMessage]{ playSound("beep.ogg"); messageText.setString(mMessage); });
		messageTimeline.append<Wait>(mDuration);
		messageTimeline.append<Do>([=]{ messageText.setString(""); });
	}
	void HexagonGame::setLevelData(LevelData mLevelSettings, bool mMusicFirstPlay)
	{
		levelData = mLevelSettings;
		styleData = getStyleData(levelData.styleId);
		musicData = getMusicData(levelData.musicId);
		musicData.firstPlay = mMusicFirstPlay;
	}

	void HexagonGame::playLevelMusic() { if(!getNoMusic()) musicData.playRandomSegment(); }
	void HexagonGame::stopLevelMusic() { if(!getNoMusic()) stopAllMusic(); }

	void HexagonGame::invalidateScore() { status.scoreInvalid = true; log("Too much slowdown, invalidating official game", "HexagonGame::invalidateScore"); }
}
