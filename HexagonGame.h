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

#ifndef HEXAGONGAME_H_
#define HEXAGONGAME_H_

#include <map>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <json/json.h>
#include <json/reader.h>
#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/StyleData.h"

using namespace sf;
using namespace ssvs;
using namespace sses;

namespace hg
{
	class MenuGame;
	class PatternManager;

	class HexagonGame
	{
		friend class PatternManager;

		private:
			Game game;
			GameWindow& window;
			Manager manager;
			RenderTexture gameTexture;
			Sprite gameSprite;

			LevelData levelData;
			MusicData musicData;
			StyleData styleData;
			Music* musicPtr{nullptr};

			Timeline timeline;

			Timeline messagesTimeline;
			vector<Text*> messageTextPtrs;

			Vector2f centerPos{0,0};
			
			float currentTime{0};
			float incrementTime{0};

			float timeStop{0};
			bool randomSideChangesEnabled;
			bool incrementEnabled;
			float maxPulse{85};
			float minPulse{75};
			float pulseSpeed{1};
			float pulseSpeedBackwards{1};

			float radius{75};
			float radiusTimer{0};
			
			bool rotationDirection{true};			

			float fastSpin{0};

			bool hasDied{false};
			bool mustRestart{false};

			void update(float);
			inline void updateIncrement();
			inline void updateEvents(float);
			inline void updateLevel(float);
			inline void updateRotation(float);
			inline void updateRadius(float);
			inline void updateDebugKeys();

			void drawText();
			void drawBackground();

			void setLevelData(LevelData mLevelSettings, bool mMusicFirstPlay);

			void playLevelMusic();
			void stopLevelMusic();

			void incrementDifficulty();
			void randomSideChange();
			void checkAndSaveScore();
			void goToMenu();
			void changeLevel(string mId);
			void addMessage(string mMessage, float mDuration);
			void clearMessages();

		public:
			MenuGame* mgPtr;
			PatternManager* pm; // owned, opaque pointer

			HexagonGame(GameWindow& mGameWindow);
			~HexagonGame();

			void recreateTextures();
			void newGame(string mId, bool mFirstPlay);
			void death();
			void drawOnTexture(Drawable&);
			void drawOnWindow(Drawable&);

			Game& getGame();
			float getRadius();
			Color getColorMain();
			Color getColorB();
			bool isKeyPressed(Keyboard::Key mKey);

			float getSpeedMultiplier();
			float getDelayMultiplier();
			float getRotationSpeed();
			int getSides();
			void setSpeedMultiplier(float mSpeedMultiplier);
			void setDelayMultiplier(float mDelayMultiplier);
			void setRotationSpeed(float mRotationSpeed);
			void setSides(int mSides);
	};
}
#endif /* HEXAGONGAME_H_ */
