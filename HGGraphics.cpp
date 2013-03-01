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
	void HexagonGame::draw()
	{
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
		overlayCamera.apply(); drawText(); render(flashPolygon);
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
		if(status.hasDied) s << "press r to restart" << endl;

		vector<Vector2f> offsets{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

		Text timeText(s.str(), getFont("imagine.ttf"), 25 / getZoomFactor());
		timeText.setPosition(15, 3);
		timeText.setColor(getColorMain());

		for(auto& offset : offsets)
		{
			Text timeOffsetText(s.str(), getFont("imagine.ttf"), timeText.getCharacterSize());
			timeOffsetText.setPosition(timeText.getPosition() + offset);
			timeOffsetText.setColor(getColor(1));
			render(timeOffsetText);
		}

		render(timeText);

		if(messageTextPtr == nullptr) return;

		for(auto& offset : offsets)
		{
			Text textPtrOffset{messageTextPtr->getString(), getFont("imagine.ttf"), messageTextPtr->getCharacterSize()};
			textPtrOffset.setPosition(messageTextPtr->getPosition() + offset);
			textPtrOffset.setOrigin(textPtrOffset.getGlobalBounds().width / 2, 0);
			textPtrOffset.setColor(getColor(1));
			render(textPtrOffset);
		}

		messageTextPtr->setColor(getColorMain());
		render(*messageTextPtr);
	}
}
