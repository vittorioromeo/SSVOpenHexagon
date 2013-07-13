// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"

using namespace std;
using namespace sf;
using namespace ssvs;

namespace hg
{
	Color HexagonGame::getColorMain() const
	{
		if(getBlackAndWhite())
		{
			if(status.drawing3D) return Color{255, 255, 255, status.overrideColor.a};
			return Color(255, 255, 255, styleData.getMainColor().a);
		}
		else if(status.drawing3D) return status.overrideColor;
		else return styleData.getMainColor();
	}
	void HexagonGame::setSides(unsigned int mSides)
	{
		playSound("beep.ogg");
		if(mSides < 3) mSides = 3;
		levelData.setValueInt("sides", mSides);
	}
}
