// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Components/CPlayer.h"
#include "SSVOpenHexagon/Components/CWall.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace sses;
using namespace ssvs::Utils;
using namespace hg::Utils;

namespace hg
{
	CPlayer::CPlayer(Entity& mEntity, HexagonGame& mHexagonGame, Vector2f mStartPos) : Component{mEntity, "player"}, hexagonGame(mHexagonGame), startPos{mStartPos}, pos{startPos} { }

	void CPlayer::draw()
	{
		drawPivot();
		if(dead && !hexagonGame.getStatus().drawing3D) drawDeathEffect();

		Color colorMain{!dead || hexagonGame.getStatus().drawing3D ? hexagonGame.getColorMain() : getColorFromHue(hue / 255.0f)};

		pLeft = getOrbitFromDegrees(pos, angle - 100, size + 3);
		pRight = getOrbitFromDegrees(pos, angle + 100, size + 3);

		vertices[0].position = getOrbitFromDegrees(pos, angle, size);
		vertices[1].position = pLeft;
		vertices[2].position = pRight;

		for(int i{0}; i < 3; ++i) vertices[i].color = colorMain;

		hexagonGame.render(vertices);
	}
	void CPlayer::drawPivot()
	{
		float thickness{5}, div{360.f / hexagonGame.getSides()}, radius{hexagonGame.getRadius() * 0.75f};
		Color colorMain{hexagonGame.getColorMain()}, colorB{hexagonGame.getColor(1)};
		if(getBlackAndWhite()) colorB = Color::Black;
		VertexArray vertices2{PrimitiveType::Quads, 4}, vertices3{PrimitiveType::Triangles, 3};

		for(unsigned int i{0}; i < hexagonGame.getSides(); ++i)
		{
			float angle{div * i};

			Vector2f p1{getOrbitFromDegrees(startPos, angle - div * 0.5f, radius)};
			Vector2f p2{getOrbitFromDegrees(startPos, angle + div * 0.5f, radius)};
			Vector2f p3{getOrbitFromDegrees(startPos, angle + div * 0.5f, radius + thickness)};
			Vector2f p4{getOrbitFromDegrees(startPos, angle - div * 0.5f, radius + thickness)};

			vertices2.append({p1, colorMain});
			vertices2.append({p2, colorMain});
			vertices2.append({p3, colorMain});
			vertices2.append({p4, colorMain});

			vertices3.append({p1, colorB});
			vertices3.append({p2, colorB});
			vertices3.append({startPos, colorB});
		}

		if(!hexagonGame.getStatus().drawing3D) hexagonGame.render(vertices3);
		hexagonGame.render(vertices2);
	}
	void CPlayer::drawDeathEffect()
	{
		float div{360.f / hexagonGame.getSides()}, radius{hue / 8}, thickness{hue / 20};
		Color colorMain{getColorFromHue((360 - hue) / 255.0f)};
		VertexArray verticesDeath{PrimitiveType::Quads, 4};
		if(hue++ > 360) hue = 0;

		for(unsigned int i{0}; i < hexagonGame.getSides(); ++i)
		{
			float angle{div * i};

			Vector2f p1{getOrbitFromDegrees(pos, angle - div * 0.5f, radius)};
			Vector2f p2{getOrbitFromDegrees(pos, angle + div * 0.5f, radius)};
			Vector2f p3{getOrbitFromDegrees(pos, angle + div * 0.5f, radius + thickness)};
			Vector2f p4{getOrbitFromDegrees(pos, angle - div * 0.5f, radius + thickness)};

			verticesDeath.append({p1, colorMain});
			verticesDeath.append({p2, colorMain});
			verticesDeath.append({p3, colorMain});
			verticesDeath.append({p4, colorMain});
		}

		hexagonGame.render(verticesDeath);
	}

	void CPlayer::update(float mFrameTime)
	{
		Vector2f lastPos{pos};
		float currentSpeed{speed}, lastAngle{angle}, radius{hexagonGame.getRadius()};
		int movement{hexagonGame.getInputMovement()};
		if(hexagonGame.getInputFocused()) currentSpeed = focusSpeed;
		angle += currentSpeed * movement * mFrameTime;

		Vector2f tempPos{getOrbitFromDegrees(startPos, angle, radius)};
		Vector2f pLeftCheck{getOrbitFromDegrees(tempPos, angle - 90, 0.01f)};
		Vector2f pRightCheck{getOrbitFromDegrees(tempPos, angle + 90, 0.01f)};

		for(auto& wall : getManager().getComponents<CWall>("wall"))
		{
			if(movement == -1 && wall->isOverlapping(pLeftCheck)) angle = lastAngle;
			if(movement == 1 && wall->isOverlapping(pRightCheck)) angle = lastAngle;
			if(wall->isOverlapping(pos))
			{
				if(!getInvincible()) dead = true;
				lastPos = getMovedTowards(lastPos, {0, 0}, 5 * hexagonGame.getSpeedMultiplier());
				pos = lastPos; hexagonGame.death(); return;
			}
		}

		pos = getOrbitFromDegrees(startPos, angle, radius);
	}
}
