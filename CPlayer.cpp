#include "CPlayer.h"
#include "Utils.h"
#include "CWall.h"
#include <iostream>

namespace hg
{
	CPlayer::CPlayer(HexagonGame *mHexagonGamePtr, Vector2f mStartPos) :
			Component{"player"}, hexagonGamePtr{mHexagonGamePtr}, startPos{mStartPos}, pos{startPos} { }

	void CPlayer::draw()
	{
		drawPivot();

		Color color = hexagonGamePtr->getColor();

		vertices[0].position = pTop;
		vertices[1].position = pLeft;
		vertices[2].position = pRight;

		vertices[0].color = color;
		vertices[1].color = color;
		vertices[2].color = color;

		hexagonGamePtr->drawOnTexture(vertices);
	}
	inline void CPlayer::drawPivot()
	{
		float div {360.f / hexagonGamePtr->getSides()};
		constexpr float thickness {4};
		Color color {hexagonGamePtr->getColor()};
		float radius {hexagonGamePtr->getRadius() * 0.75f};

		VertexArray vertices2 {PrimitiveType::Quads, 4};

		for(int i {0}; i < hexagonGamePtr->getSides(); i++)
		{
			float angle { div * i };

			Vector2f p1{orbit(startPos, angle - div * 0.5f, radius)};
			Vector2f p2{orbit(startPos, angle + div * 0.5f, radius)};
			Vector2f p3{orbit(startPos, angle + div * 0.5f, radius + thickness)};
			Vector2f p4{orbit(startPos, angle - div * 0.5f, radius + thickness)};
			
			vertices2.append(Vertex{p1, color});
			vertices2.append(Vertex{p2, color});
			vertices2.append(Vertex{p3, color});
			vertices2.append(Vertex{p4, color});			
		}

		hexagonGamePtr->drawOnTexture(vertices2);
	}

	void CPlayer::update(float mFrameTime)
	{
		pTop = orbit(pos, angle, size);
		pLeft = orbit(pos, angle - 100, size + 3);
		pRight = orbit(pos, angle + 100, size + 3);

		float currentSpeed { speed };
		float lastAngle { angle };
		int movement { 0 };

		if(Keyboard::isKeyPressed(Keyboard::LShift))
		{
			currentSpeed = focusSpeed;
			pLeft = orbit(pLeft, angle - 90, -3);
			pRight = orbit(pRight, angle + 90, -3);
		}
		if(Keyboard::isKeyPressed(Keyboard::Left)) movement = -1;
		if(Keyboard::isKeyPressed(Keyboard::Right)) movement = 1;

		angle += currentSpeed * mFrameTime * movement;

		Vector2f pLeftCheck { orbit(pos, angle - 90, 1) };
		Vector2f pRightCheck { orbit(pos, angle + 90, 1) };

		for (auto wall : getManager().getComponentPtrsByIdCasted<CWall>("wall"))
		{
			if (movement == -1 && pnpoly(wall->pointPtrs, pLeftCheck)) angle = lastAngle;
			if (movement == 1 && pnpoly(wall->pointPtrs, pRightCheck)) angle = lastAngle;
			if (pnpoly(wall->pointPtrs, pos))
			{
				hexagonGamePtr->death();
				return;
			}
		}

		float radius { hexagonGamePtr->getRadius() };
		pos = orbit(startPos, angle, radius);
	}
}
