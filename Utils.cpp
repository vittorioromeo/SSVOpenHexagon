#include "Utils.h"
#include <stdio.h>
#include <math.h>

namespace hg
{
	Vector2f orbit(const Vector2f mParent, const float mDegrees, const float mRadius)
	{
		return Vector2f(mParent.x + cos(toRadians(mDegrees)) * mRadius, mParent.y + sin(toRadians(mDegrees)) * mRadius);
	}
	Vector2f normalize(const Vector2f mVector)
	{
		float length = std::sqrt((mVector.x * mVector.x) + (mVector.y * mVector.y));
		return Vector2f(mVector.x / length, mVector.y / length);
	}

	int rnd(int mStart, int mEnd)
	{
		return (rand() % (mEnd - mStart)) + mStart;
	}

	bool pnpoly(std::vector<Vector2f*> verts, Vector2f test)
	{
		int nvert = verts.size();
		int i, j, c = 0;
		for (i = 0, j = nvert-1; i < nvert; j = i++) {
			if ( ((verts[i]->y>test.y) != (verts[j]->y>test.y)) &&
					(test.x < (verts[j]->x-verts[i]->x) * (test.y-verts[i]->y) / (verts[j]->y-verts[i]->y) + verts[i]->x) )
				c = !c;
		}
		return c;
	}

	Color hue2color(double h)
	{
		double s = 1;
		double v = 1;

		double r, g, b;

		int i = floor(h * 6);
		double f = h * 6 - i;
		double p = v * (1 - s);
		double q = v * (1 - f * s);
		double t = v * (1 - (1 - f) * s);

		switch(i % 6)
		{
			case 0: r = v, g = t, b = p; break;
			case 1: r = q, g = v, b = p; break;
			case 2: r = p, g = v, b = t; break;
			case 3: r = p, g = q, b = v; break;
			case 4: r = t, g = p, b = v; break;
			case 5: r = v, g = p, b = q; break;
		}

		return Color(r * 255, g * 255, b * 255, 255);
	}
	Color darkenColor(Color mColor, float mMultiplier)
	{
		mColor.r /= mMultiplier;
		mColor.b /= mMultiplier;
		mColor.g /= mMultiplier;
		return mColor;
	}
}
