// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Groups.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Core/MenuGame.h"
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace ssvu;
using namespace ssvs;
using namespace sses;
using namespace hg::Utils;

namespace hg
{
	HexagonGame::HexagonGame(HGAssets& mAssets, GameWindow& mGameWindow) : assets(mAssets), window(mGameWindow), fpsWatcher(window)
	{
		game.onUpdate += [this](float mFT) { update(mFT); };
		game.onDraw += [this]{ draw(); };
		window.onRecreation += [this]{ initFlashEffect(); };

		add3StateInput(game, Config::getTriggerRotateCCW(), Config::getTriggerRotateCW(), inputMovement);
		add2StateInput(game, Config::getTriggerFocus(), inputFocused);
		add2StateInput(game, Config::getTriggerSwap(), inputSwap);
		game.addInput(Config::getTriggerExit(),			[this](float){ goToMenu(); });
		game.addInput(Config::getTriggerForceRestart(),	[this](float){ status.mustRestart = true; });
		game.addInput(Config::getTriggerRestart(),		[this](float){ if(status.hasDied) status.mustRestart = true; });
		game.addInput(Config::getTriggerScreenshot(),	[this](float){ mustTakeScreenshot = true; }, Input::Trigger::Type::Once);
	}

	void HexagonGame::newGame(const string& mId, bool mFirstPlay, float mDifficultyMult)
	{
		initFlashEffect();

		firstPlay = mFirstPlay;
		setLevelData(assets.getLevelData(mId), mFirstPlay);
		difficultyMult = mDifficultyMult;

		// Audio cleanup
		assets.stopSounds(); stopLevelMusic();
		assets.playSound("go.ogg"); playLevelMusic();

		if(Config::getMusicSpeedDMSync())
		{
			auto current(assets.getMusicPlayer().getCurrent());
			if(current != nullptr) current->setPitch(pow(difficultyMult, 0.12f));
		}

		// Events cleanup
		messageText.setString("");
		eventTimeline.clear(); eventTimeline.reset();
		messageTimeline.clear(); messageTimeline.reset();

		// Manager cleanup
		manager.clear();
		factory.createPlayer();

		// Timeline cleanup
		timeline.clear(); timeline.reset();
		effectTimelineManager.clear();
		mustChangeSides = false;

		// FPSWatcher reset
		fpsWatcher.reset();
		if(Config::getOfficial()) fpsWatcher.enable();

		// LUA context and game status cleanup
		status = HexagonGameStatus{};
		if(!mFirstPlay) runLuaFunction<void>("onUnload");
		lua = Lua::LuaContext{};
		initLua();
		runLuaFile(levelData->luaScriptPath);
		runLuaFunction<void>("onInit");
		runLuaFunction<void>("onLoad");
		restartId = mId;
		restartFirstTime = false;
		setSides(levelStatus.sides);

		// Reset zoom
		overlayCamera.setView({{Config::getWidth() / 2.f, Config::getHeight() / 2.f}, Vec2f(Config::getWidth(), Config::getHeight())});
		backgroundCamera.setView({ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(), Config::getHeight() * Config::getZoomFactor()}});
		backgroundCamera.setRotation(0);

		// 3D Cameras cleanup
		depthCameras.clear();
		auto depth(styleData._3dDepth);
		if(depth > Config::get3DMaxDepth()) depth = Config::get3DMaxDepth();
		for(auto i(0u); i < depth; ++i) depthCameras.push_back({window, {}});
	}
	void HexagonGame::death(bool mForce)
	{
		fpsWatcher.disable();
		assets.playSound("death.ogg", SoundPlayer::Mode::Abort);

		if(!mForce && (Config::getInvincible() || levelStatus.tutorialMode)) return;
		assets.playSound("gameOver.ogg", SoundPlayer::Mode::Abort);

		if(!assets.pIsLocal() && Config::isEligibleForScore()) { Online::trySendDeath(); }

		status.flashEffect = 255;
		shakeCamera(effectTimelineManager, overlayCamera);
		shakeCamera(effectTimelineManager, backgroundCamera);
		for(auto& depthCamera : depthCameras) shakeCamera(effectTimelineManager, depthCamera);
		status.hasDied = true;
		stopLevelMusic();
		checkAndSaveScore();

		if(Config::getAutoRestart()) status.mustRestart = true;
	}

	void HexagonGame::incrementDifficulty()
	{
		assets.playSound("levelUp.ogg");

		levelStatus.rotationSpeed += levelStatus.rotationSpeedInc * getSign(levelStatus.rotationSpeed);
		levelStatus.rotationSpeed *= -1.f;

		const auto& rotationSpeedMax(levelStatus.rotationSpeedMax);
		if(status.fastSpin < 0 && abs(levelStatus.rotationSpeed) > rotationSpeedMax) levelStatus.rotationSpeed = rotationSpeedMax * getSign(levelStatus.rotationSpeed);

		status.fastSpin = levelStatus.fastSpin;
	}

	void HexagonGame::sideChange(int mSideNumber)
	{
		runLuaFunction<void>("onIncrement");
		levelStatus.speedMult += levelStatus.speedInc;
		levelStatus.delayMult += levelStatus.delayInc;

		if(levelStatus.rndSideChangesEnabled) setSides(mSideNumber);
		mustChangeSides = false;
	}

	void HexagonGame::checkAndSaveScore()
	{
		if(Config::getInvincible()) { lo("hg::HexagonGame::checkAndSaveScore()") << "Not saving score - invincibility on" << endl; return; }
		if(status.scoreInvalid || !Config::isEligibleForScore()) { lo("hg::HexagonGame::checkAndSaveScore()") << "Not sending/saving score - not eligible" << endl; return; }

		if(assets.pIsLocal())
		{
			string localValidator{getLocalValidator(levelData->id, difficultyMult)};
			if(assets.getLocalScore(localValidator) < status.currentTime) assets.setLocalScore(localValidator, status.currentTime);
			assets.saveCurrentLocalProfile();
		}
		else
		{
			if(status.currentTime < 1) { lo("hg::HexagonGame::checkAndSaveScore()") << "Not sending score - less than 8 seconds" << endl; return; }
			Online::trySendScore(levelData->id, difficultyMult, status.currentTime);
		}
	}
	void HexagonGame::goToMenu(bool mSendScores)
	{
		assets.stopSounds();
		assets.playSound("beep.ogg");
		fpsWatcher.disable();

		if(mSendScores && !status.hasDied) checkAndSaveScore();
		runLuaFunction<void>("onUnload");
		window.setGameState(mgPtr->getGame());
		mgPtr->init();
	}
	void HexagonGame::changeLevel(const string& mId, bool mFirstTime) { newGame(mId, mFirstTime, difficultyMult); }
	void HexagonGame::addMessage(const string& mMessage, float mDuration)
	{
		messageTimeline.append<Do>([&, mMessage]{ assets.playSound("beep.ogg"); messageText.setString(mMessage); });
		messageTimeline.append<Wait>(mDuration);
		messageTimeline.append<Do>([=]{ messageText.setString(""); });
	}
	void HexagonGame::setLevelData(const LevelData& mLevelData, bool mMusicFirstPlay)
	{
		levelData = &mLevelData;
		levelStatus = LevelStatus{};
		styleData = assets.getStyleData(levelData->styleId);
		musicData = assets.getMusicData(levelData->musicId);
		musicData.firstPlay = mMusicFirstPlay;
	}

	void HexagonGame::playLevelMusic() { if(!Config::getNoMusic()) musicData.playRandomSegment(assets); }
	void HexagonGame::stopLevelMusic() { if(!Config::getNoMusic()) assets.stopMusics(); }

	void HexagonGame::invalidateScore() { status.scoreInvalid = true; lo("HexagonGame::invalidateScore") << "Too much slowdown, invalidating official game" << endl; }

	Color HexagonGame::getColorMain() const
	{
		if(Config::getBlackAndWhite())
		{
			if(status.drawing3D) return Color{255, 255, 255, status.overrideColor.a};
			return Color(255, 255, 255, styleData.getMainColor().a);
		}
		else if(status.drawing3D) return status.overrideColor;
		else return styleData.getMainColor();
	}
	void HexagonGame::setSides(unsigned int mSides)
	{
		assets.playSound("beep.ogg");
		if(mSides < 3) mSides = 3;
		levelStatus.sides = mSides;
	}
}
