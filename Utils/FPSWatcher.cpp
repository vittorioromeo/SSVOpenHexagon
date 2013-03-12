// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SFML/System.hpp>
#include "FPSWatcher.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;

namespace hg
{
	void FPSWatcher::watch()
	{
		while(true)
		{
			if(!disabled)
			{
				float lastFps{gameWindow.getFPS()};
				sleep(milliseconds(20));
				if(gameWindow.getFPS() < minFPS || (gameWindow.getFPS() == lastFps && lostFrames <= maxLostFrames)) loseFrame();
			}
			sleep(milliseconds(20));
		}
	}
	void FPSWatcher::loseFrame() { ++lostFrames; log("Slowdown " + toStr(lostFrames) + "/" + toStr(maxLostFrames), "Performance"); }

	FPSWatcher::FPSWatcher(GameWindow& mGameWindow) : gameWindow(mGameWindow), thread([&]{ watch(); }) { thread.launch(); }
	bool FPSWatcher::isLimitReached() { return lostFrames >= maxLostFrames; }

	void FPSWatcher::enable() { disabled = false; }
	void FPSWatcher::disable() { disabled = true; }
	void FPSWatcher::reset() { lostFrames = 0; disabled = true; }
}

