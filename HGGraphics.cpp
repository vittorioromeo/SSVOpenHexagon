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

namespace hg
{
	void HexagonGame::drawOnTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void HexagonGame::drawOnWindow(Drawable &mDrawable) { window.draw(mDrawable); }

	void HexagonGame::drawText()
	{
		ostringstream s;
		s << "time: " << toStr(currentTime).substr(0, 5) << endl;
		if(hasDied) s << "press r to restart" << endl;

		vector<Vector2f> offsets{{-1,-1},{-1,1},{1,-1},{1,1}};

		Text timeText(s.str(), getFont("imagine"), 25 / getZoomFactor());
		timeText.setPosition(15, 3);
		timeText.setColor(getColorMain());

		for(auto offset : offsets)
		{
			Text timeOffsetText(s.str(), getFont("imagine"), timeText.getCharacterSize());
			timeOffsetText.setPosition(timeText.getPosition() + offset);
			timeOffsetText.setColor(getColor(1));
			drawOnWindow(timeOffsetText);
		}

		drawOnWindow(timeText);

		if(messageTextPtr == nullptr) return;

		for(auto offset : offsets)
		{
			Text textPtrOffset{messageTextPtr->getString(), getFont("imagine"), messageTextPtr->getCharacterSize()};
			textPtrOffset.setPosition(messageTextPtr->getPosition() + offset);
			textPtrOffset.setOrigin(textPtrOffset.getGlobalBounds().width / 2, 0);
			textPtrOffset.setColor(getColor(1));
			drawOnWindow(textPtrOffset);
		}

		messageTextPtr->setColor(getColorMain());
		drawOnWindow(*messageTextPtr);
	}

	void HexagonGame::recreateTextures()
	{
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(window.getWidth() / 2, window.getHeight() / 2);
	}	
}
