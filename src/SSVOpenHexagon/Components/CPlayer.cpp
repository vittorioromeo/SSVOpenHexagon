// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Components/CPlayer.h"
#include "SSVOpenHexagon/Components/CWall.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Global/Common.h"

using namespace std;
using namespace sf;
using namespace sses;
using namespace ssvs;
using namespace hg::Utils;

namespace hg
{
	constexpr float baseThickness{5.f};

	CPlayer::CPlayer(HexagonGame& mHexagonGame, const Vec2f& mStartPos) : hexagonGame(mHexagonGame), startPos{mStartPos}, pos{startPos} { }

	void CPlayer::draw()
	{
		drawPivot();
		const auto& isDrawing3D(hexagonGame.getStatus().drawing3D);

		if(!isDrawing3D && deadEffectTimer.isRunning()) drawDeathEffect();

		Color colorMain{!dead || isDrawing3D ? hexagonGame.getColorMain() : getColorFromHue(hue / 255.f)};

		pLeft = getOrbitDeg(pos, angle - 100, size + 3);
		pRight = getOrbitDeg(pos, angle + 100, size + 3);

		vertices[0].position = getOrbitDeg(pos, angle, size);
		vertices[1].position = pLeft;
		vertices[2].position = pRight;

		if(!swapTimer.isRunning() && !isDrawing3D) colorMain = getColorFromHue((swapBlinkTimer.getCurrent() * 15) / 255.f);
		for(int i{0}; i < 3; ++i) vertices[i].color = colorMain;

		hexagonGame.render(vertices);
	}
	void CPlayer::drawPivot()
	{
		auto sides(hexagonGame.getSides());
		float div{360.f / sides * 0.5f}, radius{hexagonGame.getRadius() * 0.75f};
		Color colorMain{hexagonGame.getColorMain()}, colorB{hexagonGame.getColor(1)};
		if(Config::getBlackAndWhite()) colorB = Color::Black;
		ssvs::VertexVector<sf::PrimitiveType::Quads> vertices2(4);
		ssvs::VertexVector<sf::PrimitiveType::Triangles> vertices3(3);

		for(auto i(0u); i < sides; ++i)
		{
			float sAngle{div * 2.f * i};

			Vec2f p1{getOrbitDeg(startPos, sAngle - div, radius)};
			Vec2f p2{getOrbitDeg(startPos, sAngle + div, radius)};
			Vec2f p3{getOrbitDeg(startPos, sAngle + div, radius + baseThickness)};
			Vec2f p4{getOrbitDeg(startPos, sAngle - div, radius + baseThickness)};

			vertices2.emplace_back(p1, colorMain);
			vertices2.emplace_back(p2, colorMain);
			vertices2.emplace_back(p3, colorMain);
			vertices2.emplace_back(p4, colorMain);

			vertices3.emplace_back(p1, colorB);
			vertices3.emplace_back(p2, colorB);
			vertices3.emplace_back(startPos, colorB);
		}

		if(!hexagonGame.getStatus().drawing3D) hexagonGame.render(vertices3);
		hexagonGame.render(vertices2);
	}
	void CPlayer::drawDeathEffect()
	{
		float div{360.f / hexagonGame.getSides() * 0.5f}, radius{hue / 8}, thickness{hue / 20};
		Color colorMain{getColorFromHue((360 - hue) / 255.f)};
		ssvs::VertexVector<sf::PrimitiveType::Quads> verticesDeath(4);
		if(hue++ > 360) hue = 0;

		for(auto i(0u); i < hexagonGame.getSides(); ++i)
		{
			float sAngle{div * 2.f * i};

			Vec2f p1{getOrbitDeg(pos, sAngle - div, radius)};
			Vec2f p2{getOrbitDeg(pos, sAngle + div, radius)};
			Vec2f p3{getOrbitDeg(pos, sAngle + div, radius + thickness)};
			Vec2f p4{getOrbitDeg(pos, sAngle - div, radius + thickness)};

			verticesDeath.emplace_back(p1, colorMain);
			verticesDeath.emplace_back(p2, colorMain);
			verticesDeath.emplace_back(p3, colorMain);
			verticesDeath.emplace_back(p4, colorMain);
		}

		hexagonGame.render(verticesDeath);
	}

	void CPlayer::update(float mFT)
	{
		swapBlinkTimer.update(mFT);
		if(deadEffectTimer.update(mFT) && hexagonGame.getLevelStatus().tutorialMode) deadEffectTimer.stop();
		if(hexagonGame.getLevelStatus().swapEnabled) if(swapTimer.update(mFT)) swapTimer.stop();

		Vec2f lastPos{pos};
		float currentSpeed{speed}, lastAngle{angle}, radius{hexagonGame.getRadius()};
		int movement{hexagonGame.getInputMovement()};
		if(hexagonGame.getInputFocused()) currentSpeed = focusSpeed;

		angle += currentSpeed * movement * mFT;

		if(hexagonGame.getLevelStatus().swapEnabled && hexagonGame.getInputSwap() && !swapTimer.isRunning())
		{
			hexagonGame.getAssets().playSound("swap.ogg");
			swapTimer.restart(); angle += 180;
		}

		Vec2f tempPos{getOrbitDeg(startPos, angle, radius)};
		Vec2f pLeftCheck{getOrbitDeg(tempPos, angle - 90, 0.01f)};
		Vec2f pRightCheck{getOrbitDeg(tempPos, angle + 90, 0.01f)};

		for(const auto& wall : getManager().getEntities(HGGroup::Wall))
		{
			const auto& cwall(wall->getComponent<CWall>());
			if((movement == -1 && cwall.isOverlapping(pLeftCheck)) || (movement == 1 && cwall.isOverlapping(pRightCheck))) angle = lastAngle;
			if(cwall.isOverlapping(pos))
			{
				deadEffectTimer.restart();
				if(!Config::getInvincible()) dead = true;
				lastPos = getMovedTowards(lastPos, ssvs::zeroVec2f, 5 * hexagonGame.getSpeedMultDM());
				pos = lastPos; hexagonGame.death(); return;
			}
		}

		pos = getOrbitDeg(startPos, angle, radius);
	}
}
