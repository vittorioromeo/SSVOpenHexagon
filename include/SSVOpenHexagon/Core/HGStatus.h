// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_STATUS
#define HG_STATUS

#include <SFML/Graphics.hpp>
#include <SSVUtils/SSVUtils.h>

namespace hg
{
	struct HexagonGameStatus
	{
		ssvu::ObfuscatedValue<float> currentTime{0};
		float incrementTime{0}, timeStop{50};
		float pulse{75}, pulseDirection{1}, pulseDelay{0}, pulseDelayHalf{0};
		float beatPulse{0}, beatPulseDelay{0};
		float pulse3D{1.0f}, pulse3DDirection{1};
		float flashEffect{0};
		float radius{75};
		float fastSpin{0};
		bool hasDied{false}, mustRestart{false};
		bool drawing3D{false};
		bool scoreInvalid{false};
		sf::Color overrideColor{0, 0, 0, 0};
		ssvu::ObfuscatedValue<float> lostFrames{0};
	};
}

#endif
