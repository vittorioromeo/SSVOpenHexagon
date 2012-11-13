#ifndef HEXAGONGAME_H_
#define HEXAGONGAME_H_

#include "SSVStart.h"
#include "SSVEntitySystem.h"
#include "LevelSettings.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <json/json.h>
#include <json/reader.h>
#include "MusicData.h"
#include <map>

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	enum BackType { DARK, LIGHT, GRAY };

	class PatternManager;

	class HexagonGame
	{
		friend class PatternManager;

		private:
			Game game;
			GameWindow window;
			Manager manager;
			RenderTexture gameTexture;
			Sprite gameSprite;
			Font font;

			Vector2f centerPos{0,0};

			float radius{75};
			float minRadius{75};
			float radiusTimer{0};

			Color color{Color::Red};
			float colorSwap{0};
			BackType backType{BackType::GRAY};
			double hue{0};
			float hueIncrement{1.0f};
			bool rotationDirection{true};

			vector<LevelSettings> levels;
			LevelSettings* levelPtr{nullptr};

			map<string, MusicData> musicMap;
			Music* musicPtr{nullptr};

			PatternManager* pm; // owned
			Timeline timeline;

			float currentTime {0};
			float incrementTime {0};

			int sides{6};
			float speedMult{1};
			float delayMult{1};
			float rotationSpeed{0.1f};
			float fastSpin{0};

			bool hasDied{false};
			bool mustRestart{false};

			SoundBuffer sbDeath;
			Sound sDeath;

			void update(float);
			inline void updateIncrement();
			inline void updateLevel(float);
			inline void updateColor(float);
			inline void updateRotation(float);
			inline void updateRadius(float);
			inline void updateDebugKeys(float);

			void drawDebugText();
			void drawBackground();

			void initLevelSettings();
			void initMusicData();

			void playLevelMusic();
			void stopLevelMusic();

			void incrementDifficulty();
			LevelSettings& getLevelSettings();

		public:
			HexagonGame();
			~HexagonGame();

			void newGame();
			void death();
			void drawOnTexture(Drawable&);
			void drawOnWindow(Drawable&);

			Vector2f getCenterPos();
			float getRadius();
			int getSides();
			Color getColor();
	};
}
#endif /* HEXAGONGAME_H_ */
