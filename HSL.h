#ifndef HSL_H
#define HSL_H

#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

namespace hg
{
	struct HSL
	{
		int Hue;
		int Saturation;
		int Luminance;

		HSL();
		HSL(int H, int S, int L);

		sf::Color TurnToRGB();

		private:
			float HueToRGB(float arg1, float arg2, float H);
	};

	HSL TurnToHSL(const sf::Color& C);
	HSL RGBtoHSL(sf::Color mColor);
}

#endif // HSL_H
