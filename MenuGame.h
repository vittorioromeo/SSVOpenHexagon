// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef MENUGAME_H
#define MENUGAME_H

#include <map>
#include <vector>
#include <json/json.h>
#include <json/reader.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/StyleData.h"
#include "Data/PackData.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Utils/Utils.h"

using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;

namespace hg
{
	enum StateType { LEVEL_SELECTION, PROFILE_CREATION, PROFILE_SELECTION };

	class HexagonGame;

	class MenuGame
	{
		private:
			GameState game;
			GameWindow& window;
			Manager manager;
			RenderTexture gameTexture;
			RenderTexture menuTexture;
			Sprite gameSprite;
			Sprite menuSprite;

			StateType state;

			float inputDelay{0};
			vector<string> levelDataIds;
			int currentIndex{0};
			int packIndex{0};

			string profileCreationName;
			int profileIndex{0};

			vector<float> difficultyMultipliers;
			int difficultyMultIndex{0};

			void recreateTextures();
			void update(float mFrameTime);
			void draw();
			void drawLevelSelection();
			void drawProfileCreation();
			void drawProfileSelection();
			void setIndex(int mIndex);
			void positionAndDrawCenteredText(Text& mText, Color mColor, float mElevation, bool mBold);

			LevelData levelData;
			StyleData styleData;
			Text title1{"open", getFont("imagine"), 80};
			Text title2{"hexagon", getFont("imagine"), 160};
			Text title3{toStr(getVersion()), getFont("imagine"), 25};
			Text title4{"clone of ""super hexagon"" by terry cavanagh\n              programmed by vittorio romeo\n                         music by bossfight", getFont("imagine"), 15};
			Text levelTime{"", getFont("imagine"), 50};
			Text cProfText{"", getFont("imagine"), 25};
			Text levelName{"", getFont("imagine"), 80};
			Text levelDesc{"", getFont("imagine"), 35};
			Text levelAuth{"", getFont("imagine"), 20};
			Text levelMusc{"", getFont("imagine"), 20};

		public:
			HexagonGame* hgPtr;

			MenuGame(GameWindow& mGameWindow);

			void init();

			void drawOnGameTexture(Drawable&);
			void drawOnMenuTexture(Drawable&);
			void drawOnWindow(Drawable&);

			GameState& getGame();
	};
}
#endif // MENUGAME_H
