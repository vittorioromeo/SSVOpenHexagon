#include "HexagonGame.h"
#include "CPlayer.h"
#include "CWall.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include "SSVStart.h"
#include "PatternManager.h"
#include <fstream>
#include "Config.h"
#include <SFML/Audio.hpp>
#include "Factory.h"

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	HexagonGame::HexagonGame() :
		window { (unsigned int)getWindowSizeX(), (unsigned int)getWindowSizeY(), getPixelMultiplier(), false }
	{
		sbDeath.loadFromFile("pldead00.wav");
		sDeath.setBuffer(sbDeath);

		pm = new PatternManager(this);

		font.loadFromFile("C:/Windows/Fonts/imagine.ttf"); // TODO: fix paths
		gameTexture.create(getSizeX(), getSizeY(), 32);
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}});
		gameTexture.setSmooth(true);
		gameSprite.setTexture(gameTexture.getTexture(), false);
		gameSprite.setOrigin(getSizeX()/ 2, getSizeY()/ 2);
		gameSprite.setPosition(getWindowSizeX() / 2, getWindowSizeY() / 2);
		window.renderWindow.setVerticalSyncEnabled(true);

		game.addUpdateFunc(	 [&](float frameTime) { update(frameTime); });
		game.addDrawFunc(	 [&](){ gameTexture.clear(Color::Black); }, -2);

		if(!getNoBackground())
			game.addDrawFunc([&](){ drawBackground(); }, 			   -1);

		game.addDrawFunc(	 [&](){ manager.draw(); }, 					0);
		game.addDrawFunc(	 [&](){ gameTexture.display(); }, 			1);
		game.addDrawFunc(	 [&](){ drawOnWindow(gameSprite); }, 		2);
		game.addDrawFunc(	 [&](){ drawDebugText(); }, 				3);

		initLevelSettings();
		newGame();

		window.setGame(&game);
		window.run();
	}
	HexagonGame::~HexagonGame() { delete pm; }

	void HexagonGame::newGame()
	{
		rotationDirection = rnd(0, 100) > 50 ? true : false;

		hasDied = false;
		mustRestart = false;
		currentTime = 0;
		incrementTime = 0;
		sides = 6;
		radius = minRadius;
		backType = (BackType)rnd(0, 3);

		manager.clear();
		createPlayer(manager, this, centerPos);

		timeline = Timeline{};

		speedMult 				= getLevelSettings().getSpeedMultiplier();
		rotationSpeed 			= getLevelSettings().getRotationSpeed();
		speedIncrement 			= getLevelSettings().getSpeedIncrement();
		rotationSpeedIncrement 	= getLevelSettings().getRotationSpeedIncrement();
		delayMult 				= getLevelSettings().getDelayMultiplier();
	}
	void HexagonGame::death()
	{
		hasDied = true;

		sDeath.play();
	}

	void HexagonGame::drawOnTexture(Drawable &mDrawable) { gameTexture.draw(mDrawable); }
	void HexagonGame::drawOnWindow(Drawable &mDrawable) { window.renderWindow.draw(mDrawable); }

	void HexagonGame::update(float mFrameTime)
	{
		if(!hasDied)
		{
			manager.update(mFrameTime);
			currentTime += mFrameTime / 60.0f;
			incrementTime += mFrameTime / 60.0f;

			updateIncrement();
			updateLevel(mFrameTime);
			updateRadius(mFrameTime);
			if(!getBlackAndWhite()) updateColor(mFrameTime); else color = Color::White;
		}
		else
		{
			rotationSpeed /= 1.001f;
		}

		updateDebugKeys(mFrameTime);
		if(!getNoRotation()) updateRotation(mFrameTime);

		if(mustRestart) newGame();
	}
	inline void HexagonGame::updateIncrement()
	{
		constexpr float maxIncrement = 15;

		if(incrementTime < maxIncrement) return;

		incrementTime = 0;
		incrementDifficulty();
	}
	inline void HexagonGame::updateLevel(float mFrameTime)
	{
		timeline.update(mFrameTime);

		if(timeline.isFinished())
		{
			timeline.clear();
			getLevelSettings().getRandomPattern()();
			timeline.reset();
		}
	}
	inline void HexagonGame::updateColor(float mFrameTime)
	{
		color = hue2color(hue / 255.0f);

		colorSwap += 1.f * mFrameTime;
		if (colorSwap > 100) colorSwap = 0;

		hue += hueIncrement * mFrameTime;

		if(backType == BackType::DARK)
		{
			hueIncrement = 1;
			if (hue > 254) hue = 0;
		}
		else if(backType == BackType::LIGHT)
		{
			if(hue < 149) hue = 150;
			if(hue <= 150) hueIncrement = 1;
			if(hue >= 255) hueIncrement = -1;
		}
		else if(backType == BackType::GRAY)
		{
			color = Color::White;

			if(hue < 149) hue = 150;
			if(hue <= 150) hueIncrement = 1;
			if(hue >= 255) hueIncrement = -1;
		}
	}
	inline void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = rotationSpeed * 10 * mFrameTime;
		if(rotationDirection) nextRotation *= -1;
		if(fastSpin > 0)
		{
			nextRotation += (smootherstep(0, 85, fastSpin) / 3.5f) * sign(nextRotation) * mFrameTime * 17.0f;
			fastSpin -= mFrameTime;
		}

		gameSprite.rotate(nextRotation);
	}
	inline void HexagonGame::updateRadius(float mFrameTime)
	{
		radiusTimer += 1.0f * mFrameTime;
		if(radiusTimer >= 25)
		{
			radiusTimer = 0;
			radius = rnd(82, 92);
		}

		if(radius > minRadius) radius -= 0.7f * mFrameTime;
	}
	inline void HexagonGame::updateDebugKeys(float mFrameTime)
	{
		if(Keyboard::isKeyPressed(Keyboard::R)) mustRestart = true;

		if(Keyboard::isKeyPressed(Keyboard::Num1)) { level = Level::EASY; newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num2)) { level = Level::NORMAL; newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num3)) { level = Level::HARD; newGame(); }
		if(Keyboard::isKeyPressed(Keyboard::Num4)) { level = Level::LUNATIC; newGame(); }

		if(Keyboard::isKeyPressed(Keyboard::Q)) sides++;
		if(Keyboard::isKeyPressed(Keyboard::W)) sides--;

		if(Keyboard::isKeyPressed(Keyboard::Z)) speedMult += 0.1f * mFrameTime;
		if(Keyboard::isKeyPressed(Keyboard::X))	speedMult -= 0.1f * mFrameTime;

		if(Keyboard::isKeyPressed(Keyboard::C)) rotationSpeed += 0.03f * mFrameTime;
		if(Keyboard::isKeyPressed(Keyboard::V))	rotationSpeed -= 0.03f * mFrameTime;
	}

	void HexagonGame::drawDebugText()
	{
		ostringstream s;
		s 	<< "time: " << toStr(currentTime).substr(0, 5) << endl
			<< "level: " << toStr(getLevelSettings().getName()) << endl;
		//	<< "sides: " << toStr(sides) << endl
		//	<< "speed multiplier: " << toStr(speedMult) << endl
		//	<< "rotation: " << toStr(rotationSpeed) << endl
		//	<< "hue: " << toStr(hue) << endl;

		Text t { s.str(), font, 20 };
		t.setPosition(10, 0);
		t.setColor(Color::White);
		window.renderWindow.draw(t);
	}
	void HexagonGame::drawBackground()
	{
		float div { 360.f / sides * 1.0001f };
		float distance { 1500 };

		Color darkColor1  { darkenColor(color, 2.7f) };
		Color darkColor2  { darkenColor(darkColor1, 1.5f) };
		Color lightColor1 { Color(235,235,235,255) };
		Color lightColor2 { Color(190 + color.r / 5, 190 + color.g / 5, 190 + color.b / 5, 255) };
		Color color1, color2;

		if(backType == BackType::DARK)
		{
			color1 = darkColor1;
			color2 = darkColor2;
		}
		else if(backType == BackType::LIGHT)
		{
			color1 = lightColor1;
			color2 = lightColor2;
		}
		else if (backType == BackType::GRAY)
		{
			color1 = darkenColor(Color(255,255,255,255), hue / 30.0f);
			color2 = darkenColor(Color(200,200,200,255), hue / 30.0f);
		}

		if(colorSwap > 50) swap(color1, color2);

		VertexArray vertices{PrimitiveType::Triangles, 3};

		for(int i {0}; i < sides; i++)
		{
			float angle { div * i };
			Color currentColor { color1 };

			if (i % 2 == 0)
			{
				currentColor = color2;
				if (i == sides - 1) currentColor = darkenColor(currentColor, 1.4f);
			}

			Vector2f p1 = orbit(centerPos, angle + div * 0.5f, distance);
			Vector2f p2 = orbit(centerPos, angle - div * 0.5f, distance);

			vertices.append(Vertex{centerPos, currentColor});
			vertices.append(Vertex{p1, currentColor});
			vertices.append(Vertex{p2, currentColor});
		}

		gameTexture.draw(vertices);
	}

    LevelSettings HexagonGame::loadLevelFromJson(Json::Value &mRoot, string mLevelObject)
	{
		string name { mRoot[mLevelObject]["name"].asString() };
		float s_m  	{ mRoot[mLevelObject]["speed_multiplier"].asFloat() };
		float s_i  	{ mRoot[mLevelObject]["speed_increment"].asFloat() };
		float rs  	{ mRoot[mLevelObject]["rotation_speed"].asFloat() };
		float rs_i 	{ mRoot[mLevelObject]["rotation_increment"].asFloat() };
		float d_m  	{ mRoot[mLevelObject]["delay_multiplier"].asFloat() };
		float fspin { mRoot[mLevelObject]["fast_spin"].asFloat() };
		int sid_s 	{ mRoot[mLevelObject]["sides_start"].asInt() };
		int sid_min	{ mRoot[mLevelObject]["sides_min"].asInt() };
		int sid_max	{ mRoot[mLevelObject]["sides_max"].asInt() };

		return LevelSettings{name, s_m, s_i, rs, rs_i, d_m, fspin, sid_s, sid_min, sid_max};
	}
	void HexagonGame::initLevelSettings()
	{
		Json::Value root;
		Json::Reader reader;
		ifstream test("levels.json", std::ifstream::binary);

		bool parsingSuccessful = reader.parse( test, root, false );
		if (!parsingSuccessful) cout << reader.getFormatedErrorMessages() << endl;

		LevelSettings easy 		{loadLevelFromJson(root, "0")};
		LevelSettings normal 	{loadLevelFromJson(root, "1")};
		LevelSettings hard 		{loadLevelFromJson(root, "2")};
		LevelSettings lunatic 	{loadLevelFromJson(root, "3")};

		easy.addPattern( [&]{ pm->alternateBarrageDiv(2); } 	, 1	);
		easy.addPattern( [&]{ pm->barrageSpin(3); } 			, 2	);
		easy.addPattern( [&]{ pm->mirrorSpin(8); } 				, 1	);
		easy.addPattern( [&]{ pm->evilRSpin(); } 				, 1	);
		easy.addPattern( [&]{ pm->inverseBarrage(1); }			, 2 );
		easy.addPattern( [&]{ pm->rWallStrip(); } 				, 3 );

		normal.addPattern( [&]{ pm->alternateBarrageDiv(2); } 	, 1	);
		normal.addPattern( [&]{ pm->barrageSpin(4); } 			, 2	);
		normal.addPattern( [&]{ pm->mirrorSpin(10); } 			, 1	);
		normal.addPattern( [&]{ pm->mirrorSpin(4); } 			, 2	);
		normal.addPattern( [&]{ pm->evilRSpin(); } 				, 2	);
		normal.addPattern( [&]{ pm->inverseBarrage(1); }		, 2 );
		normal.addPattern( [&]{ pm->rWallStrip(); } 			, 3 );

		hard.addPattern( [&]{ pm->alternateBarrageDiv(2); } 	, 1	);
		hard.addPattern( [&]{ pm->barrageSpin(4); } 			, 1	);
		hard.addPattern( [&]{ pm->mirrorSpin(12); } 			, 1	);
		hard.addPattern( [&]{ pm->mirrorSpin(4); } 				, 2	);
		hard.addPattern( [&]{ pm->evilRSpin(); } 				, 2	);
		hard.addPattern( [&]{ pm->inverseBarrage(1); }			, 3 );
		hard.addPattern( [&]{ pm->rWallStrip(); } 				, 4 );

		lunatic.addPattern( [&]{ pm->alternateBarrageDiv(2); } 	, 1	);
		lunatic.addPattern( [&]{ pm->barrageSpin(3); }			, 3 );
		lunatic.addPattern( [&]{ pm->barrageSpin(8, 0.5f); }	, 1 );
		lunatic.addPattern( [&]{ pm->inverseBarrage(1); }		, 3 );
		lunatic.addPattern( [&]{ pm->mirrorSpin(14); } 			, 1	);
		lunatic.addPattern( [&]{ pm->mirrorSpin(6); } 			, 2	);
		lunatic.addPattern( [&]{ pm->evilRSpin(1, 4); }			, 1	);
		lunatic.addPattern( [&]{ pm->evilRSpin(2, 3); } 		, 1	);
		lunatic.addPattern( [&]{ pm->rWallStrip(); } 			, 4	);

		levelMap.insert(make_pair(Level::EASY, easy));
		levelMap.insert(make_pair(Level::NORMAL, normal));
		levelMap.insert(make_pair(Level::HARD, hard));
		levelMap.insert(make_pair(Level::LUNATIC, lunatic));
	}
	void HexagonGame::incrementDifficulty()
	{
		speedMult += speedIncrement;
		rotationSpeed += rotationSpeedIncrement;
		rotationDirection = !rotationDirection;
		fastSpin = getLevelSettings().getFastSpin();

		if (sides != 6) sides = 6;
		else  			sides = rnd(getLevelSettings().getSidesMin(), getLevelSettings().getSidesMax() + 1);

		timeline.clear();
		timeline.reset();
		timeline.add(new Wait(40));
	}
	LevelSettings& HexagonGame::getLevelSettings() { return levelMap.find((int)level)->second; }

	Vector2f HexagonGame::getCenterPos() 	{ return centerPos; }
	int HexagonGame::getSides() 			{ return sides; }
	float HexagonGame::getRadius() 			{ return radius; }
	Color HexagonGame::getColor() 			{ return color; }
}

