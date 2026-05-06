// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Core/HexagonClient.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/HexagonServer.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/GameSystem/GameWindow.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Imgui.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/VectorToSet.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>
#include <filesystem>
#include <iostream>
#include <sodium.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>

//
//
// ----------------------------------------------------------------------------
// Floating-point sanity checks
// ----------------------------------------------------------------------------

static_assert(std::numeric_limits<float>::is_iec559);
static_assert(std::numeric_limits<float>::digits == 24);

static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<double>::digits == 53);

//
//
// ----------------------------------------------------------------------------
// Utilities
// ----------------------------------------------------------------------------

namespace
{

void createFolderIfNonExistant(const sf::base::String& folderName)
{
    const ssvu::FileSystem::Path path{folderName.cStr()};

    if (path.isFolder())
    {
        return;
    }

    hg::lo("::createFolderIfNonExistant") << "'" << folderName << "' folder does not exist, creating\n";

    createFolder(path);
}

struct ParsedArgs
{
    sf::base::Vector<sf::base::String>   args;
    sf::base::Optional<sf::base::String> cliLevelName;
    sf::base::Optional<sf::base::String> cliLevelPack;
    bool                                 printLuaDocs{false};
    bool                                 headless{false};
    bool                                 server{false};
};

[[nodiscard]] ParsedArgs parseArgs(const int argc, char* argv[])
{
    ParsedArgs result;

    for (int i = 0; i < argc; ++i)
    {
        // Find command-line pack name (to immediately run level)
        if (!std::strcmp(argv[i], "-p") && i + 1 < argc)
        {
            ++i;
            result.cliLevelPack.emplace(argv[i]);
            continue;
        }

        // Find command-line level name (to immediately run level)
        if (!std::strcmp(argv[i], "-l") && i + 1 < argc)
        {
            ++i;
            result.cliLevelName.emplace(argv[i]);
            continue;
        }

        // Find command-line argument to print Lua docs
        if (!std::strcmp(argv[i], "-printLuaDocs"))
        {
            result.printLuaDocs = true;
            continue;
        }

        // Find command-line argument to run in headless mode
        if (!std::strcmp(argv[i], "-headless"))
        {
            result.headless = true;
            continue;
        }

        // Find command-line argument to run in server mode
        if (!std::strcmp(argv[i], "-server"))
        {
            result.server = true;
            continue;
        }

        result.args.emplaceBack(argv[i]);
    }

    return result;
}

[[nodiscard]] sf::base::String makeWindowTitle()
{
    return hg::Utils::concat("Open Hexagon ", hg::GAME_VERSION_STR, " - by Vittorio Romeo - https://vittorioromeo.com");
}

[[nodiscard]] sf::base::Optional<sf::base::String> getFirstCompressedReplayFilenameFromArgs(
    const sf::base::Vector<sf::base::String>& args)
{
    for (const sf::base::String& arg : args)
    {
        if (arg.toStringView().find(".ohr.z") != sf::base::String::nPos)
        {
            return sf::base::makeOptional(arg);
        }
    }

    return sf::base::nullOpt;
}

} // namespace

//
//
// ----------------------------------------------------------------------------
// Print lua docs entrypoint
// ----------------------------------------------------------------------------

[[nodiscard]] int mainPrintLuaDocs()
{
    hg::HGAssets assets{
        nullptr,
        /* steamManager */  //
        true /* headless */ //
    };

    hg::HexagonGame hg{
        nullptr,
        /* steamManager */ //
        nullptr,
        /* discordManager */ //
        assets,              //
        nullptr,
        /* audio */ //
        nullptr,
        /* window */         //
        nullptr /* client */ //
    };

    std::cout << "\n\n\n\n\n";
    hg.initLuaAndPrintDocs();
    std::cout << "\n\n\n\n\n";

    hg::lo("::mainPrintLuaDocs") << "Finished\n";
    return 0;
}

//
//
// ----------------------------------------------------------------------------
// Server main entrypoint
// ----------------------------------------------------------------------------

[[nodiscard]] int mainServer()
{
    hg::Steam::steam_manager steamManager;

    hg::Config::loadConfig({} /* overrideIds */);
    hg::Config::setUseLuaFileCache(true);

    SFML_BASE_SCOPE_GUARD({
        hg::lo("::main") << "Saving config...\n";
        hg::Config::saveConfig();
        hg::lo("::main") << "Done saving config\n";
    });

    hg::HGAssets assets{
        &steamManager,      //
        true /* headless */ //
    };

    hg::HexagonGame hg{
        nullptr /* steamManager */,   //
        nullptr /* discordManager */, //
        assets,                       //
        nullptr /* audio */,          //
        nullptr /* window */,         //
        nullptr /* client */          //
    };

    // TODO (P0): handle `resolve` errors
    hg::HexagonServer hs{
        &assets,                                                         //
        &hg,                                                             //
        sf::IpAddressUtils::resolve(hg::Config::getServerIp()).value(),  //
        hg::Config::getServerPort(),                                     //
        hg::Config::getServerControlPort(),                              //
        hg::Utils::toUnorderedSet(hg::Config::getServerLevelWhitelist()) //
    };

    // Graceful CTRL-C: close the listener so the selector-wait unblocks and
    // `run()` notices the stop request.
    static hg::HexagonServer& globalServer = hs;
    std::signal(SIGINT,
                [](int s)
    {
        std::printf("Caught signal %d\n", s);
        globalServer.stop();
    });

    hs.run();

    hg::lo("::mainServer") << "Finished\n";
    return 0;
}

//
//
// ----------------------------------------------------------------------------
// Client main entrypoint
// ----------------------------------------------------------------------------

[[nodiscard]] int mainClient(const bool                                  headless,
                             const sf::base::Vector<sf::base::String>&   args,
                             const sf::base::Optional<sf::base::String>& cliLevelName,
                             const sf::base::Optional<sf::base::String>& cliLevelPack)
{
    // ------------------------------------------------------------------------
    // Steam integration
    hg::Steam::steam_manager steamManager;

    if (steamManager.is_initialized())
    {
        steamManager.request_encrypted_app_ticket();
        steamManager.request_stats_and_achievements();
        steamManager.run_callbacks();
    }

    //
    //
    // ------------------------------------------------------------------------
    // Discord integration
    sf::base::Optional<hg::Discord::discord_manager> discordManager;

    if (!headless)
    {
        discordManager.emplace();
    }

    //
    //
    // ------------------------------------------------------------------------
    // Create game folders if needed
    createFolderIfNonExistant("Profiles/");
    createFolderIfNonExistant("Replays/");

    //
    //
    // ------------------------------------------------------------------------
    // Load configuration (and overrides)
    hg::Config::loadConfig(args);
    hg::Config::reapplyResolution();

    // TODO (P0): server gets ALSA errors during asset load, is it loading
    // musics/sounds?
    SFML_BASE_SCOPE_GUARD({
        hg::lo("::main") << "Saving config...\n";
        hg::Config::saveConfig();
        hg::lo("::main") << "Done saving config\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Create the game window
    auto                                 graphicsContext = sf::GraphicsContext::create().value();
    sf::base::Optional<ssvs::GameWindow> window;

    if (!headless)
    {
        window.emplace(hg::Config::TIME_STEP, hg::Config::TIME_SLICE);

        window->setTitle(makeWindowTitle());
        window->setSize(hg::Config::getWidth(), hg::Config::getHeight());
        window->setPixelMult(hg::Config::getPixelMultiplier());
        window->setFullscreen(hg::Config::getFullscreen());
        window->setAntialiasingLevel(hg::Config::getAntialiasingLevel());
        window->setVsync(hg::Config::getVsync());
        window->setFPSLimited(hg::Config::getLimitFPS());
        window->setMaxFPS(hg::Config::getMaxFPS());

        {
            const auto resetIcon = [&window]
            {
                const sf::base::Optional icon = sf::Image::loadFromFile("Assets/icon.png");

                if (!icon.hasValue())
                {
                    hg::lo("::main") << "Failed to load icon image\n";
                    return;
                }

                window->getRenderWindow().setIcon(icon->getPixelsPtr(), icon->getSize());
            };

            window->onRecreation += resetIcon;
            resetIcon();
        }

        // Signal handling: exit gracefully on CTRL-C
        {
            SSVOH_ASSERT(window.hasValue());
            static ssvs::GameWindow& globalWindow = *window;

            // TODO (P2): UB
            std::signal(SIGINT,
                        [](int s)
            {
                hg::lo("::main") << "Caught signal '" << s << "' with game window open\n";

                hg::lo("::main") << "Stopping game window...\n";
                globalWindow.stop();
                hg::lo("::main") << "Done stopping game window\n";
            });
        }
    }

    //
    //
    // ------------------------------------------------------------------------
    // Initialize assets
    hg::HGAssets assets{&steamManager, headless};
    SFML_BASE_SCOPE_GUARD({
        hg::lo("::main") << "Saving all local profiles...\n";
        assets.pSaveAll();
        hg::lo("::main") << "Done saving all local profiles\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Initialize audio
    auto               audioContext = sf::AudioContext::create().value();
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    hg::Audio audio{
        //
        playbackDevice,
        [&assets](const sf::base::String& assetId) -> sf::SoundBuffer* { return assets.getSoundBuffer(assetId); }, //
        [&assets](const sf::base::String& assetId) -> const sf::base::String* { return assets.getMusicPath(assetId); } //
    };

    audio.setSoundVolume(hg::Config::getSoundVolume());
    audio.setMusicVolume(hg::Config::getMusicVolume());

    // ------------------------------------------------------------------------
    // Initialize hexagon client
    // TODO (P0): handle `resolve` errors
    hg::HexagonClient hc{steamManager,
                         sf::IpAddressUtils::resolve(hg::Config::getServerIp()).value(),
                         hg::Config::getServerPort()};

    //
    //
    // ------------------------------------------------------------------------
    // Initialize hexagon game
    hg::HexagonGame hg{
        &steamManager,                                            //
        (discordManager.hasValue() ? &*discordManager : nullptr), //
        assets,                                                   //
        &audio,                                                   //
        (window.hasValue() ? &*window : nullptr),                 //
        &hc                                                       //
    };

    //
    //
    // ------------------------------------------------------------------------
    // Two extra HexagonGame instances dedicated to menu visuals:
    //   - `hgMenuBg` permanently runs a hand-crafted backdrop level behind
    //     every menu screen, in `previewMode` so it never reacts to input
    //     and never advances the user's score / state.
    //   - `hgPreview` renders the currently-selected level into an
    //     off-screen `sf::RenderTexture`, also in preview mode. The menu
    //     draws that texture as a small "screen" inside LevelSelect.
    sf::base::Optional<hg::HexagonGame> hgMenuBg;
    sf::base::Optional<hg::HexagonGame> hgPreview;
    if (!headless && window.hasValue())
    {
        hgMenuBg.emplace(
            &steamManager,
            (discordManager.hasValue() ? &*discordManager : nullptr),
            assets, &audio, &*window, /*hexagonClient=*/nullptr);
        hgMenuBg->previewMode = true;

        hgPreview.emplace(
            &steamManager,
            (discordManager.hasValue() ? &*discordManager : nullptr),
            assets, &audio, &*window, /*hexagonClient=*/nullptr);
        hgPreview->previewMode = true;
    }

    //
    //
    // ------------------------------------------------------------------------
    // Initialize menu game and link to hexagon game
    sf::base::Optional<hg::MenuGame> mg;

    if (!headless)
    {
        SSVOH_ASSERT(window.hasValue());
        SSVOH_ASSERT(discordManager.hasValue());

        mg.emplace(steamManager, *discordManager, assets, audio, *window, hc);

        mg->fnHGTriggerRefresh = [&](const ssvs::Input::Trigger& trigger,
                                     int                         bindId) //
        {
            hg.refreshTrigger(trigger, bindId); //
        };

        mg->fnHGNewGame =
            [&](const sf::base::String& packId, const sf::base::String& levelId, bool firstPlay, float diffMult, bool executeLastReplay)
        {
            hg.newGame(packId, levelId, firstPlay, diffMult, executeLastReplay);

            window->setGameState(hg.getGame());
        };

        mg->fnHGUpdateRichPresenceCallbacks = [&] //
        {                                         //
            hg.updateRichPresenceCallbacks();
        };

        hg.fnGoToMenu = [&](const bool error)
        {
            mg->returnToLevelSelection();
            mg->init(error);

            window->setGameState(mg->getGame());
        };

        // Hand the menu the auxiliary HG instances so it can run a
        // background level under all menus and a separate per-selection
        // preview inside LevelSelect. The pack/level here is the
        // "menu-background" level the user crafts; an empty/invalid id
        // simply leaves the menu without an animated backdrop.
        if (hgMenuBg.hasValue() && hgPreview.hasValue())
        {
            // "Shader Test" from the bundled `Artwork` pack -- the only
            // pack/level the menu backdrop ever runs. Pack id format is
            // `<disambiguator>_<author>_<name>_<version>`; level id is
            // `<packId>_<levelJsonId>`.
            mg->setMenuPreviewGames(
                &*hgMenuBg, &*hgPreview,
                /*menuBgPackId=*/"thing_Synth_Morxemplum_Artwork_1",
                /*menuBgLevelId=*/"thing_Synth_Morxemplum_Artwork_1_shadertest");
        }
    }

    //
    //
    // ------------------------------------------------------------------------
    // Load drag & drop replay, if any -- otherwise run game as normal
    const sf::base::Optional<sf::base::String> compressedReplayFilename = getFirstCompressedReplayFilenameFromArgs(args);

    if (!headless)
    {
        SSVOH_ASSERT(window.hasValue());

        const auto gotoMenu = [&]
        {
            mg->init(false /* mError */);
            window->setGameState(mg->getGame());
        };

        const auto gotoGameCompressedReplay = [&](const hg::compressed_replay_file& compressedReplayFile)
        {
            sf::base::Optional<hg::replay_file> replayFileOpt = hg::decompress_replay_file(compressedReplayFile);

            if (!replayFileOpt.hasValue())
            {
                std::cerr << "Could not decompress replay file\n";
                return;
            }

            hg::replay_file& replayFile = replayFileOpt.value();

            hg.setLastReplay(replayFile);

            hg.newGame(replayFile._pack_id,
                       replayFile._level_id,
                       replayFile._first_play,
                       replayFile._difficulty_mult,
                       /* mExecuteLastReplay */ true);

            window->setGameState(hg.getGame());
        };

        if (!compressedReplayFilename.hasValue())
        {
            if (cliLevelPack.hasValue() && cliLevelName.hasValue())
            {
                // Load pack and levels specified via command line args.
                mg->init(false /* mError */, *cliLevelPack, *cliLevelName);
            }
            else
            {
                // Start game as normal.
                gotoMenu();
            }
        }
        else
        {
            if (hg::compressed_replay_file crf; crf.deserialize_from_file(compressedReplayFilename->cStr()))
            {
                hg::lo("Replay") << "Playing compressed replay file '" << *compressedReplayFilename << "'\n";

                gotoGameCompressedReplay(crf);
            }
            else
            {
                hg::lo("Replay") << "Failed to read compressed replay file '" << compressedReplayFilename.value() << "'\n";

                gotoMenu();
            }
        }
    }
    else
    {
        SSVOH_ASSERT(headless);

        // TODO (P2): code repetition, cleanup
        if (!compressedReplayFilename.hasValue())
        {
            std::cout << "Running in headless mode without replay...?\n";
            return 1;
        }

        if (hg::compressed_replay_file crf; crf.deserialize_from_file(compressedReplayFilename->cStr()))
        {
            sf::base::Optional<hg::replay_file> replayFileOpt = hg::decompress_replay_file(crf);

            if (!replayFileOpt.hasValue())
            {
                std::cerr << "Could not decompress replay file\n";
                return 1;
            }

            hg::replay_file& replayFile = replayFileOpt.value();

            hg::lo("Replay") << "Playing compressed replay file in headless mode '" << *compressedReplayFilename << "'\n";

            // TODO (P2): check level validity

            std::cout << "Player died.\nFinal time: "
                      << hg.runReplayUntilDeathAndGetScore(replayFile, 1 /* maxProcessingSeconds */, 1.f /* timescale */)
                             .value()
                             .playedTimeSeconds
                      << '\n';
        }
        else
        {
            hg::lo("Replay") << "Failed to read compressed replay file in headless mode '"
                             << compressedReplayFilename.value() << "'\n";
        }
    }

    //
    //
    // ------------------------------------------------------------------------
    // Run the game!
    if (!headless)
    {
        SSVOH_ASSERT(window.hasValue());
        window->run();
    }

    hg::lo("::mainClient") << "Finished\n";
    return 0;
}

//
//
// ----------------------------------------------------------------------------
// Program main entrypoint
// ----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc < 1)
    {
        std::cerr << "Fatal error: no executable specified" << std::endl;
        return -1;
    }

    //
    //
    // ------------------------------------------------------------------------
    // libsodium initialization
    if (sodium_init() < 0)
    {
        hg::lo("::main") << "Failed initializing libsodium\n";
        return 1;
    }

    //
    //
    // ------------------------------------------------------------------------
    // Basic signal handling
    // TODO (P2): UB
    std::signal(SIGINT,
                [](int s)
    {
        hg::lo("::main") << "Caught signal '" << s << "' without game window open, exiting...\n";

        std::exit(1);
    });

    //
    //
    // ------------------------------------------------------------------------
    // Flush and save log (at the end of the scope)
    SFML_BASE_SCOPE_GUARD({
        hg::lo("::main") << "Flushing log to 'log.txt'...\n";
        hg::lo().flush();
        hg::lo("::main") << "Done flushing log to 'log.txt'\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Set working directory to current executable location
    std::filesystem::current_path(std::filesystem::path{argv[0]}.parent_path());

    //
    //
    // ------------------------------------------------------------------------
    // Parse command line arguments
    const auto [args, cliLevelName, cliLevelPack, printLuaDocs, headlessB, server] = parseArgs(argc, argv);
    const auto headless = headlessB; // Workaround binding capture

    //
    //
    // ------------------------------------------------------------------------
    // Print Lua docs mode
    if (printLuaDocs)
    {
        return mainPrintLuaDocs();
    }

    //
    //
    // ------------------------------------------------------------------------
    // Server mode
    if (server)
    {
        return mainServer();
    }

    //
    //
    // ------------------------------------------------------------------------
    // Client mode
    SSVOH_ASSERT(!printLuaDocs);
    SSVOH_ASSERT(!server);
    return mainClient(headless, args, cliLevelName, cliLevelPack);
}
