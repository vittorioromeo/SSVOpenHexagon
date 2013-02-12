// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace sses;
using namespace ssvs::Utils;

namespace hg
{
	CPlayer::CPlayer(HexagonGame& mHexagonGame, Vector2f mStartPos) : Component{"player"}, hexagonGame(mHexagonGame), startPos{mStartPos}, pos{startPos} { }

	void CPlayer::draw()
	{
		drawPivot();

		Color colorMain = !dead ? hexagonGame.getColorMain() : getColorFromHue(hue / 255.0f);

		pLeft = getOrbit(pos, angle - 100, size + 3);
		pRight = getOrbit(pos, angle + 100, size + 3);

		vertices[0].position = getOrbit(pos, angle, size);
		vertices[1].position = pLeft;
		vertices[2].position = pRight;

		for(int i{0}; i < 3; i++) vertices[i].color = colorMain;

		hexagonGame.drawOnWindow(vertices);
	}
	void CPlayer::drawPivot()
	{
		float thickness{5};

		Color colorMain{hexagonGame.getColorMain()};
		Color colorB{hexagonGame.getColor(1)};
		
		float div {360.f / hexagonGame.getSides()};
		float radius {hexagonGame.getRadius() * 0.75f};
		Vector2f pivotPos{startPos};

		if(dead)
		{
			pivotPos = pos;			
			colorMain = getColorFromHue((360 - hue) / 255.0f);
			if(hue++ > 360) hue = 0;
			radius = hue / 8;
			thickness = hue / 20;
		}

		VertexArray vertices2 {PrimitiveType::Quads, 4};
		VertexArray vertices3 {PrimitiveType::Triangles, 3};

		for(int i{0}; i < hexagonGame.getSides(); i++)
		{
			float angle{div * i};

			Vector2f p1{getOrbit(pivotPos, angle - div * 0.5f, radius)};
			Vector2f p2{getOrbit(pivotPos, angle + div * 0.5f, radius)};
			Vector2f p3{getOrbit(pivotPos, angle + div * 0.5f, radius + thickness)};
			Vector2f p4{getOrbit(pivotPos, angle - div * 0.5f, radius + thickness)};
			
			vertices2.append({p1, colorMain});
			vertices2.append({p2, colorMain});
			vertices2.append({p3, colorMain});
			vertices2.append({p4, colorMain});

			vertices3.append({p1, colorB});
			vertices3.append({p2, colorB});
			vertices3.append({pivotPos, colorB});
		}
		
		if(!dead) hexagonGame.drawOnWindow(vertices3);
		hexagonGame.drawOnWindow(vertices2);
	}

	void CPlayer::update(float mFrameTime)
	{
		Vector2f lastPos{pos};

		float currentSpeed{speed};
		float lastAngle{angle};
		int movement{0};

		// Keyboard controls
		if(hexagonGame.getInputFocused()) currentSpeed = focusSpeed;
		movement = hexagonGame.getInputMovement();

		angle += currentSpeed * movement * mFrameTime;

		float radius{hexagonGame.getRadius()};
		Vector2f tempPos{getOrbit(startPos, angle, radius)};
		Vector2f pLeftCheck{getOrbit(tempPos, angle - 90, 0.01f)};
		Vector2f pRightCheck{getOrbit(tempPos, angle + 90, 0.01f)};

		for (auto wall : getManager().getComponents<CWall>("wall"))
		{
			if (movement == -1 && wall->isOverlapping(pLeftCheck)) angle = lastAngle;
			if (movement == 1 && wall->isOverlapping(pRightCheck)) angle = lastAngle;
			if (wall->isOverlapping(pos))
			{
				if(!getInvincible()) dead = true;

				movePointTowardsCenter(lastPos, Vector2f(0,0), 5 * hexagonGame.getSpeedMultiplier());

				pos = lastPos;
				hexagonGame.death();
				return;
			}
		}

		pos = getOrbit(startPos, angle, radius);
	}
}
