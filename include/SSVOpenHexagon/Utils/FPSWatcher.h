// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS_FPSWATCHER
#define HG_UTILS_FPSWATCHER

#include <thread>
#include <chrono>
#include <SSVStart/SSVStart.h>

namespace hg
{
	class FPSWatcher
	{
		private:
			ssvs::GameWindow& gameWindow;
			bool check{false};
			std::thread watcherThread;
			float lostFrames{0};
			const float maxLostFrames{20}, minFPS{20};
			bool disabled{true}, running{true};

			void watch()
			{
				while(running)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(80));

					if(disabled) continue;

					if(check)
					{
						check = false;
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
						while(check == false) { loseFrame(); std::this_thread::sleep_for(std::chrono::milliseconds(12)); }
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(80));
					if(gameWindow.getFPS() < minFPS) loseFrame();
				}
			}
			void loseFrame()
			{
				if(lostFrames > maxLostFrames) return;
				++lostFrames;
				ssvu::log("Slowdown " + ssvu::toStr(lostFrames) + "/" + ssvu::toStr(maxLostFrames), "FPSWatcher::watch");
			}

		public:
			FPSWatcher(ssvs::GameWindow& mGameWindow) : gameWindow(mGameWindow), watcherThread([&]{ watch(); }) { watcherThread.detach(); }

			inline bool isLimitReached() const	{ return lostFrames >= maxLostFrames; }
			inline void reset()					{ lostFrames = 0; disabled = true; check = false; }
			inline void update()				{ check = true; }
			inline void enable()				{ disabled = false; }
			inline void disable()				{ disabled = true; }
	};
}

#endif
