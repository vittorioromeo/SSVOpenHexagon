// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Online/Server.hpp"
#include "SSVOpenHexagon/Online/OHServer.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

using namespace std;
using namespace ssvs;
using namespace ssvu;
using namespace ssvu::FileSystem;
using namespace hg;

void createProfilesFolder()
{
	Path profilesPath{"Profiles/"};
	if(profilesPath.exists<ssvufs::Type::Folder>()) return;

	lo("::createProfilesFolder") << "Profiles folder does not exist, creating\n";
	createFolder(profilesPath);
}

int main(int argc, char* argv[])
{
	#define LOSIZ(x) ssvu::lo(#x) << sizeof(x) << std::endl

	SSVUT_RUN();

	// LOSIZ(Online::GlobalThreadManager);
	// LOSIZ(HGAssets);
	// LOSIZ(Online::OHServer);
	// LOSIZ(GameWindow);

	Online::GlobalThreadManager gtm;
	Online::setCurrentGtm(gtm);

	vector<string> overrideIds; for(int i{0}; i < argc; ++i) overrideIds.emplace_back(argv[i]);

	if(contains(overrideIds, "server"))
	{
		Config::loadConfig(overrideIds);
		auto levelOnlyAssets(mkUPtr<HGAssets>(true));
		Online::initializeValidators(*levelOnlyAssets);
		auto ohServer(mkUPtr<Online::OHServer>());
		ohServer->start();
		return 0;
	}

	createProfilesFolder();

	Online::initializeClient();
	Online::tryConnectToServer();

	Config::loadConfig(overrideIds);

	if(Config::getServerLocal()) ssvu::lo("Server") << "LOCAL MODE ON" << std::endl;

	GameWindow window;
	window.setTitle("Open Hexagon " + toStr(Config::getVersion()) + " - by vittorio romeo - http://vittorioromeo.info");
	window.setSize(Config::getWidth(), Config::getHeight());
	window.setPixelMult(Config::getPixelMultiplier());
	window.setFullscreen(Config::getFullscreen());
	window.setVsync(Config::getVsync());
	window.setFPSLimited(Config::getLimitFPS());
	window.setMaxFPS(Config::getMaxFPS());
	window.setMouseCursorVisible(Config::getMouseVisible());

	Config::setTimerStatic(window, Config::getTimerStatic());

	auto assets(mkUPtr<HGAssets>());
	Online::initializeValidators(*assets);
	auto hg(mkUPtr<HexagonGame>(*assets, window));
	auto mg(mkUPtr<MenuGame>(*assets, *hg, window));
	hg->mgPtr = mg.get();

	assets->refreshVolumes();
	window.setGameState(mg->getGame());
	mg->init();
	window.run();

	if(Online::getLoginStatus() != Online::LoginStat::Logged) Online::logout();

	ssvu::lo().flush();

	Config::saveConfig(); assets->pSaveCurrent(); saveLogToFile("log.txt");
	Online::cleanup();

	return 0;
}
