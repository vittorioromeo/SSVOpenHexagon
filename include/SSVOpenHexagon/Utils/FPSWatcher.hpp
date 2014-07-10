// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS_FPSWATCHER
#define HG_UTILS_FPSWATCHER

#include <thread>
#include <chrono>
#include <SSVStart/SSVStart.hpp>

namespace hg
{
	class FPSWatcher
	{
		private:
			ssvs::GameWindow& gameWindow;
			float lostFrames{0};
			const float maxLostFrames{20.f}, minFPS{25.f};
			bool disabled{true}, running{true}, check{false};
			std::future<void> watchFuture{std::async(std::launch::async, [this]{ watch(); })};

			inline void watch()
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
			inline void loseFrame()
			{
				if(lostFrames > maxLostFrames) return;
				++lostFrames;
				ssvu::lo("FPSWatcher::watch") << "Slowdown " << lostFrames << "/" << maxLostFrames << "\n";
			}

		public:
			FPSWatcher(ssvs::GameWindow& mGameWindow) : gameWindow(mGameWindow) { }
			~FPSWatcher() { running = false; }

			inline bool isLimitReached() const	{ return lostFrames >= maxLostFrames; }
			inline void reset()					{ lostFrames = 0; disabled = true; check = false; }
			inline void update()				{ check = true; }
			inline void enable()				{ disabled = false; }
			inline void disable()				{ disabled = true; }
	};
}

#endif
