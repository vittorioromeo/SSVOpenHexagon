#ifndef HGSTATUS_H
#define HGSTATUS_H

#include <SFML/Graphics.hpp>

namespace hg
{
	struct HexagonGameStatus
	{
		float currentTime{0}, incrementTime{0}, timeStop{50};
		float pulse{75}, pulseDirection{1}, pulseDelay{0}, pulseDelayHalf{0};
		float beatPulse{0}, beatPulseDelay{0};
		float flashEffect{0};
		float radius{75};
		float fastSpin{0};
		bool hasDied{false}, mustRestart{false};
		bool randomSideChangesEnabled{true};
		bool incrementEnabled{true};
		float effect3DDir{1.f}, effect3D{1.f};
		bool mainColorOverride{false};
		sf::Color overrideColor{0, 0, 0, 0};
	};
}

#endif // HGSTATUS_H
