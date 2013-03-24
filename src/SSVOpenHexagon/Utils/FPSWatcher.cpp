// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVUtils/SSVUtils.h>
#include <SFML/System.hpp>
#include "SSVOpenHexagon/Utils/FPSWatcher.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace ssvu;

namespace hg
{
	void FPSWatcher::watch()
	{
		while(running)
		{
			if(!disabled)
			{
				if(check)
				{
					check = false;
					sleep(milliseconds(50));
					while(check == false) { loseFrame(); sleep(milliseconds(12)); }
				}
				sleep(milliseconds(80));
				if(gameWindow.getFPS() < minFPS) loseFrame();
			}
			sleep(milliseconds(80));
		}
	}
	void FPSWatcher::loseFrame()
	{
		if(lostFrames > maxLostFrames) return;
		++lostFrames;
		log("Slowdown " + toStr(lostFrames) + "/" + toStr(maxLostFrames), "Performance");
	}

	FPSWatcher::FPSWatcher(GameWindow& mGameWindow) : gameWindow(mGameWindow), thread([&]{ watch(); }) { thread.launch(); }
	FPSWatcher::~FPSWatcher() { running = false; }
	bool FPSWatcher::isLimitReached() { return lostFrames >= maxLostFrames; }

	void FPSWatcher::enable() 	{ disabled = false; }
	void FPSWatcher::disable() 	{ disabled = true; }
	void FPSWatcher::reset() 	{ lostFrames = 0; disabled = true; check = false; }
	void FPSWatcher::update() 	{ check = true; }
}

