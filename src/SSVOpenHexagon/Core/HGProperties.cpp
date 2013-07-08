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
	GameState& HexagonGame::getGame()								{ return game; }
	float HexagonGame::getRadius() const							{ return status.radius; }
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
	Color HexagonGame::getColor(int mIndex) const					{ return styleData.getColors()[mIndex]; }

	unsigned int HexagonGame::getSides() const						{ return levelData.getSides(); }
	float HexagonGame::getWallSkewLeft() const						{ return levelData.getValueFloat("wall_skew_left"); }
	float HexagonGame::getWallSkewRight() const						{ return levelData.getValueFloat("wall_skew_right"); }
	float HexagonGame::getWallAngleLeft() const						{ return levelData.getValueFloat("wall_angle_left"); }
	float HexagonGame::getWallAngleRight() const					{ return levelData.getValueFloat("wall_angle_right"); }
	float HexagonGame::getSpeedMultiplier() const					{ return levelData.getSpeedMultiplier() * (pow(difficultyMult, 0.65f)); }
	float HexagonGame::getDelayMultiplier() const					{ return levelData.getDelayMultiplier() / (pow(difficultyMult, 0.10f)); }
	float HexagonGame::getRotationSpeed() const						{ return levelData.getRotationSpeed(); }
	float HexagonGame::get3DEffectMult() const						{ return levelData.getValueFloat("3d_effect_multiplier"); }
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

	bool HexagonGame::getInputFocused() const { return inputFocused; }
	int HexagonGame::getInputMovement() const { return inputMovement; }
	bool HexagonGame::getInputSwap() const { return inputSwap; }
}
