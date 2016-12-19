// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Input;
using namespace ssvms;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{
    using s = States;
    using ocs = Online::ConnectStat;
    using ols = Online::LoginStat;

    MenuGame::MenuGame(
        HGAssets& mAssets, HexagonGame& mHexagonGame, GameWindow& mGameWindow)
        : assets(mAssets), hexagonGame(mHexagonGame), window(mGameWindow)
    {
        initAssets();
        refreshCamera();

        game.onUpdate += [this](FT mFT)
        {
            update(mFT);
        };
        game.onDraw += [this]
        {
            draw();
        };
        game.onEvent(Event::EventType::TextEntered) +=
            [this](const Event& mEvent)
        {
            if(mEvent.text.unicode < 128)
                enteredChars.emplace_back(toNum<char>(mEvent.text.unicode));
        };
        game.onEvent(Event::EventType::MouseWheelMoved) +=
            [this](const Event& mEvent)
        {
            wheelProgress += mEvent.mouseWheel.delta;
            if(wheelProgress > 2.f)
            {
                wheelProgress = 0.f;
                upAction();
            }
            else if(wheelProgress < -2.f)
            {
                wheelProgress = 0.f;
                downAction();
            }
        };
        window.onRecreation += [this]
        {
            refreshCamera();
        };

        levelDataIds = assets.getLevelIdsByPack(assets.getPackPaths()[packIdx]);
        setIndex(0);
        initMenus();
        initInput();
    }

    void MenuGame::init()
    {
        assets.stopMusics();
        assets.stopSounds();
        assets.playSound("openHexagon.ogg");
        Online::setForceLeaderboardRefresh(true);
    }
    void MenuGame::initAssets()
    {
        for(const auto& t : {"titleBar.png", "creditsBar1.png",
                "creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png",
                "creditsBar2d.png", "bottomBar.png"})
            assets.get<Texture>(t).setSmooth(true);
    }

    void MenuGame::initMenus()
    {
        namespace i = ssvms::Items;

        auto whenLocal = [this]
        {
            return assets.pIsLocal();
        };
        auto whenNotLocal = [this]
        {
            return !assets.pIsLocal();
        };
        auto whenNotOfficial = []
        {
            return !Config::getOfficial();
        };
        auto whenDisconnected = []
        {
            return Online::getConnectionStatus() == ocs::Disconnected;
        };
        auto whenConnectedAndUnlogged = []
        {
            return Online::getConnectionStatus() == ocs::Connected &&
                   Online::getLoginStatus() == ols::Unlogged;
        };
        auto whenConnectedAndLogged = []
        {
            return Online::getConnectionStatus() == ocs::Connected &&
                   Online::getLoginStatus() == ols::Logged;
        };
        auto whenUnlogged = []
        {
            return Online::getLoginStatus() == ols::Unlogged;
        };
        auto whenSoundEnabled = []
        {
            return !Config::getNoSound();
        };
        auto whenMusicEnabled = []
        {
            return !Config::getNoMusic();
        };
        auto whenTimerIsStatic = []
        {
            return Config::getTimerStatic();
        };
        auto whenTimerIsDynamic = []
        {
            return !Config::getTimerStatic();
        };

        // Welcome menu
        auto& wlcm(welcomeMenu.createCategory("welcome"));
        wlcm.create<i::Single>("connect",
            [this]
            {
                Online::tryConnectToServer();
            }) |
            whenDisconnected;
        wlcm.create<i::Single>("login",
            [this]
            {
                assets.pSaveCurrent();
                assets.pSetPlayingLocally(false);
                enteredStr = "";
                state = s::ETUser;
            }) |
            whenConnectedAndUnlogged;
        wlcm.create<i::Single>("logout",
            [this]
            {
                Online::logout();
            }) |
            whenConnectedAndLogged;
        wlcm.create<i::Single>("play locally",
            [this]
            {
                assets.pSaveCurrent();
                assets.pSetPlayingLocally(true);
                enteredStr = "";
                state = assets.getLocalProfilesSize() == 0 ? s::ETLPNew
                                                           : s::SLPSelect;
            }) |
            whenUnlogged;
        wlcm.create<i::Single>("exit game", [this]
            {
                window.stop();
            });

        // Options menu
        auto& main(optionsMenu.createCategory("options"));
        auto& friends(optionsMenu.createCategory("friends"));
        auto& play(optionsMenu.createCategory("gameplay"));
        auto& resolution(optionsMenu.createCategory("resolution"));
        auto& gfx(optionsMenu.createCategory("graphics"));
        auto& sfx(optionsMenu.createCategory("audio"));
        auto& debug(optionsMenu.createCategory("debug"));
        auto& localProfiles(optionsMenu.createCategory("local profiles"));

        main.create<i::Goto>("friends", friends) | whenNotLocal;
        main.create<i::Goto>("gameplay", play);
        main.create<i::Goto>("resolution", resolution);
        main.create<i::Goto>("graphics", gfx);
        main.create<i::Goto>("audio", sfx);
        main.create<i::Goto>("debug", debug) | whenNotOfficial;
        main.create<i::Goto>("local profiles", localProfiles) | whenLocal;
        main.create<i::Single>("login screen", [this]
            {
                state = s::MWlcm;
            });
        main.create<i::Toggle>(
            "online", &Config::getOnline, &Config::setOnline);
        main.create<i::Toggle>(
            "official mode", &Config::getOfficial, &Config::setOfficial);
        main.create<i::Single>("exit game", [this]
            {
                window.stop();
            });
        main.create<i::Single>("back", [this]
            {
                state = s::SMain;
            });

        resolution.create<i::Single>("auto", [this]
            {
                Config::setCurrentResolutionAuto(window);
            });

        for(const auto& vm : VideoMode::getFullscreenModes())
            if(vm.bitsPerPixel == 32)
                resolution.create<i::Single>(
                    toStr(vm.width) + "x" + toStr(vm.height), [this, &vm]
                    {
                        Config::setCurrentResolution(
                            window, vm.width, vm.height);
                    });

        resolution.create<i::Single>("go windowed", [this]
            {
                Config::setFullscreen(window, false);
            });
        resolution.create<i::Single>("go fullscreen", [this]
            {
                Config::setFullscreen(window, true);
            });
        resolution.create<i::GoBack>("back");

        gfx.create<i::Toggle>("3D effects", &Config::get3D, &Config::set3D);
        gfx.create<i::Toggle>(
            "no rotation", &Config::getNoRotation, &Config::setNoRotation) |
            whenNotOfficial;
        gfx.create<i::Toggle>("no background", &Config::getNoBackground,
            &Config::setNoBackground) |
            whenNotOfficial;
        gfx.create<i::Toggle>("b&w colors", &Config::getBlackAndWhite,
            &Config::setBlackAndWhite) |
            whenNotOfficial;
        gfx.create<i::Toggle>("pulse", &Config::getPulse, &Config::setPulse) |
            whenNotOfficial;

        gfx.create<i::Toggle>("flash", &Config::getFlash, &Config::setFlash);
        gfx.create<i::Toggle>("vsync", &Config::getVsync, [this](bool mValue)
            {
                Config::setVsync(window, mValue);
            });
        gfx.create<i::Single>("go windowed", [this]
            {
                Config::setFullscreen(window, false);
            });
        gfx.create<i::Single>("go fullscreen", [this]
            {
                Config::setFullscreen(window, true);
            });

        gfx.create<i::Single>("use static fps",
            [this]
            {
                Config::setTimerStatic(window, true);
            }) |
            whenTimerIsDynamic;
        gfx.create<i::Single>("use dynamic fps",
            [this]
            {
                Config::setTimerStatic(window, false);
            }) |
            whenTimerIsStatic;

        gfx.create<i::Toggle>("limit fps", &Config::getLimitFPS,
            [this](bool mValue)
            {
                Config::setLimitFPS(window, mValue);
            }) |
            whenTimerIsStatic;
        gfx.create<i::Slider>("max fps", &Config::getMaxFPS,
            [this](unsigned int mValue)
            {
                Config::setMaxFPS(window, mValue);
            },
            30u, 200u, 5u) |
            whenTimerIsStatic;
        gfx.create<i::Slider>("antialiasing", &Config::getAntialiasingLevel,
            [this](unsigned int mValue)
            {
                Config::setAntialiasingLevel(window, mValue);
            },
            0u, 3u, 1u);
        gfx.create<i::Toggle>(
            "show fps", &Config::getShowFPS, &Config::setShowFPS);
        gfx.create<i::Toggle>("text outlines", &Config::getDrawTextOutlines,
            &Config::setDrawTextOutlines);
        gfx.create<i::GoBack>("back");

        sfx.create<i::Toggle>(
            "no sound", &Config::getNoSound, &Config::setNoSound);
        sfx.create<i::Toggle>(
            "no music", &Config::getNoMusic, &Config::setNoMusic);
        sfx.create<i::Slider>("sound volume", &Config::getSoundVolume,
            [this](unsigned int mValue)
            {
                Config::setSoundVolume(mValue);
                assets.refreshVolumes();
            },
            0u, 100u, 5u) |
            whenSoundEnabled;
        sfx.create<i::Slider>("music volume", &Config::getMusicVolume,
            [this](unsigned int mValue)
            {
                Config::setMusicVolume(mValue);
                assets.refreshVolumes();
            },
            0u, 100u, 5u) |
            whenMusicEnabled;
        sfx.create<i::Toggle>("sync music with difficulty",
            &Config::getMusicSpeedDMSync, &Config::setMusicSpeedDMSync) |
            whenMusicEnabled;
        sfx.create<i::Slider>("music speed multipler",
            &Config::getMusicSpeedMult,
            [this](float mValue)
            {
                Config::setMusicSpeedMult(mValue);
            },
            0.7f, 1.3f, 0.05f) |
            whenMusicEnabled;
        sfx.create<i::GoBack>("back");

        play.create<i::Toggle>(
            "autorestart", &Config::getAutoRestart, &Config::setAutoRestart);
        play.create<i::Toggle>("rotate to start", &Config::getRotateToStart,
            &Config::setRotateToStart);
        play.create<i::GoBack>("back");

        localProfiles.create<i::Single>("change local profile", [this]
            {
                enteredStr = "";
                state = s::SLPSelect;
            });
        localProfiles.create<i::Single>("new local profile", [this]
            {
                enteredStr = "";
                state = s::SLPSelect;
            });
        localProfiles.create<i::GoBack>("back");

        debug.create<i::Toggle>(
            "invincible", &Config::getInvincible, &Config::setInvincible);
        debug.create<i::GoBack>("back");

        friends.create<i::Single>("add friend", [this]
            {
                enteredStr = "";
                state = s::ETFriend;
            });
        friends.create<i::Single>("clear friends", [this]
            {
                assets.pClearTrackedNames();
            });
        friends.create<i::GoBack>("back");
    }

    void MenuGame::leftAction()
    {
        assets.playSound("beep.ogg");
        touchDelay = 50.f;

        if(state == s::SLPSelect)
        {
            --profileIdx;
        }
        else if(state == s::SMain)
        {
            setIndex(currentIndex - 1);
        }
        else if(isInMenu())
        {
            getCurrentMenu()->decrease();
        }
    }

    void MenuGame::rightAction()
    {
        assets.playSound("beep.ogg");
        touchDelay = 50.f;

        if(state == s::SLPSelect)
        {
            ++profileIdx;
        }
        else if(state == s::SMain)
        {
            setIndex(currentIndex + 1);
        }
        else if(isInMenu())
        {
            getCurrentMenu()->increase();
        }
    }
    void MenuGame::upAction()
    {
        assets.playSound("beep.ogg");
        touchDelay = 50.f;

        if(state == s::SMain)
        {
            ++diffMultIdx;
        }
        else if(isInMenu())
        {
            getCurrentMenu()->previous();
        }
    }
    void MenuGame::downAction()
    {
        assets.playSound("beep.ogg");
        touchDelay = 50.f;

        if(state == s::SMain)
        {
            --diffMultIdx;
        }
        else if(isInMenu())
        {
            getCurrentMenu()->next();
        }
    }
    void MenuGame::okAction()
    {
        assets.playSound("beep.ogg");
        touchDelay = 50.f;

        if(state == s::SLPSelect)
        {
            assets.pSetCurrent(enteredStr);
            state = s::SMain;
        }
        else if(state == s::SMain)
        {
            window.setGameState(hexagonGame.getGame());
            hexagonGame.newGame(levelDataIds[currentIndex], true,
                ssvu::getByModIdx(diffMults, diffMultIdx));
        }
        else if(isInMenu())
        {
            getCurrentMenu()->exec();
        }
        else if(state == s::ETLPNew)
        {
            if(!enteredStr.empty())
            {
                assets.pCreate(enteredStr);
                assets.pSetCurrent(enteredStr);
                state = s::SMain;
                enteredStr = "";
            }
        }
        else if(state == s::ETFriend)
        {
            if(!enteredStr.empty() &&
                !ssvu::contains(assets.pGetTrackedNames(), enteredStr))
            {
                assets.pAddTrackedName(enteredStr);
                state = s::SMain;
                enteredStr = "";
            }
        }
        else if(state == s::ETUser)
        {
            if(!enteredStr.empty())
            {
                lrUser = enteredStr;
                state = s::ETPass;
                enteredStr = "";
            }
        }
        else if(state == s::ETPass)
        {
            if(!enteredStr.empty())
            {
                lrPass = enteredStr;
                state = s::SLogging;
                enteredStr = "";
                Online::tryLogin(lrUser, lrPass);
            }
        }
        else if(state == s::ETEmail)
        {
            if(!enteredStr.empty() && ssvu::contains(enteredStr, '@'))
            {
                lrEmail = enteredStr;
                enteredStr = "";
                Online::trySendUserEmail(lrEmail);
            }
        }
    }

    void MenuGame::initInput()
    {
        using k = KKey;
        using t = Type;

        game.addInput(Config::getTriggerRotateCCW(),
            [this](FT)
            {
                leftAction();
            },
            t::Once);
        game.addInput(Config::getTriggerRotateCW(),
            [this](FT)
            {
                rightAction();
            },
            t::Once);
        game.addInput(Config::getTriggerUp(),
            [this](FT)
            {
                upAction();
            },
            t::Once);
        game.addInput(Config::getTriggerDown(),
            [this](FT)
            {
                downAction();
            },
            t::Once);
        game.addInput(Config::getTriggerRestart(),
            [this](FT)
            {
                okAction();
            },
            t::Once);
        game.addInput({{k::F1}},
            [this](FT)
            {
                assets.playSound("beep.ogg");
                if(!assets.pIsLocal())
                {
                    state = s::MWlcm;
                    return;
                }
                if(state == s::SLPSelect)
                {
                    enteredStr = "";
                    state = s::ETLPNew;
                }
            },
            t::Once);
        game.addInput({{k::F2}, {k::J}},
            [this](FT)
            {
                assets.playSound("beep.ogg");
                if(state != s::SMain) return;
                if(!assets.pIsLocal())
                {
                    state = s::MWlcm;
                    return;
                }
                enteredStr = "";
                state = s::SLPSelect;
            },
            t::Once);
        game.addInput({{k::F3}, {k::K}},
            [this](FT)
            {
                assets.playSound("beep.ogg");
                if(state != s::SMain) return;
                state = s::MOpts;
            },
            t::Once);
        game.addInput({{k::F4}, {k::L}},
            [this](FT)
            {
                assets.playSound("beep.ogg");
                if(state == s::SMain)
                {
                    auto p(assets.getPackPaths());
                    packIdx = ssvu::getMod(packIdx + 1, p.size());
                    levelDataIds = assets.getLevelIdsByPack(p[packIdx]);
                    setIndex(0);
                }
            },
            t::Once);
        game.addInput(Config::getTriggerExit(),
            [this](FT)
            {
                assets.playSound("beep.ogg");
                bool valid{
                    (assets.pIsLocal() && assets.pIsValidLocalProfile()) ||
                    !assets.pIsLocal()};
                if(isInMenu() && valid)
                {
                    if(getCurrentMenu()->canGoBack())
                        getCurrentMenu()->goBack();
                    else
                        state = s::SMain;
                }
                else if((state == s::ETFriend || state == s::SLPSelect) &&
                        valid)
                    state = s::SMain;
            },
            t::Once);

        game.addInput(Config::getTriggerExit(),
            [this](FT mFT)
            {
                if(state != s::MOpts) exitTimer += mFT;
            },
            [this](FT)
            {
                exitTimer = 0;
            });
        game.addInput(Config::getTriggerScreenshot(),
            [this](FT)
            {
                mustTakeScreenshot = true;
            },
            t::Once);
        game.addInput({{k::LAlt, k::Return}},
                [this](FT)
                {
                    Config::setFullscreen(window, !window.getFullscreen());
                    game.ignoreNextInputs();
                },
                t::Once)
            .setPriorityUser(-1000);
        game.addInput({{k::BackSpace}},
            [this](FT)
            {
                if(isEnteringText() && !enteredStr.empty())
                    enteredStr.erase(enteredStr.end() - 1);
            },
            t::Once);
    }

    void MenuGame::initLua(Lua::LuaContext& mLua)
    {
        mLua.writeVariable("u_log", [this](string mLog)
            {
                lo("lua-menu") << mLog << "\n";
            });
        mLua.writeVariable("u_execScript", [this, &mLua](string mName)
            {
                Utils::runLuaFile(
                    mLua, levelData->packPath + "Scripts/" + mName);
            });
        mLua.writeVariable("u_getDifficultyMult", [this]
            {
                return 1;
            });
        mLua.writeVariable("u_getSpeedMultDM", [this]
            {
                return 1;
            });
        mLua.writeVariable("u_getDelayMultDM", [this]
            {
                return 1;
            });
        mLua.writeVariable("l_setRotationSpeed", [this](float mValue)
            {
                levelStatus.rotationSpeed = mValue;
            });
        mLua.writeVariable("l_setSides", [this](unsigned int mValue)
            {
                levelStatus.sides = mValue;
            });
        mLua.writeVariable("l_getRotationSpeed", [this]
            {
                return levelStatus.rotationSpeed;
            });
        mLua.writeVariable("l_getSides", [this]
            {
                return levelStatus.sides;
            });
        mLua.writeVariable("s_setPulseInc", [this](float mValue)
            {
                styleData.pulseIncrement = mValue;
            });
        mLua.writeVariable("s_setHueInc", [this](float mValue)
            {
                styleData.hueIncrement = mValue;
            });
        mLua.writeVariable("s_getHueInc", [this]
            {
                return styleData.hueIncrement;
            });

        // Unused functions
        for(const auto& un :
            {"l_setSpeedMult", "l_setSpeedInc", "l_setRotationSpeedMax",
                "l_setRotationSpeedInc", "l_setDelayInc", "l_setFastSpin",
                "l_setSidesMin", "l_setSidesMax", "l_setIncTime",
                "l_setPulseMin", "l_setPulseMax", "l_setPulseSpeed",
                "l_setPulseSpeedR", "l_setPulseDelayMax", "l_setBeatPulseMax",
                "l_setBeatPulseDelayMax", "l_setWallSkewLeft",
                "l_setWallSkewRight", "l_setWallAngleLeft",
                "l_setWallAngleRight", "l_setRadiusMin", "l_setSwapEnabled",
                "l_setTutorialMode", "l_setIncEnabled",
                "l_enableRndSideChanges", "l_getSpeedMult", "l_getDelayMult",
                "l_addTracked", "u_playSound", "u_isKeyPressed",
                "u_isFastSpinning", "u_forceIncrement", "u_kill", "u_eventKill",
                "m_messageAdd", "m_messageAddImportant", "t_wait", "t_waitS",
                "t_waitUntilS", "e_eventStopTime", "e_eventStopTimeS",
                "e_eventWait", "e_eventWaitS", "e_eventWaitUntilS", "w_wall",
                "w_wallAdj", "w_wallAcc", "w_wallHModSpeedData",
                "w_wallHModCurveData", "l_setDelayMult", "l_setMaxInc",
                "s_setStyle", "u_setMusic", "l_getRotation", "l_setRotation",
                "s_getCameraShake", "s_setCameraShake", "l_getOfficial"})
            mLua.writeVariable(un, []
                {
                });
    }

    void MenuGame::setIndex(int mIdx)
    {
        currentIndex = mIdx;

        if(currentIndex > ssvu::toInt(levelDataIds.size() - 1))
            currentIndex = 0;
        else if(currentIndex < 0)
            currentIndex = ssvu::toInt(levelDataIds.size()) - 1;

        levelData = &assets.getLevelData(levelDataIds[currentIndex]);

        styleData = assets.getStyleData(levelData->styleId);
        diffMults = levelData->difficultyMults;
        diffMultIdx = idxOf(diffMults, 1);

        Lua::LuaContext lua;
        initLua(lua);
        Utils::runLuaFile(lua, levelData->luaScriptPath);
        Utils::runLuaFunction<void>(lua, "onInit");
        Utils::runLuaFunction<void>(lua, "onLoad");
    }

    void MenuGame::updateLeaderboard()
    {
        if(assets.pIsLocal())
        {
            leaderboardString = "playing locally";
            return;
        }

        currentLeaderboard = Online::getCurrentLeaderboard();
        if(currentLeaderboard == "NULL")
        {
            leaderboardString = "...";
            return;
        }

        constexpr unsigned int leaderboardRecordCount{8};
        ssvuj::Obj root{getFromStr(currentLeaderboard)};
        if(getExtr<string>(root, "id") != levelData->id)
        {
            leaderboardString = "...";
            return;
        }

        auto currentPlayerScore = getExtr<string>(root, "ps");
        string currentPlayerPosition = getExtr<string>(root, "pp");

        using RecordPair = pair<string, float>;
        vector<RecordPair> recordPairs;

        int playerPosition{-1};

        for(auto& record : ssvuj::getObj(root, "r"))
        {
            string name{toLower(getExtr<string>(record, 0))};
            float score{getExtr<float>(record, 1)};
            recordPairs.emplace_back(name, score);
        }

        bool foundPlayer{false};
        for(auto i(0u); i < recordPairs.size(); ++i)
        {
            if(recordPairs[i].first != assets.pGetName()) continue;
            playerPosition = ssvu::toInt(i) + 1;
            foundPlayer = true;
            break;
        }

        string result;
        for(auto i(0u); i < recordPairs.size(); ++i)
        {
            if(currentPlayerScore != "NULL" && currentPlayerScore != "" &&
                !foundPlayer && i == leaderboardRecordCount - 1)
            {
                result.append("...(" + currentPlayerPosition + ") " +
                              assets.pGetName() + ": " +
                              toStr(currentPlayerScore) + "\n");
                break;
            }

            if(i <= leaderboardRecordCount)
            {
                if(playerPosition == -1 || i < leaderboardRecordCount)
                {
                    auto& recordPair(recordPairs[i]);
                    if(recordPair.first == assets.pGetName())
                        result.append(" >> ");
                    result.append("(" + toStr(i + 1) + ") " + recordPair.first +
                                  ": " + toStr(recordPair.second) + "\n");
                }
            }
            else
                break;
        }

        leaderboardString = result;
    }

    void MenuGame::updateFriends()
    {
        if(state != s::SMain) return;

        if(assets.pIsLocal())
        {
            friendsString = "playing locally";
            return;
        }
        if(assets.pGetTrackedNames().empty())
        {
            friendsString =
                "you have no friends! :(\nadd them in the options menu";
            return;
        }

        const auto& fs(Online::getCurrentFriendScores());

        if(ssvuj::getObjSize(fs) == 0)
        {
            friendsString = "";
            for(const auto& n : assets.pGetTrackedNames())
                friendsString.append("(?)" + n + "\n");
            return;
        }

        using ScoreTuple = tuple<int, string, float>;
        vector<ScoreTuple> tuples;
        for(const auto& n : assets.pGetTrackedNames())
        {
            if(!ssvuj::hasObj(fs, n)) continue;

            const auto& score(ssvuj::getExtr<float>(fs[n], 0));
            const auto& pos(ssvuj::getExtr<unsigned int>(fs[n], 1));

            if(pos == 0) continue;
            tuples.emplace_back(pos, n, score);
        }

        sort(tuples, [](const auto& mA, const auto& mB)
            {
                return std::get<0>(mA) < std::get<0>(mB);
            });
        friendsString.clear();
        for(const auto& t : tuples)
            friendsString.append("(" + toStr(std::get<0>(t)) + ") " +
                                 std::get<1>(t) + ": " + toStr(std::get<2>(t)) +
                                 "\n");
    }

    void MenuGame::refreshCamera()
    {
        float fw{1024.f / Config::getWidth()}, fh{768.f / Config::getHeight()};
        float fmax{max(fw, fh)};
        w = Config::getWidth() * fmax;
        h = Config::getHeight() * fmax;
        overlayCamera.setView(View{FloatRect(0, 0, w, h)});
        titleBar.setOrigin(ssvs::zeroVec2f);
        titleBar.setScale({0.5f, 0.5f});
        titleBar.setPosition({20.f, 20.f});

        txtVersion.setString(toStr(Config::getVersion()));
        txtVersion.setFillColor(Color::White);
        txtVersion.setOrigin({getLocalRight(txtVersion), 0.f});
        txtVersion.setPosition(
            {getGlobalRight(titleBar) - 15.f, getGlobalTop(titleBar) + 15.f});

        creditsBar1.setOrigin({getLocalWidth(creditsBar1), 0.f});
        creditsBar1.setScale({0.373f, 0.373f});
        creditsBar1.setPosition({w - 20.f, 20.f});

        creditsBar2.setOrigin({getLocalWidth(creditsBar2), 0});
        creditsBar2.setScale({0.373f, 0.373f});
        creditsBar2.setPosition(
            {w - 20.f, 17.f + getGlobalBottom(creditsBar1)});

        float scaleFactor{w / 1024.f};
        bottomBar.setOrigin({0, 56.f});
        bottomBar.setScale({scaleFactor, scaleFactor});
        bottomBar.setPosition(Vec2f(0, h));
    }

    void MenuGame::update(FT mFT)
    {
        if(touchDelay > 0.f) touchDelay -= mFT;

        if(window.getFingerDownCount() == 1)
        {
            auto wThird = window.getWidth() / 3.f;
            auto wLT = window.getWidth() - wThird;
            auto hThird = window.getHeight() / 3.f;
            auto hLT = window.getHeight() - hThird;

            for(const auto& p : window.getFingerDownPositions())
            {
                if(p.y > hThird && p.y < hLT)
                {
                    if(p.x > 0.f && p.x < wThird)
                    {
                        leftAction();
                    }
                    else if(p.x < toNum<int>(window.getWidth()) && p.x > wLT)
                    {
                        rightAction();
                    }
                    else if(p.x > wThird && p.x < wLT)
                    {
                        okAction();
                    }
                }
                else
                {
                    if(p.y < hThird)
                    {
                        upAction();
                    }
                    else if(p.y > hLT)
                    {
                        downAction();
                    }
                }
            }
        }

        overlayCamera.update(mFT);
        backgroundCamera.update(mFT);

        if(getCurrentMenu() != nullptr) getCurrentMenu()->update();

        currentCreditsId += mFT;
        creditsBar2.setTexture(assets.get<Texture>(ssvu::getByModIdx(
            creditsIds, ssvu::toInt(currentCreditsId / 100))));

        // If connection is lost, kick the player back into welcome screen
        if(!assets.pIsLocal() &&
            Online::getConnectionStatus() != ocs::Connected)
            state = s::MWlcm;

        updateLeaderboard();
        updateFriends();

        if(exitTimer > 20) window.stop();

        if(isEnteringText())
        {
            unsigned int limit{state == s::ETEmail ? 40u : 18u};
            for(const auto& c : enteredChars)
                if(enteredStr.size() < limit &&
                    (ssvu::isAlphanumeric(c) || ssvu::isPunctuation(c)))
                {
                    assets.playSound("beep.ogg");
                    enteredStr.append(toStr(c));
                }
        }
        else if(state == s::SLPSelect)
        {
            enteredStr =
                ssvu::getByModIdx(assets.getLocalProfileNames(), profileIdx);
        }
        else if(state == s::SMain)
        {
            styleData.update(mFT);
            backgroundCamera.turn(levelStatus.rotationSpeed * 10.f);

            if(!assets.pIsLocal())
            {
                float diffMult{ssvu::getByModIdx(diffMults, diffMultIdx)};
                Online::requestLeaderboardIfNeeded(levelData->id, diffMult);
            }
        }
        else if(state == s::SLogging)
        {
            if(Online::getLoginStatus() == ols::Logged)
            {
                state = Online::getNewUserReg() ? s::ETEmail : s::SMain;
            }
            else if(Online::getLoginStatus() == ols::Unlogged)
            {
                state = s::MWlcm;
            }
        }

        if(state == s::ETEmail && !Online::getNewUserReg()) state = s::SMain;

        enteredChars.clear();
    }
    void MenuGame::draw()
    {
        styleData.computeColors();
        window.clear(
            state != s::SMain ? Color::Black : styleData.getColors()[0]);

        backgroundCamera.apply();
        if(state == s::SMain)
            styleData.drawBackground(
                window, ssvs::zeroVec2f, levelStatus.sides);

        overlayCamera.apply();
        if(state == s::SMain)
        {
            drawLevelSelection();
            render(bottomBar);
        }
        else if(isEnteringText())
        {
            drawEnteringText();
        }
        else if(state == s::SLPSelect)
        {
            drawProfileSelection();
        }
        else if(state == s::MOpts)
        {
            drawOptions();
        }
        else if(state == s::MWlcm)
        {
            drawWelcome();
        }

        render(titleBar);
        render(creditsBar1);
        render(creditsBar2);
        render(txtVersion);
        if(mustTakeScreenshot)
        {
            window.saveScreenshot("screenshot.png");
            mustTakeScreenshot = false;
        }
    }

    void MenuGame::drawLevelSelection()
    {
        MusicData musicData{assets.getMusicData(levelData->musicId)};
        const auto& packPathStr(levelData->packPath.getStr());
        PackData packData{
            assets.getPackData(packPathStr.substr(6, packPathStr.size() - 7))};
        const string& packName{packData.name};

        if(Config::getOnline())
        {
            string versionMessage{"connecting to server..."};
            float serverVersion{Online::getServerVersion()};

            if(serverVersion == -1)
                versionMessage = "error connecting to server";
            else if(serverVersion == Config::getVersion())
                versionMessage = "you have the latest version";
            else if(serverVersion < Config::getVersion())
                versionMessage = "your version is newer (beta)";
            else if(serverVersion > Config::getVersion())
                versionMessage =
                    "update available (" + toStr(serverVersion) + ")";
            renderText(versionMessage, txtProf, {20, 4}, 13);

            Text& profile = renderText("profile: " + assets.pGetName(), txtProf,
                Vec2f{20.f, getGlobalBottom(titleBar) + 8}, 18);
            Text& pack =
                renderText("pack: " + packName + " (" + toStr(packIdx + 1) +
                               "/" + toStr(assets.getPackPaths().size()) + ")",
                    txtProf, {20.f, getGlobalBottom(profile) - 7.f}, 18);

            string lbestStr;
            if(assets.pIsLocal())
            {
                SSVU_ASSERT(diffMults.size() != 0);
                lbestStr =
                    "local best: " +
                    toStr(assets.getLocalScore(getLocalValidator(levelData->id,
                        ssvu::getByModIdx(diffMults, diffMultIdx))));
            }
            else
            {
                lbestStr = Online::getLoginStatus() == ols::Logged
                               ? "logged in as: " + Online::getCurrentUsername()
                               : "logging in...";
            }

            Text& lbest = renderText(
                lbestStr, txtProf, {20.f, getGlobalBottom(pack) - 7.f}, 18);
            if(diffMults.size() > 1)
                renderText("difficulty: " +
                               toStr(ssvu::getByModIdx(diffMults, diffMultIdx)),
                    txtProf, {20.f, getGlobalBottom(lbest) - 7.f}, 18);

            renderText(
                leaderboardString, txtProf, {20.f, getGlobalBottom(lbest)}, 15);
            Text& smsg =
                renderText("server message: " + Online::getServerMessage(),
                    txtLAuth, {20.f, getGlobalTop(bottomBar) - 20.f}, 14);
            txtFriends.setOrigin({getLocalWidth(txtFriends), 0.f});
            renderText("friends:\n" + friendsString, txtFriends,
                {w - 20.f, getGlobalBottom(titleBar) + 8}, 18);

            if(!Config::isEligibleForScore())
                renderText("not eligible for scoring: " +
                               Config::getUneligibilityReason(),
                    txtProf, {20.f, getGlobalTop(smsg) - 20.f}, 11);

            if(!assets.pIsLocal() && Online::getLoginStatus() == ols::Logged)
            {
                const auto& us(Online::getUserStats());
                string userStats;
                userStats += "deaths: " + toStr(us.deaths) + "\n";
                userStats += "restarts: " + toStr(us.restarts) + "\n";
                userStats +=
                    "played: " + toStr(us.minutesSpentPlaying) + " min";
                renderText(userStats, txtLMus,
                    {getGlobalRight(titleBar) + 10.f, getGlobalTop(titleBar)},
                    13);
            }
        }
        else
            renderText("online disabled", txtProf, {20, 0}, 13);

        Text& lname = renderText(levelData->name, txtLName, {20.f, h / 2.f});
        Text& ldesc = renderText(levelData->description, txtLDesc,
            {20.f, getGlobalBottom(lname) + 2.f});
        Text& lauth = renderText("author: " + levelData->author, txtLAuth,
            {20.f, getGlobalBottom(ldesc) + 25.f});
        renderText("music: " + musicData.name + " by " + musicData.author +
                       " (" + musicData.album + ")",
            txtLMus, {20.f, getGlobalBottom(lauth) - 5.f});
        renderText("(" + toStr(currentIndex + 1) + "/" +
                       toStr(levelDataIds.size()) + ")",
            txtLMus, {20.f, getGlobalTop(lname) - 25.f});

        string packNames{"Installed packs:\n"};
        for(const auto& n : assets.getPackIds())
        {
            if(packData.id == n) packNames += ">>> ";
            packNames.append(n + "\n");
        }
        txtPacks.setString(packNames);
        txtPacks.setOrigin(getGlobalWidth(txtPacks), getGlobalHeight(txtPacks));
        txtPacks.setPosition({w - 20.f, getGlobalTop(bottomBar) - 15.f});
        txtPacks.setFillColor(styleData.getMainColor());
        render(txtPacks);
    }
    void MenuGame::drawEnteringText()
    {
        string title;
        switch(state)
        {
            case s::ETUser: title = "insert username"; break;
            case s::ETPass: title = "insert password"; break;
            case s::ETEmail: title = "insert email"; break;
            case s::ETFriend: title = "add friend"; break;
            case s::ETLPNew: title = "create local profile"; break;
            default: throw;
        }

        renderText(title, txtProf, {20, 768 - 395});
        renderText("insert text", txtProf, {20, 768 - 375});
        renderText("press enter when done", txtProf, {20, 768 - 335});
        renderText("keep esc pressed to exit", txtProf, {20, 768 - 315});
        renderText(
            state == s::ETPass ? string(enteredStr.size(), '*') : enteredStr,
            txtLName, {20, 768 - 245 - 40}, (state == s::ETEmail) ? 32 : 65);
    }
    void MenuGame::drawProfileSelection()
    {
        if(!assets.pIsLocal()) throw;
        renderText("local profile selection", txtProf, {20, 768 - 395});
        renderText(
            "press left/right to browse profiles", txtProf, {20, 768 - 375});
        renderText("press enter to select profile", txtProf, {20, 768 - 355});
        renderText(
            "press f1 to create a new profile", txtProf, {20, 768 - 335});
        renderText(enteredStr, txtLName, {20, 768 - 245 - 40});
    }

    void MenuGame::drawMenu(const Menu& mMenu)
    {
        renderText(mMenu.getCategory().getName(), txtLDesc,
            {20.f, getGlobalBottom(titleBar)});

        float currentX{0.f}, currentY{0.f};
        auto& currentItems(mMenu.getItems());
        for(int i{0}; i < ssvu::toInt(currentItems.size()); ++i)
        {
            currentY += 19;
            if(i != 0 && i % 21 == 0)
            {
                currentY = 0;
                currentX += 180;
            }
            string name, itemName{currentItems[i]->getName()};
            if(i == mMenu.getIdx()) name.append(">> ");
            name.append(itemName);

            int extraSpacing{0};
            if(itemName == "back") extraSpacing = 20;
            renderText(name, txtProf,
                {20.f + currentX,
                    getGlobalBottom(titleBar) + 20.f + currentY + extraSpacing},
                currentItems[i]->isEnabled() ? Color::White
                                             : Color{155, 155, 155, 255});
        }
    }

    void MenuGame::drawOptions()
    {
        drawMenu(optionsMenu);

        if(Config::getOfficial())
            renderText("official mode on - some options cannot be changed",
                txtProf, {20, h - 30.f});
        else
            renderText("official mode off - not eligible for scoring", txtProf,
                {20, h - 30.f});

        if(assets.pIsLocal())
            renderText("local mode on - some options cannot be changed",
                txtProf, {20, h - 60.f});
    }

    void MenuGame::drawWelcome()
    {
        drawMenu(welcomeMenu);

        renderText(Online::getLoginStatus() == ols::Logged
                       ? "logged in as: " + Online::getCurrentUsername()
                       : "not logged in",
            txtProf, {20, h - 50.f});

        string connStatus;
        switch(Online::getConnectionStatus())
        {
            case ocs::Disconnected:
                connStatus = "not connected to server";
                break;
            case ocs::Connecting: connStatus = "connecting to server..."; break;
            case ocs::Connected: connStatus = "connected to server"; break;
        }
        renderText(connStatus, txtProf, {20, h - 30.f});
    }
}
