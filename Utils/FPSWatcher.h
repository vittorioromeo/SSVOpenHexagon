// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_UTILS_FPSWATCHER
#define HG_UTILS_FPSWATCHER

#include <SSVStart.h>
#include "Utils/ThreadWrapper.h"

namespace hg
{
	class FPSWatcher
	{
		private:
			ssvs::GameWindow& gameWindow;
			bool check{false};
			ssvs::Utils::ThreadWrapper thread;
			float lostFrames{0};
			const float maxLostFrames{20}, minFPS{20};
			bool disabled{true}, running{true};

			void watch();
			void loseFrame();

		public:
			FPSWatcher(ssvs::GameWindow& mGameWindow);
			~FPSWatcher();
			bool isLimitReached();
			void reset();
			void update();

			void enable();
			void disable();
	};
}

#endif
