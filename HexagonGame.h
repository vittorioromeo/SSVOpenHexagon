#ifndef HEXAGONGAME_H_
#define HEXAGONGAME_H_

#include "SSVStart.h"
#include "SSVEntitySystem.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	class PatternManager;

	class HexagonGame
	{
		friend class PatternManager;

		private:
			Game game;
			GameWindow window { 1024, 768, 1, false };
			Manager manager;
			RenderTexture gameTexture;
			Sprite gameSprite;
			Font font;

			Vector2f centerPos { 1024 / 2, 768 / 2 };

			float radius { 75 };
			int sides { 6 };
			int freeSides { 1 };
			float speedMult { 1 };
			float thickness { 32 };
			double hue { 0 };
			float rotationSpeed { 0.1f };
			Color color { Color::Red };

			PatternManager* pm; // owned
			Timeline timeline;

			float currentTime { 0 };

			Entity* createPlayer();
			void update(float);
			inline void updateLevel(float);
			inline void updateColor(float);
			inline void updateDebugKeys(float);
			void drawDebugText();
			void drawBackground();

		public:
			void newGame();
			void drawOnTexture(Drawable&);
			void drawOnWindow(Drawable&);

			Vector2f getCenterPos();
			float getRadius();
			int getSides();
			Color getColor();

			HexagonGame();
			~HexagonGame();
	};
}
#endif /* HEXAGONGAME_H_ */
