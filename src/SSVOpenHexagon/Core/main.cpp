// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

#include <SSVStart/GameSystem/GameWindow.hpp>

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <filesystem>
#include <string_view>

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

    // ------------------------------------------------------------------------
    // Set working directory to current executable location
    std::filesystem::current_path(std::filesystem::path{argv[0]}.parent_path());

    // ------------------------------------------------------------------------
    // Steam integration
    hg::Steam::steam_manager steamManager;
    steamManager.request_stats_and_achievements();

    // ------------------------------------------------------------------------
    // Discord integration
    hg::Discord::discord_manager discordManager;

    // ------------------------------------------------------------------------
    // Create game folders if needed
    createFolderIfNonExistant("Profiles/");
    createFolderIfNonExistant("Replays/");

    // ------------------------------------------------------------------------
    // Parse arguments and load configuration (and overrides)
    const auto [args, cliLevelName, cliLevelPack, printLuaDocs] =
        parseArgs(argc, argv);

    hg::Config::loadConfig(args);

    // ------------------------------------------------------------------------
    // Create the game window
    ssvs::GameWindow window;

    window.setTitle(makeWindowTitle());
    window.setSize(hg::Config::getWidth(), hg::Config::getHeight());
    window.setPixelMult(hg::Config::getPixelMultiplier());
    window.setFullscreen(hg::Config::getFullscreen());
    window.setAntialiasingLevel(hg::Config::getAntialiasingLevel());
    window.setVsync(hg::Config::getVsync());
    window.setFPSLimited(hg::Config::getLimitFPS());
    window.setMaxFPS(hg::Config::getMaxFPS());

    window.setTimer<ssvs::TimerStatic>(0.25f, 0.25f);

    // ------------------------------------------------------------------------
    // Initialize IMGUI
    ImGui::SFML::Init(window);

    // ------------------------------------------------------------------------
    // Create the game and menu states
    auto assets = std::make_unique<hg::HGAssets>(steamManager);

    auto hg = std::make_unique<hg::HexagonGame>(
        steamManager, discordManager, *assets, window, printLuaDocs);

    auto mg = std::make_unique<hg::MenuGame>(
        steamManager, discordManager, *assets, *hg, window);

    hg->mgPtr = mg.get();

    assets->refreshVolumes();

    // ------------------------------------------------------------------------
    // Load drag & drop replay, if any -- otherwise run game as normal
    const std::optional<std::string> replayFilename =
        getFirstReplayFilenameFromArgs(args);

    const auto gotoMenu = [&] {
        window.setGameState(mg->getGame());
        mg->init(false /* mError */);
    };

    const auto gotoGameReplay = [&](const hg::replay_file& replayFile) {
        hg->setLastReplay(replayFile);

        hg->newGame(replayFile._pack_id, replayFile._level_id,
            replayFile._first_play, replayFile._difficulty_mult,
            /* mExecuteLastReplay */ true);

        window.setGameState(hg->getGame());
    };

    if(!replayFilename.has_value())
    {
        if(cliLevelPack.has_value() && cliLevelName.has_value())
        {
            mg->init(false /* mError */, *cliLevelPack, *cliLevelName);
        }
        else
        {
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

    // ------------------------------------------------------------------------
    // Run the game!
    window.run();

    // ------------------------------------------------------------------------
    // Shut down IMGUI
    ImGui::SFML::Shutdown();

    // ------------------------------------------------------------------------
    // Flush output, save configuration and log
    ssvu::lo().flush();

    hg::Config::saveConfig();
    assets->pSaveAll();

    ssvu::saveLogToFile("log.txt");

    return 0;
}
