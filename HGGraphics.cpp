// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Global/Assets.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "Global/Config.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;
using namespace hg::Utils;
using namespace hg::UtilsJson;

namespace hg
{
	void HexagonGame::draw()
	{
		styleData.computeColors();

		window.clear(Color::Black);
		if(!getNoBackground()) { backgroundCamera.apply(); styleData.drawBackground(window.getRenderWindow(), {0, 0}, getSides()); }
		if(get3D())
		{
			status.drawing3D = true;
			for(unsigned int i{0}; i < depthCameras.size(); ++i)
			{
				status.overrideColor = getColorDarkened(styleData.get3DOverrideColor(), styleData.get3DDarkenMultiplier());
				status.overrideColor.a /= styleData.get3DAlphaMultiplier();
				status.overrideColor.a -= i * styleData.get3DAlphaFalloff();

				depthCameras[i].apply();
				manager.draw();
			}
			status.drawing3D = false;
		}
		backgroundCamera.apply(); manager.draw();
		overlayCamera.apply(); drawText();

		if(getFlash()) render(flashPolygon);
		if(mustTakeScreenshot) { window.getRenderWindow().capture().saveToFile("screenshot.png"); mustTakeScreenshot = false; }
	}

	void HexagonGame::render(Drawable &mDrawable) { window.draw(mDrawable); }

	void HexagonGame::initFlashEffect()
	{
		flashPolygon.clear();
		flashPolygon.append({{-100.f, -100.f}, Color{255, 255, 255, 0}});
		flashPolygon.append({{getWidth() + 100.f, -100.f}, Color{255, 255, 255, 0}});
		flashPolygon.append({{getWidth() + 100.f, getHeight() + 100.f}, Color{255, 255, 255, 0}});
		flashPolygon.append({{-100.f, getHeight() + 100.f}, Color{255, 255, 255, 0}});
	}

	void HexagonGame::drawText()
	{
		ostringstream s;
		s << "time: " << toStr(status.currentTime).substr(0, 5) << endl;
		if(getOfficial()) s << "official mode" << endl;
		if(getDebug()) s << "debug mode" << endl;
		if(status.scoreInvalid) s << "score invalidated (performance issues)" << endl;
		if(status.hasDied) s << "press r to restart" << endl;

		vector<Vector2f> offsets{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

		Color offsetColor{getColor(1)};
		if(getBlackAndWhite()) offsetColor = Color::Black;
		Text timeText(s.str(), getFont("imagine.ttf"), 25 / getZoomFactor());
		timeText.setPosition(15, 3);
		timeText.setColor(getColorMain());

		for(auto& offset : offsets)
		{
			Text timeOffsetText(s.str(), getFont("imagine.ttf"), timeText.getCharacterSize());
			timeOffsetText.setPosition(timeText.getPosition() + offset);
			timeOffsetText.setColor(offsetColor);
			render(timeOffsetText);
		}

		render(timeText);

		if(messageTextPtr == nullptr) return;

		for(auto& offset : offsets)
		{
			Text textPtrOffset{messageTextPtr->getString(), getFont("imagine.ttf"), messageTextPtr->getCharacterSize()};
			textPtrOffset.setPosition(messageTextPtr->getPosition() + offset);
			textPtrOffset.setOrigin(textPtrOffset.getGlobalBounds().width / 2, 0);
			textPtrOffset.setColor(offsetColor);
			render(textPtrOffset);
		}

		messageTextPtr->setColor(getColorMain());
		render(*messageTextPtr);
	}
}
