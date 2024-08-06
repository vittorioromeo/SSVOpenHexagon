// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/HexagonServer.hpp"
#include "SSVOpenHexagon/Core/HexagonClient.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Imgui.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/ScopeGuard.hpp"
#include "SSVOpenHexagon/Utils/VectorToSet.hpp"

#include <sodium.h>

#include <SSVStart/GameSystem/GameWindow.hpp>

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Log/Log.inl>
#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <SFML/Graphics/Image.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/IpAddressUtils.hpp>

#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

#include <SFML/Graphics/GraphicsContext.hpp>

#include <SFML/Base/Optional.hpp>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

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

namespace {

void createFolderIfNonExistant(const std::string& folderName)
{
    const ssvu::FileSystem::Path path{folderName};

    if (path.isFolder())
    {
        return;
    }

    ssvu::lo("::createFolderIfNonExistant")
        << "'" << folderName << "' folder does not exist, creating\n";

    createFolder(path);
}

struct ParsedArgs
{
    std::vector<std::string> args;
    std::optional<std::string> cliLevelName;
    std::optional<std::string> cliLevelPack;
    bool printLuaDocs{false};
    bool headless{false};
    bool server{false};
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
            result.cliLevelPack = argv[i];
            continue;
        }

        // Find command-line level name (to immediately run level)
        if (!std::strcmp(argv[i], "-l") && i + 1 < argc)
        {
            ++i;
            result.cliLevelName = argv[i];
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

        result.args.emplace_back(argv[i]);
    }

    return result;
}

[[nodiscard]] std::string makeWindowTitle()
{
    return hg::Utils::concat("Open Hexagon ", hg::GAME_VERSION_STR,
        " - by Vittorio Romeo - https://vittorioromeo.info");
}

[[nodiscard]] std::optional<std::string>
getFirstCompressedReplayFilenameFromArgs(const std::vector<std::string>& args)
{
    for (const std::string& arg : args)
    {
        if (arg.find(".ohr.z") != std::string::npos)
        {
            return arg;
        }
    }

    return std::nullopt;
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
        nullptr, /* graphicsContext */ //
        nullptr, /* steamManager */    //
        true /* headless */            //
    };

    hg::HexagonGame hg{
        nullptr, /* graphicsContext */ //
        nullptr, /* steamManager */    //
        nullptr, /* discordManager */  //
        assets,                        //
        nullptr, /* audio */           //
        nullptr, /* window */          //
        nullptr /* client */           //
    };

    std::cout << "\n\n\n\n\n";
    hg.initLuaAndPrintDocs();
    std::cout << "\n\n\n\n\n";

    ssvu::lo("::mainPrintLuaDocs") << "Finished\n";
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

    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Saving config...\n";
        hg::Config::saveConfig();
        ssvu::lo("::main") << "Done saving config\n";
    });

    hg::HGAssets assets{
        nullptr, /* graphicsContext */ //
        &steamManager,                 //
        true /* headless */            //
    };

    hg::HexagonGame hg{
        nullptr, /* graphicsContext */ //
        nullptr /* steamManager */,    //
        nullptr /* discordManager */,  //
        assets,                        //
        nullptr /* audio */,           //
        nullptr /* window */,          //
        nullptr /* client */           //
    };

    // TODO (P0): handle `resolve` errors
    hg::HexagonServer hs{
        assets,                                                          //
        hg,                                                              //
        sf::IpAddressUtils::resolve(hg::Config::getServerIp()).value(),  //
        hg::Config::getServerPort(),                                     //
        hg::Config::getServerControlPort(),                              //
        hg::Utils::toUnorderedSet(hg::Config::getServerLevelWhitelist()) //
    };

    ssvu::lo("::mainServer") << "Finished\n";
    return 0;
}

//
//
// ----------------------------------------------------------------------------
// Client main entrypoint
// ----------------------------------------------------------------------------

[[nodiscard]] int mainClient(const bool headless,
    const std::vector<std::string>& args,
    const std::optional<std::string>& cliLevelName,
    const std::optional<std::string>& cliLevelPack)
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
    std::optional<hg::Discord::discord_manager> discordManager;

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
    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Saving config...\n";
        hg::Config::saveConfig();
        ssvu::lo("::main") << "Done saving config\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Create the game window
    sf::GraphicsContext graphicsContext;
    std::optional<ssvs::GameWindow> window;

    if (!headless)
    {
        window.emplace(graphicsContext);

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
                const sf::base::Optional icon =
                    sf::Image::loadFromFile("Assets/icon.png");

                if (!icon.hasValue())
                {
                    ssvu::lo("::main") << "Failed to load icon image\n";
                    return;
                }

                window->getRenderWindow().setIcon(
                    {icon->getSize().x, icon->getSize().y},
                    icon->getPixelsPtr());
            };

            window->onRecreation += resetIcon;
            resetIcon();
        }

        // 240 ticks per second.
        window->setTimer<ssvs::TimerStatic>(
            hg::Config::TIME_STEP, hg::Config::TIME_SLICE);

        // Signal handling: exit gracefully on CTRL-C
        {
            SSVOH_ASSERT(window.has_value());
            static ssvs::GameWindow& globalWindow = *window;

            // TODO (P2): UB
            std::signal(SIGINT,
                [](int s)
                {
                    ssvu::lo("::main") << "Caught signal '" << s
                                       << "' with game window open\n";

                    ssvu::lo("::main") << "Stopping game window...\n";
                    globalWindow.stop();
                    ssvu::lo("::main") << "Done stopping game window\n";
                });
        }
    }

    //
    //
    // ------------------------------------------------------------------------
    // Initialize IMGUI
    if (!headless)
    {
        SSVOH_ASSERT(window.has_value());
        if (!hg::Imgui::initialize(graphicsContext, *window))
        {
            ssvu::lo("::main") << "Failed to initialize ImGui...\n";
        }
    }

    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Shutting down ImGui...\n";

        if (!headless)
        {
            hg::Imgui::shutdown();
        }

        ssvu::lo("::main") << "Done shutting down ImGui...\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Initialize assets
    hg::HGAssets assets{&graphicsContext, &steamManager, headless};
    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Saving all local profiles...\n";
        assets.pSaveAll();
        ssvu::lo("::main") << "Done saving all local profiles\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Initialize audio
    auto audioContext = sf::AudioContext::create().value();
    auto playbackDevice =
        sf::PlaybackDevice::createDefault(audioContext).value();

    hg::Audio audio{
        //
        playbackDevice,
        [&assets](const std::string& assetId) -> sf::SoundBuffer*
        { return assets.getSoundBuffer(assetId); }, //
        [&assets](const std::string& assetId) -> const std::string*
        { return assets.getMusicPath(assetId); } //
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
        &graphicsContext,
        &steamManager,                                             //
        (discordManager.has_value() ? &*discordManager : nullptr), //
        assets,                                                    //
        &audio,                                                    //
        (window.has_value() ? &*window : nullptr),                 //
        &hc                                                        //
    };

    //
    //
    // ------------------------------------------------------------------------
    // Initialize menu game and link to hexagon game
    std::optional<hg::MenuGame> mg;

    if (!headless)
    {
        SSVOH_ASSERT(window.has_value());
        SSVOH_ASSERT(discordManager.has_value());

        mg.emplace(graphicsContext, steamManager, *discordManager, assets,
            audio, *window, hc);

        mg->fnHGTriggerRefresh = [&](const ssvs::Input::Trigger& trigger,
                                     int bindId) //
        {
            hg.refreshTrigger(trigger, bindId); //
        };

        mg->fnHGNewGame = [&](const std::string& packId,
                              const std::string& levelId, bool firstPlay,
                              float diffMult, bool executeLastReplay)
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
    }

    //
    //
    // ------------------------------------------------------------------------
    // Load drag & drop replay, if any -- otherwise run game as normal
    const std::optional<std::string> compressedReplayFilename =
        getFirstCompressedReplayFilenameFromArgs(args);

    if (!headless)
    {
        SSVOH_ASSERT(window.has_value());

        const auto gotoMenu = [&]
        {
            mg->init(false /* mError */);
            window->setGameState(mg->getGame());
        };

        const auto gotoGameCompressedReplay =
            [&](const hg::compressed_replay_file& compressedReplayFile)
        {
            std::optional<hg::replay_file> replayFileOpt =
                hg::decompress_replay_file(compressedReplayFile);

            if (!replayFileOpt.has_value())
            {
                std::cerr << "Could not decompress replay file\n";
                return;
            }

            hg::replay_file& replayFile = replayFileOpt.value();

            hg.setLastReplay(replayFile);

            hg.newGame(replayFile._pack_id, replayFile._level_id,
                replayFile._first_play, replayFile._difficulty_mult,
                /* mExecuteLastReplay */ true);

            window->setGameState(hg.getGame());
        };

        if (!compressedReplayFilename.has_value())
        {
            if (cliLevelPack.has_value() && cliLevelName.has_value())
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
            if (hg::compressed_replay_file crf;
                crf.deserialize_from_file(*compressedReplayFilename))
            {
                ssvu::lo("Replay") << "Playing compressed replay file '"
                                   << *compressedReplayFilename << "'\n";

                gotoGameCompressedReplay(crf);
            }
            else
            {
                ssvu::lo("Replay") << "Failed to read compressed replay file '"
                                   << compressedReplayFilename.value() << "'\n";

                gotoMenu();
            }
        }
    }
    else
    {
        SSVOH_ASSERT(headless);

        // TODO (P2): code repetition, cleanup
        if (!compressedReplayFilename.has_value())
        {
            std::cout << "Running in headless mode without replay...?\n";
            return 1;
        }

        if (hg::compressed_replay_file crf;
            crf.deserialize_from_file(*compressedReplayFilename))
        {
            std::optional<hg::replay_file> replayFileOpt =
                hg::decompress_replay_file(crf);

            if (!replayFileOpt.has_value())
            {
                std::cerr << "Could not decompress replay file\n";
                return 1;
            }

            hg::replay_file& replayFile = replayFileOpt.value();

            ssvu::lo("Replay")
                << "Playing compressed replay file in headless mode '"
                << *compressedReplayFilename << "'\n";

            // TODO (P2): check level validity

            std::cout << "Player died.\nFinal time: "
                      << hg.runReplayUntilDeathAndGetScore(replayFile,
                               1 /* maxProcessingSeconds */,
                               1.f /* timescale */)
                             .value()
                             .playedTimeSeconds
                      << '\n';
        }
        else
        {
            ssvu::lo("Replay")
                << "Failed to read compressed replay file in headless mode '"
                << compressedReplayFilename.value() << "'\n";
        }
    }

    //
    //
    // ------------------------------------------------------------------------
    // Run the game!
    if (!headless)
    {
        SSVOH_ASSERT(window.has_value());
        window->run(graphicsContext);
    }

    ssvu::lo("::mainClient") << "Finished\n";
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
        ssvu::lo("::main") << "Failed initializing libsodium\n";
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
            ssvu::lo("::main") << "Caught signal '" << s
                               << "' without game window open, exiting...\n";

            std::exit(1);
        });

    //
    //
    // ------------------------------------------------------------------------
    // Flush and save log (at the end of the scope)
    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Saving log to 'log.txt'...\n";

        ssvu::lo().flush();
        ssvu::saveLogToFile("log.txt");

        ssvu::lo("::main") << "Done saving log to 'log.txt'\n";
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
    const auto [args, cliLevelName, cliLevelPack, printLuaDocs, headlessB,
        server] = parseArgs(argc, argv);
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
