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
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;

namespace hg
{
	const float baseThickness{5};

	CPlayer::CPlayer(HexagonGame& mHexagonGame, Vec2f mStartPos) : hexagonGame(mHexagonGame), startPos{mStartPos}, pos{startPos} { }

	void CPlayer::draw()
	{
		drawPivot();
		if(!hexagonGame.getStatus().drawing3D && deadEffectTimer > 0) drawDeathEffect();

		Color colorMain{!dead || hexagonGame.getStatus().drawing3D ? hexagonGame.getColorMain() : getColorFromHue(hue / 255.0f)};

		pLeft = getOrbitFromDegrees(pos, angle - 100, size + 3);
		pRight = getOrbitFromDegrees(pos, angle + 100, size + 3);

		vertices[0].position = getOrbitFromDegrees(pos, angle, size);
		vertices[1].position = pLeft;
		vertices[2].position = pRight;

		if(swapTimer <= 0) colorMain = Utils::TransformH(colorMain, 90);
		for(int i{0}; i < 3; ++i) vertices[i].color = colorMain;

		hexagonGame.render(vertices);
	}
	void CPlayer::drawPivot()
	{
		unsigned int sides{hexagonGame.getSides()};
		float div{360.f / sides * 0.5f}, radius{hexagonGame.getRadius() * 0.75f};
		Color colorMain{hexagonGame.getColorMain()}, colorB{hexagonGame.getColor(1)};
		if(getBlackAndWhite()) colorB = Color::Black;
		VertexArray vertices2{PrimitiveType::Quads, 4}, vertices3{PrimitiveType::Triangles, 3};

		for(unsigned int i{0}; i < sides; ++i)
		{
			float angle{div / 0.5f * i};

			Vec2f p1{getOrbitFromDegrees(startPos, angle - div, radius)};
			Vec2f p2{getOrbitFromDegrees(startPos, angle + div, radius)};
			Vec2f p3{getOrbitFromDegrees(startPos, angle + div, radius + baseThickness)};
			Vec2f p4{getOrbitFromDegrees(startPos, angle - div, radius + baseThickness)};

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
		float div{360.f / hexagonGame.getSides() * 0.5f}, radius{hue / 8}, thickness{hue / 20};
		Color colorMain{getColorFromHue((360 - hue) / 255.0f)};
		VertexArray verticesDeath{PrimitiveType::Quads, 4};
		if(hue++ > 360) hue = 0;

		for(unsigned int i{0}; i < hexagonGame.getSides(); ++i)
		{
			float angle{div / 0.5f * i};

			Vec2f p1{getOrbitFromDegrees(pos, angle - div, radius)};
			Vec2f p2{getOrbitFromDegrees(pos, angle + div, radius)};
			Vec2f p3{getOrbitFromDegrees(pos, angle + div, radius + thickness)};
			Vec2f p4{getOrbitFromDegrees(pos, angle - div, radius + thickness)};

			verticesDeath.append({p1, colorMain});
			verticesDeath.append({p2, colorMain});
			verticesDeath.append({p3, colorMain});
			verticesDeath.append({p4, colorMain});
		}

		hexagonGame.render(verticesDeath);
	}

	void CPlayer::update(float mFrameTime)
	{
		if(hexagonGame.getStatus().swapEnabled && swapTimer > 0) swapTimer -= mFrameTime;
		if(hexagonGame.getStatus().tutorialMode && deadEffectTimer > 0) deadEffectTimer -= mFrameTime;

		Vec2f lastPos{pos};
		float currentSpeed{speed}, lastAngle{angle}, radius{hexagonGame.getRadius()};
		int movement{hexagonGame.getInputMovement()};
		if(hexagonGame.getInputFocused()) currentSpeed = focusSpeed;

		angle += currentSpeed * movement * mFrameTime;

		if(hexagonGame.getStatus().swapEnabled && hexagonGame.getInputSwap() && swapTimer <= 0)
		{
			playSound("swap.ogg");
			swapTimer = 40; angle += 180;
		}

		Vec2f tempPos{getOrbitFromDegrees(startPos, angle, radius)};
		Vec2f pLeftCheck{getOrbitFromDegrees(tempPos, angle - 90, 0.01f)};
		Vec2f pRightCheck{getOrbitFromDegrees(tempPos, angle + 90, 0.01f)};

		for(const auto& wall : getManager().getEntities("wall"))
		{
			const auto& cwall(wall->getComponent<CWall>());
			if((movement == -1 && cwall.isOverlapping(pLeftCheck)) || (movement == 1 && cwall.isOverlapping(pRightCheck))) angle = lastAngle;
			if(cwall.isOverlapping(pos))
			{
				deadEffectTimer = 80;
				if(!getInvincible()) dead = true;
				lastPos = getMovedTowards(lastPos, {0, 0}, 5 * hexagonGame.getSpeedMultiplier());
				pos = lastPos; hexagonGame.death(); return;
			}
		}

		pos = getOrbitFromDegrees(startPos, angle, radius);
	}
}
