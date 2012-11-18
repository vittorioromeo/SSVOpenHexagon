#ifndef MENUGAME_H
#define MENUGAME_H

#include "SSVStart.h"
#include "SSVEntitySystem.h"
#include "LevelData.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <json/json.h>
#include <json/reader.h>
#include "MusicData.h"
#include <map>
#include "StyleData.h"
#include "Assets.h"

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	class HexagonGame;

	class MenuGame
	{
		private:
			Game game;
			GameWindow& window;
			Manager manager;
			RenderTexture gameTexture;
			RenderTexture menuTexture;
			Sprite gameSprite;
			Sprite menuSprite;

			float inputDelay{0};
			vector<string> levelDataIds;
			int currentIndex{0};

			void recreate();
			void update(float mFrameTime);
			void draw();
			void drawBackground();
			void drawText();
			void setIndex(int mIndex);

			LevelData levelData;
			StyleData styleData;
			Text title1{"open", getFont("imagine"), 80};
			Text title2{"hexagon", getFont("imagine"), 160};
			Text title3{getVersion(), getFont("imagine"), 15};
			Text levelTime{"", getFont("imagine"), 50};
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

			Game& getGame();
	};
}
#endif // MENUGAME_H

