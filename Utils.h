#ifndef UTILS_H_HG
#define UTILS_H_HG

#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;
using namespace sf;

namespace hg
{
	Vector2f orbit(const Vector2f&, const float, const float);
	Vector2f normalize(const Vector2f);
	int rnd(int, int);
	bool pnpoly(std::vector<Vector2f*>, Vector2f);

	template<class T>
	T toRadians(const T mValue)
	{
		return mValue / 57.3f;
	}

	template<class T>
	string toStr(const T &t)
	{
		ostringstream oss;
		oss << t;
		return string(oss.str());
	}

	Color hue2color(double);
	Color darkenColor(Color, float);

	float saturate(float);
	float smootherstep(float, float, float);

	template <class T>
	int sign(T value)
	{
		if (value > 0) return 1; else return -1;
	}
}

#endif /* UTILS_H_HG */
