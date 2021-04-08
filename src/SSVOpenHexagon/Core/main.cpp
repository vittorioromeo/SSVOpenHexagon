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
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/ScopeGuard.hpp"

#include <sodium.h>

#include <imgui.h>
#include <imgui-SFML.h>

#include <SSVStart/GameSystem/GameWindow.hpp>

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <filesystem>
#include <string_view>
#include <csignal>
#include <cstdlib>
#include <cstdio>

namespace
{

void createFolderIfNonExistant(const std::string& folderName)
{
    const ssvu::FileSystem::Path path{folderName};

    if(path.exists<ssvufs::Type::Folder>())
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

ParsedArgs parseArgs(const int argc, char* argv[])
{
    ParsedArgs result;

    for(int i = 0; i < argc; ++i)
    {
        // Find command-line pack name (to immediately run level)
        if(!std::strcmp(argv[i], "-p") && i + 1 < argc)
        {
            ++i;
            result.cliLevelPack = argv[i];
            continue;
        }

        // Find command-line level name (to immediately run level)
        if(!std::strcmp(argv[i], "-l") && i + 1 < argc)
        {
            ++i;
            result.cliLevelName = argv[i];
            continue;
        }

        // Find command-line argument to print Lua docs
        if(!std::strcmp(argv[i], "-printLuaDocs"))
        {
            result.printLuaDocs = true;
            continue;
        }

        // Find command-line argument to run in headless mode
        if(!std::strcmp(argv[i], "-headless"))
        {
            result.headless = true;
            continue;
        }

        // Find command-line argument to run in server mode
        if(!std::strcmp(argv[i], "-server"))
        {
            result.server = true;
            continue;
        }

        result.args.emplace_back(argv[i]);
    }

    return result;
}

std::string makeWindowTitle()
{
    return "Open Hexagon " + hg::Config::getVersionString() +
           " - by Vittorio Romeo - https://vittorioromeo.info";
}

std::optional<std::string> getFirstReplayFilenameFromArgs(
    const std::vector<std::string>& args)
{
    for(const std::string& arg : args)
    {
        if(arg.find(".ohreplay") != std::string::npos)
        {
            return arg;
        }
    }

    return std::nullopt;
}

} // namespace

int main(int argc, char* argv[])
{
    if(argc < 1)
    {
        std::cerr << "Fatal error: no executable specified" << std::endl;
        return -1;
    }

    //
    //
    // ------------------------------------------------------------------------
    // libsodium initialization
    if(sodium_init() < 0)
    {
        ssvu::lo("::main") << "Failed initializing libsodium\n";
        return 1;
    }

    //
    //
    // ------------------------------------------------------------------------
    // Basic signal handling
    // TODO: UB
    std::signal(SIGINT, [](int s) {
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
    const auto [args, cliLevelName, cliLevelPack, printLuaDocs, headless,
        server] = parseArgs(argc, argv);

    // ------------------------------------------------------------------------
    // Steam integration
    hg::Steam::steam_manager steamManager;

    if(steamManager.is_initialized() && !server)
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

    if(!headless && !server)
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
    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Saving config...\n";
        hg::Config::saveConfig();
        ssvu::lo("::main") << "Done saving config\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Create the game window
    std::optional<ssvs::GameWindow> window;

    if(!headless)
    {
        window.emplace();

        window->setTitle(makeWindowTitle());
        window->setSize(hg::Config::getWidth(), hg::Config::getHeight());
        window->setPixelMult(hg::Config::getPixelMultiplier());
        window->setFullscreen(hg::Config::getFullscreen());
        window->setAntialiasingLevel(hg::Config::getAntialiasingLevel());
        window->setVsync(hg::Config::getVsync());
        window->setFPSLimited(hg::Config::getLimitFPS());
        window->setMaxFPS(hg::Config::getMaxFPS());

        // 240 ticks per second.
        window->setTimer<ssvs::TimerStatic>(
            hg::Config::TIME_STEP, hg::Config::TIME_SLICE);

        // Signal handling: exit gracefully on CTRL-C
        {
            static ssvs::GameWindow& globalWindow = *window;

            // TODO: UB
            std::signal(SIGINT, [](int s) {
                ssvu::lo("::main")
                    << "Caught signal '" << s << "' with game window open\n";

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
    if(!headless)
    {
        SSVOH_ASSERT(window.has_value());
        ImGui::SFML::Init(*window);
    }

    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Shutting down ImGui...\n";

        if(!headless)
        {
            ImGui::SFML::Shutdown();
        }

        ssvu::lo("::main") << "Done shutting down ImGui...\n";
    });

    //
    //
    // ------------------------------------------------------------------------
    // Initialize assets
    auto assets = std::make_unique<hg::HGAssets>(steamManager, headless);
    HG_SCOPE_GUARD({
        ssvu::lo("::main") << "Saving all local profiles...\n";
        assets->pSaveAll();
        ssvu::lo("::main") << "Done saving all local profiles\n";
    });

    assets->refreshVolumes();

    // ------------------------------------------------------------------------
    // Initialize hexagon client
    std::unique_ptr<hg::HexagonClient> hc;

    if(!server)
    {
        hc = std::make_unique<hg::HexagonClient>(steamManager);
    }

    //
    //
    // ------------------------------------------------------------------------
    // Initialize hexagon game
    SSVOH_ASSERT(assets != nullptr);

    auto hg = std::make_unique<hg::HexagonGame>(steamManager,
        (discordManager.has_value() ? &*discordManager : nullptr), *assets,
        (window.has_value() ? &*window : nullptr), &*hc);

    if(printLuaDocs)
    {
        hg->initLuaAndPrintDocs();
        return 0;
    }

    //
    //
    // ------------------------------------------------------------------------
    // Initialize menu game and link to hexagon game
    std::unique_ptr<hg::MenuGame> mg;

    if(!headless && !server)
    {
        SSVOH_ASSERT(window.has_value());
        SSVOH_ASSERT(discordManager.has_value());
        SSVOH_ASSERT(assets != nullptr);
        SSVOH_ASSERT(hg != nullptr);
        SSVOH_ASSERT(hc != nullptr);

        mg = std::make_unique<hg::MenuGame>(
            steamManager, *discordManager, *assets, *hg, *window, *hc);

        hg->mgPtr = mg.get();
    }
    else
    {
        hg->mgPtr = nullptr;
    }

    //
    //
    // ------------------------------------------------------------------------
    // Load drag & drop replay, if any -- otherwise run game as normal
    const std::optional<std::string> replayFilename =
        getFirstReplayFilenameFromArgs(args);

    if(!headless)
    {
        SSVOH_ASSERT(window.has_value());

        const auto gotoMenu = [&] {
            window->setGameState(mg->getGame());
            mg->init(false /* mError */);
        };

        const auto gotoGameReplay = [&](const hg::replay_file& replayFile) {
            hg->setLastReplay(replayFile);

            hg->newGame(replayFile._pack_id, replayFile._level_id,
                replayFile._first_play, replayFile._difficulty_mult,
                /* mExecuteLastReplay */ true);

            window->setGameState(hg->getGame());
        };

        if(!replayFilename.has_value())
        {
            if(cliLevelPack.has_value() && cliLevelName.has_value())
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
            if(hg::replay_file rf; rf.deserialize_from_file(*replayFilename))
            {
                ssvu::lo("Replay")
                    << "Playing replay file '" << *replayFilename << "'\n";

                gotoGameReplay(rf);
            }
            else
            {
                ssvu::lo("Replay") << "Failed to read replay file '"
                                   << replayFilename.value() << "'\n";

                gotoMenu();
            }
        }
    }
    else
    {
        SSVOH_ASSERT(headless);

        if(server)
        {
            auto hs = std::make_unique<hg::HexagonServer>(*assets, *hg);
        }
        else
        {
            // TODO: code repetition, cleanup
            if(!replayFilename.has_value())
            {
                std::cout << "TODO: Running in headless mode without replay...?"
                          << std::endl;
            }
            else
            {
                if(hg::replay_file rf;
                    rf.deserialize_from_file(*replayFilename))
                {
                    ssvu::lo("Replay")
                        << "Playing replay file in headless mode '"
                        << *replayFilename << "'\n";

                    // TODO: check level validity

                    std::cout << "Player died.\nFinal time: "
                              << hg->runReplayUntilDeathAndGetScore(rf) << '\n';
                }
                else
                {
                    ssvu::lo("Replay")
                        << "Failed to read replay file in headless mode '"
                        << replayFilename.value() << "'\n";
                }
            }
        }
    }

    //
    //
    // ------------------------------------------------------------------------
    // Run the game!
    if(!headless)
    {
        SSVOH_ASSERT(window.has_value());
        window->run();
    }

    ssvu::lo("::main") << "Reached end of 'main'\n";
    return 0;
}
