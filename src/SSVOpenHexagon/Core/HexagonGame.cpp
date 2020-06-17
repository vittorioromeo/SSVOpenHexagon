// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

using namespace std;
using namespace sf;
using namespace ssvu;
using namespace ssvs;
using namespace hg::Utils;

namespace hg
{
void Factory::createWall(int mSide, float mThickness, const SpeedData& mSpeed,
    const SpeedData& mCurve, float mHueMod)
{
    hexagonGame.walls.emplace_back(hexagonGame, centerPos, mSide, mThickness,
        Config::getSpawnDistance(), mSpeed, mCurve);
    hexagonGame.walls.back().setHueMod(mHueMod);
}

HexagonGame::HexagonGame(HGAssets& mAssets, GameWindow& mGameWindow)
    : assets(mAssets), window(mGameWindow), player{*this, ssvs::zeroVec2f},
      fpsWatcher(window)
{
    game.onUpdate += [this](FT mFT) { update(mFT); };
    game.onPostUpdate += [this] {
        inputImplLastMovement = inputMovement;
        inputImplBothCWCCW = inputImplCW && inputImplCCW;
    };
    game.onDraw += [this] { draw(); };
    window.onRecreation += [this] { initFlashEffect(); };

    add2StateInput(game, Config::getTriggerRotateCW(), inputImplCW);
    add2StateInput(game, Config::getTriggerRotateCCW(), inputImplCCW);
    add2StateInput(game, Config::getTriggerFocus(), inputFocused);
    add2StateInput(game, Config::getTriggerSwap(), inputSwap);
    game.addInput(
        Config::getTriggerExit(), [this](FT /*unused*/) { goToMenu(); });
    game.addInput(
        Config::getTriggerForceRestart(),
        [this](FT /*unused*/) { status.mustRestart = true; },
        Input::Type::Once);
    game.addInput(
        Config::getTriggerRestart(),
        [this](FT /*unused*/) {
            if(status.hasDied)
            {
                status.mustRestart = true;
            }
        },
        Input::Type::Once);
    game.addInput(
        Config::getTriggerScreenshot(),
        [this](FT /*unused*/) { mustTakeScreenshot = true; },
        Input::Type::Once);
}

void HexagonGame::newGame(
    const string& mId, bool mFirstPlay, float mDifficultyMult)
{
    initFlashEffect();

    firstPlay = mFirstPlay;
    setLevelData(assets.getLevelData(mId), mFirstPlay);
    difficultyMult = mDifficultyMult;

    // Audio cleanup
    assets.stopSounds();
    stopLevelMusic();
    // assets.playSound("go.ogg");
    playLevelMusic();
    assets.musicPlayer.pause();

    auto* current(assets.getMusicPlayer().getCurrent());
    if(current != nullptr)
    {
        current->setPitch(
            (Config::getMusicSpeedDMSync() ? pow(difficultyMult, 0.12f) : 1.f) *
            Config::getMusicSpeedMult());
    }

    // Events cleanup
    messageText.setString("");
    eventTimeline.clear();
    eventTimeline.reset();
    messageTimeline.clear();
    messageTimeline.reset();

    // Manager cleanup
    walls.clear();
    player = CPlayer{*this, ssvs::zeroVec2f};


    // Timeline cleanup
    timeline.clear();
    timeline.reset();
    effectTimelineManager.clear();
    mustChangeSides = false;

    // FPSWatcher reset
    fpsWatcher.reset();
    // if(Config::getOfficial()) fpsWatcher.enable();

    // LUA context and game status cleanup
    inputImplCCW = inputImplCW = inputImplBothCWCCW = false;
    status = HexagonGameStatus{};
    if(!mFirstPlay)
    {
        runLuaFunction<void>("onUnload");
    }
    lua = Lua::LuaContext{};
    initLua();
    runLuaFile(levelData->luaScriptPath);
    runLuaFunction<void>("onInit");
    runLuaFunction<void>("onLoad");
    restartId = mId;
    restartFirstTime = false;
    setSides(levelStatus.sides);

    // Reset zoom
    overlayCamera.setView(
        {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
            Vec2f(Config::getWidth(), Config::getHeight())});
    backgroundCamera.setView(
        {ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(),
                              Config::getHeight() * Config::getZoomFactor()}});
    backgroundCamera.setRotation(0);

    // Reset skew
    overlayCamera.setSkew(ssvs::Vec2f{1.f, 1.f});
    backgroundCamera.setSkew(ssvs::Vec2f{1.f, 1.f});
}
void HexagonGame::death(bool mForce)
{
    fpsWatcher.disable();
    assets.playSound("death.ogg", SoundPlayer::Mode::Abort);

    if(!mForce && (Config::getInvincible() || levelStatus.tutorialMode))
    {
        return;
    }
    assets.playSound("gameOver.ogg", SoundPlayer::Mode::Abort);

    if(!assets.pIsLocal() && Config::isEligibleForScore())
    {
        Online::trySendDeath();
    }

    status.flashEffect = 255;
    overlayCamera.setView(
        {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
            Vec2f(Config::getWidth(), Config::getHeight())});
    backgroundCamera.setCenter(ssvs::zeroVec2f);
    shakeCamera(effectTimelineManager, overlayCamera);
    shakeCamera(effectTimelineManager, backgroundCamera);

    status.hasDied = true;
    stopLevelMusic();
    checkAndSaveScore();

    if(Config::getAutoRestart())
    {
        status.mustRestart = true;
    }
}

void HexagonGame::incrementDifficulty()
{
    assets.playSound("levelUp.ogg");

    if(levelStatus.shouldIncrement())
    {
        levelStatus.rotationSpeed +=
            levelStatus.rotationSpeedInc * getSign(levelStatus.rotationSpeed);
    }
    levelStatus.rotationSpeed *= -1.f;

    const auto& rotationSpeedMax(levelStatus.rotationSpeedMax);
    if(status.fastSpin < 0 && abs(levelStatus.rotationSpeed) > rotationSpeedMax)
    {
        levelStatus.rotationSpeed =
            rotationSpeedMax * getSign(levelStatus.rotationSpeed);
    }

    status.fastSpin = levelStatus.fastSpin;
}

void HexagonGame::sideChange(unsigned int mSideNumber)
{
    runLuaFunction<void>("onIncrement");

    if(levelStatus.shouldIncrement())
    {
        levelStatus.speedMult += levelStatus.speedInc;
        levelStatus.delayMult += levelStatus.delayInc;
    }

    if(levelStatus.rndSideChangesEnabled)
    {
        setSides(mSideNumber);
    }
    mustChangeSides = false;
}

void HexagonGame::checkAndSaveScore()
{
    if(Config::getInvincible())
    {
        lo("hg::HexagonGame::checkAndSaveScore()")
            << "Not saving score - invincibility on\n";
        return;
    }

    if(assets.pIsLocal())
    {
        string localValidator{getLocalValidator(levelData->id, difficultyMult)};
        if(assets.getLocalScore(localValidator) < status.currentTime)
        {
            assets.setLocalScore(localValidator, status.currentTime);
        }
        assets.saveCurrentLocalProfile();
    }
    else
    {
        if(status.scoreInvalid || !Config::isEligibleForScore())
        {
            lo("hg::HexagonGame::checkAndSaveScore()")
                << "Not sending/saving score - not eligible\n"
                << Config::getUneligibilityReason() << "\n";
            return;
        }
        if(status.currentTime < 8)
        {
            lo("hg::HexagonGame::checkAndSaveScore()")
                << "Not sending score - less than 8 seconds\n";
            return;
        }
        Online::trySendScore(levelData->id, difficultyMult, status.currentTime);
    }
}
void HexagonGame::goToMenu(bool mSendScores)
{
    assets.stopSounds();
    assets.playSound("beep.ogg");
    fpsWatcher.disable();

    if(mSendScores && !status.hasDied)
    {
        checkAndSaveScore();
    }
    runLuaFunction<void>("onUnload");
    window.setGameState(mgPtr->getGame());
    mgPtr->init();
}
void HexagonGame::changeLevel(const string& mId, bool mFirstTime)
{
    newGame(mId, mFirstTime, difficultyMult);
}
void HexagonGame::addMessage(const string& mMessage, float mDuration)
{
    messageTimeline.append<Do>([&, mMessage] {
        assets.playSound("beep.ogg");
        messageText.setString(mMessage);
    });
    messageTimeline.append<Wait>(mDuration);
    messageTimeline.append<Do>([=] { messageText.setString(""); });
}
void HexagonGame::setLevelData(
    const LevelData& mLevelData, bool mMusicFirstPlay)
{
    levelData = &mLevelData;
    levelStatus = LevelStatus{};
    styleData = assets.getStyleData(levelData->styleId);
    musicData = assets.getMusicData(levelData->musicId);
    musicData.firstPlay = mMusicFirstPlay;
}

void HexagonGame::playLevelMusic()
{
    if(!Config::getNoMusic())
    {
        musicData.playRandomSegment(assets);
    }
}
void HexagonGame::stopLevelMusic()
{
    if(!Config::getNoMusic())
    {
        assets.stopMusics();
    }
}

void HexagonGame::invalidateScore()
{
    status.scoreInvalid = true;
    lo("HexagonGame::invalidateScore")
        << "Too much slowdown, invalidating official game\n";
}

auto HexagonGame::getColorMain() const -> Color
{
    if(Config::getBlackAndWhite())
    {
        //			if(status.drawing3D) return Color{255, 255, 255,
        // status.overrideColor.a};
        return Color(255, 255, 255, styleData.getMainColor().a);
    }
    //	else if(status.drawing3D) return status.overrideColor;
    {
        return styleData.getMainColor();
    }
}
void HexagonGame::setSides(unsigned int mSides)
{
    assets.playSound("beep.ogg");
    if(mSides < 3)
    {
        mSides = 3;
    }
    levelStatus.sides = mSides;
}
} // namespace hg
