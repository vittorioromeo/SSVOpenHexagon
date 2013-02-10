#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <SFML/Audio.hpp>
#include <SSVStart.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Data/StyleData.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Global/Factory.h"
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
	float HexagonGame::getRadius() 									{ return radius; }
	Color HexagonGame::getColorMain() 								{ return getBlackAndWhite() ? Color::White : styleData.getMainColor(); }
	Color HexagonGame::getColor(int mIndex)							{ return styleData.getColors()[mIndex]; }

	int HexagonGame::getSides() 									{ return levelData.getSides(); }
	float HexagonGame::getWallSkewLeft() 							{ return levelData.getValueFloat("wall_skew_left"); }
	float HexagonGame::getWallSkewRight() 							{ return levelData.getValueFloat("wall_skew_right"); }
	float HexagonGame::getWallAngleLeft() 							{ return levelData.getValueFloat("wall_angle_left"); }
	float HexagonGame::getWallAngleRight() 							{ return levelData.getValueFloat("wall_angle_right"); }
	float HexagonGame::getSpeedMultiplier() 						{ return levelData.getSpeedMultiplier() * (pow(difficultyMult, 0.65f)); }
	float HexagonGame::getDelayMultiplier() 						{ return levelData.getDelayMultiplier() / (pow(difficultyMult, 0.10f)); }
	float HexagonGame::getRotationSpeed() 							{ return levelData.getRotationSpeed(); }
	float HexagonGame::get3DEffectMult() 							{ return levelData.getValueFloat("3d_effect_multiplier"); }

	void HexagonGame::setSpeedMultiplier(float mSpeedMultiplier) 	{ levelData.setSpeedMultiplier(mSpeedMultiplier); }
	void HexagonGame::setDelayMultiplier(float mDelayMultiplier)	{ levelData.setDelayMultiplier(mDelayMultiplier); }
	void HexagonGame::setRotationSpeed(float mRotationSpeed) 	 	{ levelData.setRotationSpeed(mRotationSpeed); }
	void HexagonGame::setSides(int mSides)
	{
		playSound("beep");
		if (mSides < 3) mSides = 3;
		levelData.setValueInt("sides", mSides);
	}

}
