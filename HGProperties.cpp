// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Global/Assets.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;

namespace hg
{
	GameState& HexagonGame::getGame()								{ return game; }
	float HexagonGame::getRadius() 									{ return status.radius; }
	Color HexagonGame::getColorMain()
	{
		if(getBlackAndWhite()) return Color::White;
		else if(status.drawing3D) return status.overrideColor;
		else return styleData.getMainColor();
	}
	Color HexagonGame::getColor(int mIndex)							{ return styleData.getColors()[mIndex]; }

	unsigned int HexagonGame::getSides() 							{ return levelData.getSides(); }
	float HexagonGame::getWallSkewLeft() 							{ return levelData.getValueFloat("wall_skew_left"); }
	float HexagonGame::getWallSkewRight() 							{ return levelData.getValueFloat("wall_skew_right"); }
	float HexagonGame::getWallAngleLeft() 							{ return levelData.getValueFloat("wall_angle_left"); }
	float HexagonGame::getWallAngleRight() 							{ return levelData.getValueFloat("wall_angle_right"); }
	float HexagonGame::getSpeedMultiplier() 						{ return levelData.getSpeedMultiplier() * (pow(difficultyMult, 0.65f)); }
	float HexagonGame::getDelayMultiplier() 						{ return levelData.getDelayMultiplier() / (pow(difficultyMult, 0.10f)); }
	float HexagonGame::getRotationSpeed() 							{ return levelData.getRotationSpeed(); }
	float HexagonGame::get3DEffectMult() 							{ return levelData.getValueFloat("3d_effect_multiplier"); }
	HexagonGameStatus& HexagonGame::getStatus()						{ return status; }

	void HexagonGame::setSpeedMultiplier(float mSpeedMultiplier) 	{ levelData.setSpeedMultiplier(mSpeedMultiplier); }
	void HexagonGame::setDelayMultiplier(float mDelayMultiplier)	{ levelData.setDelayMultiplier(mDelayMultiplier); }
	void HexagonGame::setRotationSpeed(float mRotationSpeed) 	 	{ levelData.setRotationSpeed(mRotationSpeed); }
	void HexagonGame::setSides(unsigned int mSides)
	{
		playSound("beep.ogg");
		if(mSides < 3) mSides = 3;
		levelData.setValueInt("sides", mSides);
	}

	bool HexagonGame::getInputFocused() { return inputFocused; }
	int HexagonGame::getInputMovement() { return inputMovement; }
}
