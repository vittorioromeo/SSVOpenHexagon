// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/SoundPlayer/SoundPlayer.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

using namespace hg::Utils;
namespace hg
{

void HexagonGame::createWall(int mSide, float mThickness,
    const SpeedData& mSpeed, const SpeedData& mCurve, float mHueMod)
{
    walls.emplace_back(*this, centerPos, mSide, mThickness,
        Config::getSpawnDistance(), mSpeed, mCurve);

    walls.back().setHueMod(mHueMod);
}

HexagonGame::HexagonGame(Steam::steam_manager& mSteamManager,
    Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
    ssvs::GameWindow& mGameWindow)
    : steamManager(mSteamManager), discordManager(mDiscordManager),
      assets(mAssets), window(mGameWindow), player{ssvs::zeroVec2f},
      fpsWatcher(window)
{
    game.onUpdate += [this](ssvu::FT mFT) { update(mFT); };
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
        Config::getTriggerExit(), [this](ssvu::FT /*unused*/) { goToMenu(); });
    game.addInput(
        Config::getTriggerForceRestart(),
        [this](ssvu::FT /*unused*/) { status.mustRestart = true; },
        ssvs::Input::Type::Once);
    game.addInput(
        Config::getTriggerRestart(),
        [this](ssvu::FT /*unused*/) {
            if(status.hasDied)
            {
                status.mustRestart = true;
            }
        },
        ssvs::Input::Type::Once);
    game.addInput(
        Config::getTriggerScreenshot(),
        [this](ssvu::FT /*unused*/) { mustTakeScreenshot = true; },
        ssvs::Input::Type::Once);
}

void HexagonGame::newGame(const std::string& mPackId, const std::string& mId,
    bool mFirstPlay, float mDifficultyMult)
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

    // Event timeline cleanup
    eventTimeline.clear();
    eventTimelineRunner = {};

    // Message timeline cleanup
    messageTimeline.clear();
    messageTimelineRunner = {};

    // Manager cleanup
    walls.clear();
    cwManager.clear();
    player = CPlayer{ssvs::zeroVec2f};

    // Timeline cleanup
    timeline.clear();
    timelineRunner = {};

    effectTimelineManager.clear();
    mustChangeSides = false;

    // FPSWatcher reset
    fpsWatcher.reset();
    // if(Config::getOfficial()) fpsWatcher.enable();

    // Reset zoom
    overlayCamera.setView(
        {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
            sf::Vector2f(Config::getWidth(), Config::getHeight())});
    backgroundCamera.setView(
        {ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(),
                              Config::getHeight() * Config::getZoomFactor()}});
    backgroundCamera.setRotation(0);

    // Reset skew
    overlayCamera.setSkew(sf::Vector2f{1.f, 1.f});
    backgroundCamera.setSkew(sf::Vector2f{1.f, 1.f});

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
}

void HexagonGame::death(bool mForce)
{
    fpsWatcher.disable();
    assets.playSound("death.ogg", ssvs::SoundPlayer::Mode::Abort);

    if(!mForce && (Config::getInvincible() || levelStatus.tutorialMode))
    {
        return;
    }
    assets.playSound("gameOver.ogg", ssvs::SoundPlayer::Mode::Abort);

    if(!assets.pIsLocal() && Config::isEligibleForScore())
    {
        Online::trySendDeath();
    }

    status.flashEffect = 255;
    overlayCamera.setView(
        {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
            sf::Vector2f(Config::getWidth(), Config::getHeight())});
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

    const float signMult = (levelStatus.rotationSpeed > 0.f) ? 1.f : -1.f;

    levelStatus.rotationSpeed +=
        levelStatus.rotationSpeedInc * signMult;

    levelStatus.rotationSpeed *= -1.f;

    const auto& rotationSpeedMax(levelStatus.rotationSpeedMax);
    if(status.fastSpin < 0 && abs(levelStatus.rotationSpeed) > rotationSpeedMax)
    {
        levelStatus.rotationSpeed =
            rotationSpeedMax * signMult;
    }

    status.fastSpin = levelStatus.fastSpin;
}

void HexagonGame::sideChange(unsigned int mSideNumber)
{
    runLuaFunction<void>("onIncrement");

    levelStatus.speedMult += levelStatus.speedInc;
    levelStatus.delayMult += levelStatus.delayInc;

    if(levelStatus.rndSideChangesEnabled)
    {
        setSides(mSideNumber);
    }
    mustChangeSides = false;
}

void HexagonGame::checkAndSaveScore()
{
    const float time = status.getTimeSeconds();

    if(Config::getInvincible())
    {
        ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
            << "Not saving score - invincibility on\n";
        return;
    }

    if(assets.pIsLocal())
    {
        std::string localValidator{
            getLocalValidator(levelData->id, difficultyMult)};
        if(assets.getLocalScore(localValidator) < time)
        {
            assets.setLocalScore(localValidator, time);
        }
        assets.saveCurrentLocalProfile();
    }
    else
    {
        if(status.scoreInvalid || !Config::isEligibleForScore())
        {
            ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
                << "Not sending/saving score - not eligible\n"
                << Config::getUneligibilityReason() << "\n";
            return;
        }
        if(time < 8)
        {
            ssvu::lo("hg::HexagonGame::checkAndSaveScore()")
                << "Not sending score - less than 8 seconds\n";
            return;
        }
        Online::trySendScore(levelData->id, difficultyMult, time);
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

void HexagonGame::changeLevel(
    const std::string& mPackId, const std::string& mId, bool mFirstTime)
{
    newGame(mPackId, mId, mFirstTime, difficultyMult);
}

void HexagonGame::addMessage(
    std::string mMessage, double mDuration, bool mSoundToggle)
{
    Utils::uppercasify(mMessage);

    messageTimeline.append_do([this, mSoundToggle, mMessage] {
        if(mSoundToggle)
        {
            assets.playSound("beep.ogg");
        }
        messageText.setString(mMessage);
    });

    messageTimeline.append_wait_for_sixths(mDuration);
    messageTimeline.append_do([this] { messageText.setString(""); });
}

void HexagonGame::clearMessages()
{
    messageTimeline.clear();
}

void HexagonGame::setLevelData(
    const LevelData& mLevelData, bool mMusicFirstPlay)
{
    levelData = &mLevelData;
    levelStatus = LevelStatus{};
    styleData = assets.getStyleData(levelData->packId, levelData->styleId);
    musicData = assets.getMusicData(levelData->packId, levelData->musicId);
    musicData.firstPlay = mMusicFirstPlay;
}

[[nodiscard]] const std::string& HexagonGame::getPackId() const
{
    return levelData->packId;
}

void HexagonGame::playLevelMusic()
{
    if(!Config::getNoMusic())
    {
        musicData.playRandomSegment(getPackId(), assets);
    }
}

void HexagonGame::playLevelMusicAtTime(float mSeconds)
{
    if(!Config::getNoMusic())
    {
        musicData.playSeconds(getPackId(), assets, mSeconds);
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
    ssvu::lo("HexagonGame::invalidateScore")
        << "Too much slowdown, invalidating official game\n";
}

auto HexagonGame::getColorMain() const -> sf::Color
{
    if(Config::getBlackAndWhite())
    {
        //			if(status.drawing3D) return Color{255, 255, 255,
        // status.overrideColor.a};
        return sf::Color(255, 255, 255, styleData.getMainColor().a);
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

bool HexagonGame::getInputSwap() const
{
    return inputSwap || hg::Joystick::aRisingEdge();
}

} // namespace hg
