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
	

	template<class T> T toRadians(const T mValue) { return mValue / 57.3f; }
	template <class T> int sign(T value) { if (value > 0) return 1; else return -1; }

	template<class T>
	string toStr(const T &t)
	{
		ostringstream oss;
		oss << t;
		return string(oss.str());
	}

	Vector2f orbit(const Vector2f&, const float, const float);
	Vector2f normalize(const Vector2f);
	Color hue2color(double);
	Color darkenColor(Color, float);

	bool pnpoly(std::vector<Vector2f*>, Vector2f);
	int rnd(int, int);
	float saturate(float);
	float smootherstep(float, float, float);
	
	string exePath(); // windows-only!
}

#endif /* UTILS_H_HG */
