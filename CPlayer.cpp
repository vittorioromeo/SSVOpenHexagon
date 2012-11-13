#include "CPlayer.h"
#include "Utils.h"
#include "CWall.h"
#include <iostream>

namespace hg
{
	CPlayer::CPlayer(HexagonGame *mHgPtr, Vector2f mStartPos) :
		Component{"player"}, hgPtr{mHgPtr}, startPos{mStartPos}, pos{startPos} { }

	void CPlayer::draw()
	{
		drawPivot();

		Color color{hgPtr->getColor()};

		if(isDead) color = Color::Red;

		pLeft = orbit(pos, angle - 100, size + 3);
		pRight = orbit(pos, angle + 100, size + 3);

		vertices[0].position = orbit(pos, angle, size);
		vertices[1].position = pLeft;
		vertices[2].position = pRight;

		vertices[0].color = color;
		vertices[1].color = color;
		vertices[2].color = color;

		hgPtr->drawOnTexture(vertices);
	}
	inline void CPlayer::drawPivot()
	{
		constexpr float thickness {4};

		Color color {hgPtr->getColor()};
		float div {360.f / hgPtr->getSides()};
		float radius {hgPtr->getRadius() * 0.75f};
		Vector2f pivotPos{startPos};

		if(isDead)
		{
			pivotPos = pos;
			radius = size * 2;
			color = Color::White;
		}

		VertexArray vertices2 {PrimitiveType::Quads, 4};

		for(int i {0}; i < hgPtr->getSides(); i++)
		{
			float angle { div * i };

			Vector2f p1{orbit(pivotPos, angle - div * 0.5f, radius)};
			Vector2f p2{orbit(pivotPos, angle + div * 0.5f, radius)};
			Vector2f p3{orbit(pivotPos, angle + div * 0.5f, radius + thickness)};
			Vector2f p4{orbit(pivotPos, angle - div * 0.5f, radius + thickness)};
			
			vertices2.append(Vertex{p1, color});
			vertices2.append(Vertex{p2, color});
			vertices2.append(Vertex{p3, color});
			vertices2.append(Vertex{p4, color});			
		}

		hgPtr->drawOnTexture(vertices2);
	}

	void CPlayer::update(float mFrameTime)
	{
		float currentSpeed{speed};
		float lastAngle{angle};
		int movement{0};

		if(Keyboard::isKeyPressed(Keyboard::LShift)) currentSpeed = focusSpeed;
		if(Keyboard::isKeyPressed(Keyboard::Left)) movement = -1;
		if(Keyboard::isKeyPressed(Keyboard::Right)) movement = 1;

		angle += currentSpeed * movement * mFrameTime;

		float radius{hgPtr->getRadius()};
		Vector2f tempPos{orbit(startPos, angle, radius)};
		Vector2f pLeftCheck{orbit(tempPos, angle - 90, 0.01f)};
		Vector2f pRightCheck{orbit(tempPos, angle + 90, 0.01f)};

		for (auto wall : getManager().getComponentPtrsByIdCasted<CWall>("wall"))
		{
			if (movement == -1 && wall->isOverlapping(pLeftCheck)) angle = lastAngle;
			if (movement == 1 && wall->isOverlapping(pRightCheck)) angle = lastAngle;
			if (wall->isOverlapping(pos))
			{
				isDead = true;
				hgPtr->death();
			}
		}

		pos = orbit(startPos, angle, radius);
	}
}
