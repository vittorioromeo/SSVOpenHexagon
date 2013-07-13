// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_MENUGAME
#define HG_MENUGAME

#include <vector>
#include <SFML/Graphics.hpp>
#include <SSVStart/SSVStart.h>
#include <SSVMenuSystem/SSVMenuSystem.h>
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Data/StyleData.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"

namespace hg
{
	enum class States { MAIN, PROFILE_NEW, PROFILES, OPTIONS, ADD_FRIEND };

	class HexagonGame;

	class MenuGame
	{
		private:
			float fw, fh, fmin, w, h;

			HexagonGame& hexagonGame;
			ssvs::GameState game;
			ssvs::GameWindow& window;
			sses::Manager manager;
			ssvs::Camera backgroundCamera{window, {{0, 0}, {getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}}};
			ssvs::Camera overlayCamera{window, {{getWidth() / 2.f, getHeight() * getZoomFactor() / 2.f}, {getWidth() * getZoomFactor(), getHeight() * getZoomFactor()}}};
			States state{States::PROFILES};
			ssvms::Menu optionsMenu;
			std::string scoresMessage;
			float exitTimer{0}, currentCreditsId{0};
			bool mustTakeScreenshot{false};
			std::string currentLeaderboard, currentPlayerScore, enteredString, leaderboardString, friendsString;
			std::vector<char> enteredChars;
			std::vector<std::string> friendsScores;
			std::vector<std::string> creditsIds{"creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png", "creditsBar2d.png", "creditsBar2d.png", "creditsBar2d.png"};

			sf::Sprite titleBar{getAssetManager().get<sf::Texture>("titleBar.png")}, creditsBar1{getAssetManager().get<sf::Texture>("creditsBar1.png")},
			creditsBar2{getAssetManager().get<sf::Texture>("creditsBar2.png")}, bottomBar{getAssetManager().get<sf::Texture>("bottomBar.png")};

			std::vector<std::string> levelDataIds;
			std::vector<float> difficultyMultipliers;
			int currentIndex{0}, packIndex{0}, profileIndex{0}, difficultyMultIndex{0};

			bool wasOverloaded{false};

			LevelData levelData;
			StyleData styleData;
			sf::Text versionText{"", getFont("imagine.ttf"), 40}, cProfText{"", getFont("imagine.ttf"), 21}, levelName{"", getFont("imagine.ttf"), 65},
				levelDesc{"", getFont("imagine.ttf"), 32}, levelAuth{"", getFont("imagine.ttf"), 20}, levelMusc{"", getFont("imagine.ttf"), 20},
				friendsText{"", getFont("imagine.ttf"), 21}, packsText{"", getFont("imagine.ttf"), 14};

			void refreshCamera();
			void initAssets();
			void initOptionsMenu();
			void initInput();
			void update(float mFrameTime);
			void draw();
			void drawLevelSelection();
			void drawProfileCreation();
			void drawProfileSelection();
			void drawOptions();
			void render(sf::Drawable&);
			sf::Text& renderText(const std::string& mString, sf::Text& mText, ssvs::Vec2f mPosition, unsigned int mSize = 0);
			void setIndex(int mIndex);
			void refreshScores();
			void updateLeaderboard();
			void updateFriends();

		public:
			MenuGame(HexagonGame& mHexagonGame, ssvs::GameWindow& mGameWindow);
			void init();
			ssvs::GameState& getGame();
	};
}

#endif
