// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

#include <SSVStart/Utils/Vector2.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVUtils/Core/Utils/Containers.hpp>

#include <optional>

namespace hg
{

void HexagonGame::update(ssvu::FT mFT)
{
    mFT *= Config::getTimescale();

    // ------------------------------------------------------------------------
    // Update Discord and Steam "rich presence".
    // Discord "rich presence" is also updated in `HexagonGame::start`.
    std::string nameStr = levelData->name;
    nameFormat(nameStr);

    const std::string diffStr = diffFormat(difficultyMult);
    const std::string timeStr = timeFormat(status.getTimeSeconds());

    constexpr float DELAY_TO_UPDATE = 5.f; // X seconds
    timeUntilRichPresenceUpdate -= ssvu::getFTToSeconds(mFT);

    if(timeUntilRichPresenceUpdate <= 0.f)
    {
        steamManager.set_rich_presence_in_game(nameStr, diffStr, timeStr);
        timeUntilRichPresenceUpdate = DELAY_TO_UPDATE;
    }

    updateRichPresenceCallbacks();

    // ------------------------------------------------------------------------

    updateText(mFT);

    if(!debugPause)
    {
        updateFlash(mFT);
        effectTimelineManager.update(mFT);

        if(!mustReplayInput())
        {
            updateInput();
        }
        else
        {
            assert(activeReplay.has_value());

            if(!status.started)
            {
                start();
            }

            const input_bitset ib =
                activeReplay->replayPlayer.get_current_and_move_forward();

            if(ib[static_cast<unsigned int>(input_bit::left)])
            {
                inputMovement = -1;
            }
            else if(ib[static_cast<unsigned int>(input_bit::right)])
            {
                inputMovement = 1;
            }
            else
            {
                inputMovement = 0;
            }

            inputSwap = ib[static_cast<unsigned int>(input_bit::swap)];
            inputFocused = ib[static_cast<unsigned int>(input_bit::focus)];
        }

        // ------------------------------------------------------------------------
        // Update key icons.
        if(Config::getShowKeyIcons() || mustShowReplayUI())
        {
            updateKeyIcons();
        }

        // ------------------------------------------------------------------------
        // Update level info.
        if(Config::getShowLevelInfo() || mustShowReplayUI())
        {
            updateLevelInfo();
        }

        // ------------------------------------------------------------------------
        // Update input leniency time after death to avoid accidental restart.
        if(deathInputIgnore > 0.f)
        {
            deathInputIgnore -= mFT;
        }

        // ------------------------------------------------------------------------
        if(status.started)
        {
            player.update(getInputFocused(), getLevelStatus().swapEnabled, mFT);

            if(!status.hasDied)
            {
                const std::optional<bool> preventPlayerInput =
                    runLuaFunctionIfExists<bool, float, int, bool, bool>(
                        "onInput", mFT, getInputMovement(), getInputFocused(),
                        getInputSwap());

                if(!preventPlayerInput.has_value() || !(*preventPlayerInput))
                {
                    player.updateInputMovement(getInputMovement(),
                        getPlayerSpeedMult(), getInputFocused(), mFT);

                    if(getLevelStatus().swapEnabled && getInputSwap() &&
                        player.isReadyToSwap())
                    {
                        performPlayerSwap(true /* mPlaySound */);
                        player.resetSwap(getSwapCooldown());
                        player.setJustSwapped(true);
                    }
                    else
                    {
                        player.setJustSwapped(false);
                    }
                }

                status.accumulateFrametime(mFT);
                if(levelStatus.scoreOverridden)
                {
                    status.updateCustomScore(
                        lua.readVariable<float>(levelStatus.scoreOverride));
                }

                updateEvents(mFT);
                updateIncrement();

                if(mustChangeSides && walls.empty())
                {
                    sideChange(rng.get_int(
                        levelStatus.sidesMin, levelStatus.sidesMax));
                }

                updateLevel(mFT);

                if(Config::getBeatPulse())
                {
                    updateBeatPulse(mFT);
                }

                if(Config::getPulse())
                {
                    updatePulse(mFT);
                }

                if(!Config::getBlackAndWhite())
                {
                    styleData.update(mFT, pow(difficultyMult, 0.8f));
                }

                player.updatePosition(getRadius());

                updateWalls(mFT);
                ssvu::eraseRemoveIf(
                    walls, [](const CWall& w) { return w.isDead(); });

                updateCustomWalls(mFT);
            }
            else
            {
                levelStatus.rotationSpeed *= 0.99f;
            }

            if(Config::get3D())
            {
                update3D(mFT);
            }

            if(!Config::getNoRotation())
            {
                updateRotation(mFT);
            }
        }

        updateParticles(mFT);

        overlayCamera.update(mFT);
        backgroundCamera.update(mFT);
    }

    if(status.started)
    {
        if(status.mustStateChange != StateChange::None)
        {
            fpsWatcher.disable();

            const bool executeLastReplay =
                status.mustStateChange == StateChange::MustReplay;

            if(!executeLastReplay && !assets.anyLocalProfileActive())
            {
                // If playing a replay from file, there is no local profile
                // active, so just go to the menu when attempting to restart the
                // level.

                goToMenu();
                return;
            }

            newGame(getPackId(), restartId, restartFirstTime, difficultyMult,
                executeLastReplay);
        }

        if(!status.scoreInvalid && Config::getOfficial() &&
            fpsWatcher.isLimitReached())
        {
            invalidateScore("PERFORMANCE ISSUES");
        }
        else if(!status.scoreInvalid && !Config::get3D() &&
                levelStatus._3DRequired)
        {
            invalidateScore("3D REQUIRED");
        }

        fpsWatcher.update();
    }
}

void HexagonGame::updateWalls(ssvu::FT mFT)
{
    bool collided{false};
    const float radiusSquared{status.radius * status.radius + 8.f};
    const sf::Vector2f& pPos{player.getPosition()};

    for(CWall& w : walls)
    {
        w.update(levelStatus.wallSpawnDistance, getRadius(), centerPos, mFT);

        // If there is no collision skip to the next wall.
        if(!w.isOverlapping(pPos))
        {
            continue;
        }

        // Kill after a swap or if player could not be pushed out to safety.
        if(player.getJustSwapped())
        {
            performPlayerKill();
            steamManager.unlock_achievement("a22_swapdeath");
        }
        else if(player.push(getInputMovement(), getRadius(), w, centerPos,
                    radiusSquared, mFT))
        {
            performPlayerKill();
        }

        collided = true;
    }

    // There was no collision, so we can stop here.
    if(!collided)
    {
        return;
    }

    // Second round, always deadly...
    for(CWall& w : walls)
    {
        if(!w.isOverlapping(pPos))
        {
            continue;
        }

        if(player.getJustSwapped())
        {
            steamManager.unlock_achievement("a22_swapdeath");
        }

        performPlayerKill();
    }
}

void HexagonGame::updateCustomWalls(ssvu::FT mFT)
{
    if(cwManager.handleCollision(getInputMovement(), getRadius(), player, mFT))
    {
        performPlayerKill();

        if(player.getJustSwapped())
        {
            steamManager.unlock_achievement("a22_swapdeath");
        }
    }
}

void HexagonGame::start()
{
    status.start();
    messageText.setString("");
    assets.playSound("go.ogg");

    if(!mustReplayInput())
    {
        std::string nameStr = levelData->name;
        nameFormat(nameStr);

        std::string packStr = getPackName();
        nameFormat(packStr);

        const std::string diffStr = diffFormat(difficultyMult);

        discordManager.set_rich_presence_in_game(
            nameStr + " [x" + diffStr + "]", packStr);
    }
    else
    {
        discordManager.set_rich_presence_on_replay();
    }

    if(!Config::getNoMusic())
    {
        assets.musicPlayer.resume();
    }

    if(Config::getOfficial())
    {
        fpsWatcher.enable();
    }

    runLuaFunctionIfExists<void>("onLoad");
}

void HexagonGame::updateInput()
{
    if(imguiLuaConsoleHasInput())
    {
        return;
    }

    // Joystick support
    Joystick::update();

    const bool jCW = Joystick::pressed(Joystick::Jdir::Right);
    const bool jCCW = Joystick::pressed(Joystick::Jdir::Left);

    if(!status.started && (!Config::getRotateToStart() || inputImplCCW ||
                              inputImplCW || inputImplBothCWCCW || jCW || jCCW))
    {
        start();
    }

    // Naive touch controls
    for(const auto& p : window.getFingerDownPositions())
    {
        if(p.x < window.getWidth() / 2.f)
        {
            inputImplCCW = 1;
        }
        else
        {
            inputImplCW = 1;
        }
    }

    if(inputImplCW && !inputImplCCW)
    {
        inputMovement = 1;
    }
    else if(!inputImplCW && inputImplCCW)
    {
        inputMovement = -1;
    }
    else if(inputImplCW && inputImplCCW)
    {
        if(!inputImplBothCWCCW)
        {
            if(inputMovement == 1 && inputImplLastMovement == 1)
            {
                inputMovement = -1;
            }
            else if(inputMovement == -1 && inputImplLastMovement == -1)
            {
                inputMovement = 1;
            }
        }
    }
    else
    {
        inputMovement = 0;

        // Joystick support
        {
            if(jCW && !jCCW)
            {
                inputMovement = 1;
            }
            else if(!jCW && jCCW)
            {
                inputMovement = -1;
            }
            else if(jCW && jCCW)
            {
                if(!inputImplBothCWCCW)
                {
                    if(inputMovement == 1 && inputImplLastMovement == 1)
                    {
                        inputMovement = -1;
                    }
                    else if(inputMovement == -1 && inputImplLastMovement == -1)
                    {
                        inputMovement = 1;
                    }
                }
            }
            else
            {
                inputMovement = 0;
            }
        }
    }

    // Replay support
    if(status.started && !status.hasDied)
    {
        const bool left = getInputMovement() == -1;
        const bool right = getInputMovement() == 1;
        const bool swap = getInputSwap();
        const bool focus = getInputFocused();

        lastReplayData.record_input(left, right, swap, focus);
    }

    // Joystick support
    if(Joystick::risingEdge(Joystick::Jid::Exit))
    {
        goToMenu();
    }
    else if(Joystick::risingEdge(Joystick::Jid::ForceRestart) ||
            (status.hasDied && Joystick::risingEdge(Joystick::Jid::Restart)))
    {
        status.mustStateChange = StateChange::MustRestart;
    }
    else if(status.hasDied && Joystick::risingEdge(Joystick::Jid::Replay))
    {
        status.mustStateChange = StateChange::MustReplay;
    }
}

void HexagonGame::updateEvents(ssvu::FT)
{
    if(const auto o =
            eventTimelineRunner.update(eventTimeline, status.getTimeTP());
        o == hg::Utils::timeline2_runner::outcome::finished)
    {
        eventTimeline.clear();
        eventTimelineRunner = {};
    }

    if(const auto o = messageTimelineRunner.update(
           messageTimeline, status.getCurrentTP());
        o == hg::Utils::timeline2_runner::outcome::finished)
    {
        messageTimeline.clear();
        messageTimelineRunner = {};
    }
}

void HexagonGame::updateIncrement()
{
    if(!levelStatus.incEnabled)
    {
        return;
    }

    if(status.getIncrementTimeSeconds() < levelStatus.incTime)
    {
        return;
    }

    ++levelStatus.currentIncrements;
    incrementDifficulty();
    status.resetIncrementTime();
    mustChangeSides = true;
}

void HexagonGame::updateLevel(ssvu::FT mFT)
{
    if(status.isTimePaused())
    {
        return;
    }

    runLuaFunctionIfExists<float>("onUpdate", mFT);

    const auto o = timelineRunner.update(timeline, status.getTimeTP());

    if(o == hg::Utils::timeline2_runner::outcome::finished && !mustChangeSides)
    {
        timeline.clear();
        runLuaFunctionIfExists<void>("onStep");
        timelineRunner = {};
    }
}

void HexagonGame::updatePulse(ssvu::FT mFT)
{
    if(!levelStatus.manualPulseControl)
    {
        if(status.pulseDelay <= 0 && status.pulseDelayHalf <= 0)
        {
            const float pulseAdd{status.pulseDirection > 0
                                     ? levelStatus.pulseSpeed
                                     : -levelStatus.pulseSpeedR};

            const float pulseLimit{status.pulseDirection > 0
                                       ? levelStatus.pulseMax
                                       : levelStatus.pulseMin};

            status.pulse += pulseAdd * mFT * getMusicDMSyncFactor();

            if((status.pulseDirection > 0 && status.pulse >= pulseLimit) ||
                (status.pulseDirection < 0 && status.pulse <= pulseLimit))
            {
                status.pulse = pulseLimit;
                status.pulseDirection *= -1;
                status.pulseDelayHalf = levelStatus.pulseDelayHalfMax;

                if(status.pulseDirection < 0)
                {
                    status.pulseDelay = levelStatus.pulseDelayMax;
                }
            }
        }

        status.pulseDelay -= mFT * getMusicDMSyncFactor();
        status.pulseDelayHalf -= mFT * getMusicDMSyncFactor();
    }

    const float p{status.pulse / levelStatus.pulseMin};
    const float rotation{backgroundCamera.getRotation()};

    backgroundCamera.setView({ssvs::zeroVec2f,
        {(Config::getWidth() * Config::getZoomFactor()) * p,
            (Config::getHeight() * Config::getZoomFactor()) * p}});

    backgroundCamera.setRotation(rotation);
}

void HexagonGame::updateBeatPulse(ssvu::FT mFT)
{
    if(!levelStatus.manualBeatPulseControl)
    {
        if(status.beatPulseDelay <= 0)
        {
            status.beatPulse = levelStatus.beatPulseMax;
            status.beatPulseDelay = levelStatus.beatPulseDelayMax;
        }
        else
        {
            status.beatPulseDelay -= mFT * getMusicDMSyncFactor();
        }

        if(status.beatPulse > 0)
        {
            status.beatPulse -= (2.f * mFT * getMusicDMSyncFactor()) *
                                levelStatus.beatPulseSpeedMult;
        }
    }

    const float radiusMin{Config::getBeatPulse() ? levelStatus.radiusMin : 75};
    status.radius =
        radiusMin * (status.pulse / levelStatus.pulseMin) + status.beatPulse;
}

void HexagonGame::updateRotation(ssvu::FT mFT)
{
    auto nextRotation(getRotationSpeed() * 10.f);
    if(status.fastSpin > 0)
    {
        nextRotation += std::abs((Utils::getSmootherStep(0,
                                      levelStatus.fastSpin, status.fastSpin) /
                                     3.5f) *
                                 17.f) *
                        ssvu::getSign(nextRotation);

        status.fastSpin -= mFT;
    }

    backgroundCamera.turn(nextRotation);
}

void HexagonGame::updateFlash(ssvu::FT mFT)
{
    if(status.flashEffect > 0)
    {
        status.flashEffect -= 3 * mFT;
    }

    status.flashEffect = ssvu::getClamped(status.flashEffect, 0.f, 255.f);

    for(auto i(0u); i < 4; ++i)
    {
        flashPolygon[i].color.a = status.flashEffect;
    }
}

void HexagonGame::update3D(ssvu::FT mFT)
{
    status.pulse3D += styleData._3dPulseSpeed * status.pulse3DDirection * mFT;
    if(status.pulse3D > styleData._3dPulseMax)
    {
        status.pulse3DDirection = -1.f;
    }
    else if(status.pulse3D < styleData._3dPulseMin)
    {
        status.pulse3DDirection = 1.f;
    }
}

void HexagonGame::updateParticles(ssvu::FT mFT)
{
    const auto isOutOfBounds = [](const Particle& p) {
        const sf::Sprite& sp = p.sprite;
        const sf::Vector2f& pos = sp.getPosition();
        constexpr float padding = 256.f;

        return (pos.x < 0 - padding || pos.x > Config::getWidth() + padding ||
                pos.y < 0 - padding || pos.y > Config::getHeight() + padding);
    };

    const auto makePBParticle = [this] {
        Particle p;

        p.sprite.setTexture(assets.get<sf::Texture>("starParticle.png"));
        p.sprite.setPosition(
            {ssvu::getRndR(-64.f, Config::getWidth() + 64.f), -64.f});
        p.sprite.setRotation(ssvu::getRndR(0.f, 360.f));

        const float scale = ssvu::getRndR(0.75f, 1.35f);
        p.sprite.setScale({scale, scale});

        sf::Color c = getColorMain();
        c.a = ssvu::getRndI(90, 145);
        p.sprite.setColor(c);

        p.velocity = {ssvu::getRndR(-12.f, 12.f), ssvu::getRndR(4.f, 18.f)};
        p.angularVelocity = ssvu::getRndR(-6.f, 6.f);

        return p;
    };

    ssvu::eraseRemoveIf(particles, isOutOfBounds);

    for(Particle& p : particles)
    {
        sf::Sprite& sp = p.sprite;
        sp.setPosition(sp.getPosition() + p.velocity * mFT);
        sp.setRotation(sp.getRotation() + p.angularVelocity * mFT);
    }

    if(mustSpawnPBParticles)
    {
        nextPBParticleSpawn -= mFT;
        if(nextPBParticleSpawn <= 0.f)
        {
            particles.emplace_back(makePBParticle());
            nextPBParticleSpawn = 2.75f;
        }
    }
}

static int ilcTextEditCallbackStub(ImGuiInputTextCallbackData* data)
{
    auto hg = (HexagonGame*)data->UserData;
    return hg->ilcTextEditCallback(data);
}

static int Stricmp(const char* s1, const char* s2)
{
    int d;
    while((d = toupper(*s2) - toupper(*s1)) == 0 && *s1)
    {
        s1++;
        s2++;
    }
    return d;
}

static int Strnicmp(const char* s1, const char* s2, int n)
{
    int d = 0;
    while(n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1)
    {
        s1++;
        s2++;
        n--;
    }
    return d;
}

int HexagonGame::ilcTextEditCallback(ImGuiInputTextCallbackData* data)
{
    switch(data->EventFlag)
    {
        case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while(word_start > data->Buf)
            {
                const char c = word_start[-1];
                if(c == ' ' || c == '\t' || c == ',' || c == ';')
                {
                    break;
                }

                word_start--;
            }

            // Skip starting `?` for Lua docs
            if(*word_start == '?')
            {
                ++word_start;
            }

            // Build a list of candidates
            ImVector<const char*> candidates;
            for(const std::string& fnName : getAllLuaFunctionNames())
            {
                if(Strnicmp(fnName.c_str(), word_start,
                       (int)(word_end - word_start)) == 0)
                {
                    candidates.push_back(fnName.c_str());
                }
            }

            if(candidates.Size == 0)
            {
                char buf[255];
                std::snprintf(buf, sizeof(buf), "No match for \"%.*s\"!\n",
                    (int)(word_end - word_start), word_start);

                // No match
                ilcCmdLog.emplace_back(buf);
            }
            else if(candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it
                // entirely so we've got nice casing.
                data->DeleteChars((int)(word_start - data->Buf),
                    (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display
                // "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int)(word_end - word_start);
                for(;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for(int i = 0;
                        i < candidates.Size && all_candidates_matches; i++)
                        if(i == 0)
                            c = std::toupper(candidates[i][match_len]);
                        else if(c == 0 ||
                                c != std::toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if(!all_candidates_matches) break;
                    match_len++;
                }

                if(match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf),
                        (int)(word_end - word_start));

                    data->InsertChars(data->CursorPos, candidates[0],
                        candidates[0] + match_len);
                }

                // List matches
                ilcCmdLog.emplace_back("Possible matches:\n");
                for(int i = 0; i < candidates.Size; i++)
                {
                    ilcCmdLog.emplace_back(
                        std::string{"- "} + candidates[i] + '\n');
                }
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory:
        {
            const int prev_history_pos = ilcHistoryPos;
            if(data->EventKey == ImGuiKey_UpArrow)
            {
                if(ilcHistoryPos == -1)
                {
                    ilcHistoryPos = ilcHistory.size() - 1;
                }
                else if(ilcHistoryPos > 0)
                {
                    ilcHistoryPos--;
                }
            }
            else if(data->EventKey == ImGuiKey_DownArrow)
            {
                if(ilcHistoryPos != -1)
                {
                    if(++ilcHistoryPos >= static_cast<int>(ilcHistory.size()))
                    {
                        ilcHistoryPos = -1;
                    }
                }
            }

            if(prev_history_pos != ilcHistoryPos)
            {
                const char* history_str =
                    (ilcHistoryPos >= 0) ? ilcHistory[ilcHistoryPos].c_str()
                                         : "";

                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
    }

    return 0;
}

void HexagonGame::postUpdateImguiLuaConsole()
{
    if(ilcShowConsoleNext)
    {
        ilcShowConsole = !ilcShowConsole;
        ilcShowConsoleNext = false;

        ImGui::SFML::ProcessEvent(sf::Event{sf::Event::GainedFocus});
    }

    if(!ilcShowConsole)
    {
        return;
    }

    ImGui::SFML::Update(window, ilcDeltaClock.restart());

    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Lua Console");

    ImGui::Text("Enter `!help` to show help.");
    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y +
                                           ImGui::GetFrameHeightWithSpacing() +
                                           150;

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve),
        false, ImGuiWindowFlags_HorizontalScrollbar);

    for(decltype(ilcCmdLog.size()) i = 0; i < ilcCmdLog.size(); ++i)
    {
        const std::string& sItem = ilcCmdLog[i];
        const char* item = sItem.c_str();

        std::optional<ImVec4> color;

        if(std::strstr(item, "[error]"))
        {
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        }
        else if(std::strstr(item, "[lua]"))
        {
            color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
        }
        else if(std::strncmp(item, "# ", 2) == 0)
        {
            color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
        }
        else if(std::strstr(item, "[?]"))
        {
            color = ImVec4(0.4f, 0.4f, 1.0f, 1.0f);
        }

        if(color.has_value())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, *color);
        }

        std::vector<std::string> split;
        std::size_t last = 0;

        for(std::size_t j = 0; j < sItem.size(); ++j)
        {
            if(sItem[j] == '\n')
            {
                split.emplace_back(sItem.substr(last, j - last));
                last = j + 1;
            }
        }

        const std::string lastPiece = sItem.substr(last);
        if(!lastPiece.empty())
        {
            split.emplace_back(lastPiece);
        }

        for(const std::string& s : split)
        {
            constexpr std::size_t lineLimit = 80;
            if(s.size() <= lineLimit)
            {
                ImGui::TextUnformatted(s.c_str());
            }
            else
            {
                constexpr std::size_t charsPerSubstr = lineLimit;
                const std::size_t nSubstrs = s.size() / charsPerSubstr;

                for(std::size_t j = 0; j < nSubstrs + 1; ++j)
                {
                    ImGui::TextUnformatted(
                        s.substr(j * charsPerSubstr, charsPerSubstr).c_str());
                }
            }
        }

        if(color.has_value())
        {
            ImGui::PopStyleColor();
        }
    }

    ImGui::SetScrollHereY(1.0f);
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    ImGuiInputTextFlags input_text_flags =
        ImGuiInputTextFlags_EnterReturnsTrue |
        ImGuiInputTextFlags_CallbackCompletion |
        ImGuiInputTextFlags_CallbackHistory;

    if(ImGui::InputText("Command", ilcCmdBuffer, IM_ARRAYSIZE(ilcCmdBuffer),
           input_text_flags, &ilcTextEditCallbackStub, (void*)this))
    {
        const std::string cmdString = ilcCmdBuffer;
        ilcCmdLog.emplace_back(std::string{"# "} + cmdString + '\n');

        ilcHistoryPos = -1;
        for(int i = ilcHistory.size() - 1; i >= 0; --i)
        {
            if(ilcHistory[i] == cmdString)
            {
                ilcHistory.erase(ilcHistory.begin() + i);
                break;
            }
        }

        ilcHistory.emplace_back(cmdString);

        if(Stricmp(cmdString.c_str(), "!CLEAR") == 0)
        {
            ilcCmdLog.clear();
        }
        else if(Stricmp(cmdString.c_str(), "!HELP") == 0)
        {
            ilcCmdLog.emplace_back(R"(Built-in commands:
!clear          Clears the console
!help           Display this help
?fn             Display Lua docs for function `fn`
)");
        }
        else if(cmdString[0] == '?')
        {
            const std::string rest = Utils::getRTrim(cmdString.substr(1));
            const std::string docs = getDocsForLuaFunction(rest);
            ilcCmdLog.emplace_back(std::string{"[?]: "} + docs);
        }
        else
        {
            try
            {
                try
                {
                    lua.executeCode(std::string{"u_log("} + cmdString + ")\n");
                }
                catch(std::runtime_error& mError)
                {
                    lua.executeCode(cmdString + "\n");
                }
            }
            catch(std::runtime_error& mError)
            {
                std::string temp = "[error]: ";
                temp += mError.what();
                temp += '\n';

                ilcCmdLog.emplace_back(temp);
            }
            catch(...)
            {
                ilcCmdLog.emplace_back("[error]: unknown\n");
            }
        }

        std::strcpy(ilcCmdBuffer, "");
        ImGui::SetItemDefaultFocus();
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::Separator();

    ImGui::Text("update ms: %.2f", window.getMsUpdate());
    ImGui::SameLine();
    ImGui::Text("draw ms: %.2f", window.getMsDraw());

    static float simSpeed = Config::getTimescale();
    ImGui::DragFloat("Timescale", &simSpeed, 0.005f);
    Config::setTimescale(simSpeed);

    ImGui::SameLine();

    static bool invincible = Config::getInvincible();
    ImGui::Checkbox("Invincible", &invincible);
    Config::setInvincible(invincible);

    ImGui::Separator();

    {
        if(ImGui::InputText("Track", ilcTrackBuffer,
               IM_ARRAYSIZE(ilcTrackBuffer),
               ImGuiInputTextFlags_EnterReturnsTrue))
        {
            const std::string codeToTrack = Utils::getLRTrim(ilcTrackBuffer);
            ilcLuaTracked.emplace_back(
                std::string{"u_impl_addTrackedResult("} + codeToTrack + ")\n");
            ilcLuaTrackedNames.emplace_back(codeToTrack);

            std::strcpy(ilcTrackBuffer, "");
            ImGui::SetItemDefaultFocus();
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::SameLine();

        if(ImGui::Button("Untrack All"))
        {
            ilcLuaTracked.clear();
            ilcLuaTrackedNames.clear();
        }
    }

    if(ilcLuaTracked.size() > 0)
    {
        ilcLuaTrackedResults.clear();
        bool problem = false;

        for(std::size_t i = 0; i < ilcLuaTracked.size(); ++i)
        {
            const std::string& code = ilcLuaTracked[i];

            try
            {
                lua.executeCode(code);
            }
            catch(std::runtime_error& e)
            {
                ilcCmdLog.emplace_back(std::string{"[error]: error '"} +
                                       e.what() + "' while tracking " + code +
                                       '\n');

                ilcLuaTracked.erase(ilcLuaTracked.begin() + i);
                ilcLuaTrackedNames.erase(ilcLuaTrackedNames.begin() + i);
                problem = true;
                break;
            }
            catch(...)
            {

                ilcCmdLog.emplace_back(
                    std::string{"[error]: unknown error while tracking "} +
                    code + '\n');

                ilcLuaTracked.erase(ilcLuaTracked.begin() + i);
                ilcLuaTrackedNames.erase(ilcLuaTrackedNames.begin() + i);
                problem = true;
                break;
            }
        }

        if(!problem)
        {
            ImGui::Separator();
            assert(ilcLuaTracked.size() == ilcLuaTrackedNames.size());
            assert(ilcLuaTracked.size() == ilcLuaTrackedResults.size());

            if(ImGui::BeginTable("TrackedResults", 2))
            {
                for(std::size_t i = 0; i < ilcLuaTracked.size(); ++i)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(ilcLuaTrackedNames[i].c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(ilcLuaTrackedResults[i].c_str());
                }

                ImGui::EndTable();
            }
        }
    }

    ImGui::End();
}

} // namespace hg
