/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Utils/Utils.h"

namespace hg
{
	CPlayer::CPlayer(HexagonGame *mHgPtr, Vector2f mStartPos) :
		Component{"player"}, hgPtr{mHgPtr}, startPos{mStartPos}, pos{startPos} { }

	void CPlayer::draw()
	{
		drawPivot();

		Color colorMain{hgPtr->getColorMain()};

		if(isDead) colorMain = getColorFromHue(hue / 255.0f);

		pLeft = getOrbit(pos, angle - 100, size + 3);
		pRight = getOrbit(pos, angle + 100, size + 3);

		vertices[0].position = getOrbit(pos, angle, size);
		vertices[1].position = pLeft;
		vertices[2].position = pRight;

		vertices[0].color = colorMain;
		vertices[1].color = colorMain;
		vertices[2].color = colorMain;

		hgPtr->drawOnTexture(vertices);
	}
	inline void CPlayer::drawPivot()
	{
		float thickness{5};

		Color colorMain{hgPtr->getColorMain()};
		Color colorB{hgPtr->getColor(1)};
		
		float div {360.f / hgPtr->getSides()};
		float radius {hgPtr->getRadius() * 0.75f};
		Vector2f pivotPos{startPos};

		if(isDead)
		{
			pivotPos = pos;			
			colorMain = getColorFromHue((360 - hue) / 255.0f);
			if(hue++ > 360) hue = 0;
			radius = hue / 8;
			thickness = hue / 20;
		}

		VertexArray vertices2 {PrimitiveType::Quads, 4};
		VertexArray vertices3 {PrimitiveType::Triangles, 3};

		for(int i{0}; i < hgPtr->getSides(); i++)
		{
			float angle{div * i};

			Vector2f p1{getOrbit(pivotPos, angle - div * 0.5f, radius)};
			Vector2f p2{getOrbit(pivotPos, angle + div * 0.5f, radius)};
			Vector2f p3{getOrbit(pivotPos, angle + div * 0.5f, radius + thickness)};
			Vector2f p4{getOrbit(pivotPos, angle - div * 0.5f, radius + thickness)};
			
			vertices2.append(Vertex{p1, colorMain});
			vertices2.append(Vertex{p2, colorMain});
			vertices2.append(Vertex{p3, colorMain});
			vertices2.append(Vertex{p4, colorMain});

			vertices3.append(Vertex{p1, colorB});
			vertices3.append(Vertex{p2, colorB});
			vertices3.append(Vertex{pivotPos, colorB});
		}
		
		if(!isDead) hgPtr->drawOnTexture(vertices3);
		hgPtr->drawOnTexture(vertices2);
	}

	void CPlayer::update(float mFrameTime)
	{
		Vector2f lastPos{pos};

		float currentSpeed{speed};
		float lastAngle{angle};
		int movement{0};

		// Keyboard controls
		vector<Keyboard::Key> leftKeys{Keyboard::Left, Keyboard::A};
		vector<Keyboard::Key> rightKeys{Keyboard::Right, Keyboard::D};
		if(hgPtr->isKeyPressed(Keyboard::LShift)) currentSpeed = focusSpeed;
		for (auto key : leftKeys) if(hgPtr->isKeyPressed(key)) movement = -1;
		for (auto key : rightKeys) if(hgPtr->isKeyPressed(key)) movement = 1;

		// Mousebutton controls
		if(hgPtr->isButtonPressed(Mouse::Button::Middle)) currentSpeed = focusSpeed;
		if(hgPtr->isButtonPressed(Mouse::Button::Left)) movement = -1;
		if(hgPtr->isButtonPressed(Mouse::Button::Right)) movement = 1;

		angle += currentSpeed * movement * mFrameTime;

		float radius{hgPtr->getRadius()};
		Vector2f tempPos{getOrbit(startPos, angle, radius)};
		Vector2f pLeftCheck{getOrbit(tempPos, angle - 90, 0.01f)};
		Vector2f pRightCheck{getOrbit(tempPos, angle + 90, 0.01f)};

		for (auto wall : getManager().getComponentPtrsCasted<CWall>("wall"))
		{
			if (movement == -1 && wall->isOverlapping(pLeftCheck)) angle = lastAngle;
			if (movement == 1 && wall->isOverlapping(pRightCheck)) angle = lastAngle;
			if (wall->isOverlapping(pos))
			{
				isDead = true;

				movePointTowardsCenter(lastPos, Vector2f(0,0), 5 * hgPtr->getSpeedMultiplier());

				pos = lastPos;
				hgPtr->death();
				return;
			}
		}

		pos = getOrbit(startPos, angle, radius);
	}
}
