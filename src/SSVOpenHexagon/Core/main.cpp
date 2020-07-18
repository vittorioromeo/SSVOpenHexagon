// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Online/Server.hpp"
#include "SSVOpenHexagon/Online/OHServer.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

#include <SSVStart/GameSystem/GameWindow.hpp>

static void createProfilesFolder()
{
    const ssvu::FileSystem::Path profilesPath{"Profiles/"};

    if(profilesPath.exists<ssvufs::Type::Folder>())
    {
        return;
    }

    ssvu::lo("::createProfilesFolder")
        << "Profiles folder does not exist, creating\n";

    createFolder(profilesPath);
}

int main(int argc, char* argv[])
{
    // Steam integration
    hg::Steam::steam_manager steamManager;
    steamManager.request_stats_and_achievements();

    // Discord integration
    hg::Discord::discord_manager discordManager;

    hg::Online::setCurrentGtm(
        std::make_unique<hg::Online::GlobalThreadManager>());

    const auto overrideIds = [&] {
        std::vector<std::string> result;
        for(int i{0}; i < argc; ++i)
        {
            result.emplace_back(argv[i]);
        }
        return result;
    }();

    if(ssvu::contains(overrideIds, "server"))
    {
        hg::Config::loadConfig(overrideIds);

        auto levelOnlyAssets = std::make_unique<hg::HGAssets>(
            steamManager, true /* mLevelsOnly */);
        hg::Online::initializeValidators(*levelOnlyAssets);

        auto ohServer = std::make_unique<hg::Online::OHServer>();
        ohServer->start();

        return 0;
    }

    createProfilesFolder();

    hg::Online::initializeClient();
    hg::Online::tryConnectToServer();

    hg::Config::loadConfig(overrideIds);

    if(hg::Config::getServerLocal())
    {
        ssvu::lo("Server") << "LOCAL MODE ON\n";
    }

    ssvs::GameWindow window;
    window.setTitle("Open Hexagon " +
                    std::string{hg::Config::getVersionString()} +
                    " - by vittorio romeo - http://vittorioromeo.info");
    window.setSize(hg::Config::getWidth(), hg::Config::getHeight());
    window.setPixelMult(hg::Config::getPixelMultiplier());
    window.setFullscreen(hg::Config::getFullscreen());
    window.setAntialiasingLevel(hg::Config::getAntialiasingLevel());
    window.setVsync(hg::Config::getVsync());
    window.setFPSLimited(hg::Config::getLimitFPS());
    window.setMaxFPS(hg::Config::getMaxFPS());

    hg::Config::setTimerStatic(window, hg::Config::getTimerStatic());

    auto assets = std::make_unique<hg::HGAssets>(steamManager);
    hg::Online::initializeValidators(*assets);

    auto hg = std::make_unique<hg::HexagonGame>(
        steamManager, discordManager, *assets, window);

    auto mg = std::make_unique<hg::MenuGame>(
        steamManager, discordManager, *assets, *hg, window);

    hg->mgPtr = mg.get();

    assets->refreshVolumes();
    window.setGameState(mg->getGame());
    mg->init(false /* mError */);

    window.run();

    if(hg::Online::getLoginStatus() != hg::Online::LoginStat::Logged)
    {
        hg::Online::logout();
    }

    ssvu::lo().flush();

    hg::Config::saveConfig();
    assets->pSaveCurrent();

    ssvu::saveLogToFile("log.txt");
    hg::Online::cleanup();

    return 0;
}
