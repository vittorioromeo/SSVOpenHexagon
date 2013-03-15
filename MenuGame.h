// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_MENUGAME
#define HG_MENUGAME

#include <vector>
#include <SFML/Graphics.hpp>
#include <SSVStart.h>
#include <SSVMenuSystem.h>
#include "Data/LevelData.h"
#include "Data/StyleData.h"
#include "Global/Config.h"

namespace hg
{
	enum class States { MAIN, PROFILE_NEW, PROFILES, OPTIONS };

	class HexagonGame;

	class MenuGame
	{
		private:
			HexagonGame& hexagonGame;
			ssvs::GameState game;
			ssvs::GameWindow& window;
			sses::Manager manager;
			ssvs::Camera backgroundCamera{window, {{0, 0}, {getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}}};
			ssvs::Camera overlayCamera{window, {{getWidth() / 2.f, getHeight() * getZoomFactor() / 2.f}, {getWidth() * getZoomFactor(), getHeight() * getZoomFactor()}}};
			States state{States::PROFILES};
			ssvms::Menu optionsMenu;
			std::string scoresMessage;
			float exitTimer{0};
			bool mustTakeScreenshot{false};
			std::string currentScores{""}, profileNewName{""};

			sf::Sprite titleBar{getAssetManager().getTexture("titleBar.png")};
			sf::Sprite creditsBar1{getAssetManager().getTexture("creditsBar1.png")};
			sf::Sprite creditsBar2{getAssetManager().getTexture("creditsBar2.png")};
			sf::Sprite bottomBar{getAssetManager().getTexture("bottomBar.png")};

			std::vector<std::string> levelDataIds;
			std::vector<float> difficultyMultipliers;
			int currentIndex{0}, packIndex{0}, profileIndex{0}, difficultyMultIndex{0};

			LevelData levelData;
			StyleData styleData;
			sf::Text versionText{"", getFont("imagine.ttf"), 40}, cProfText{"", getFont("imagine.ttf"), 21}, levelName{"", getFont("imagine.ttf"), 65},
				levelDesc{"", getFont("imagine.ttf"), 32}, levelAuth{"", getFont("imagine.ttf"), 20}, levelMusc{"", getFont("imagine.ttf"), 20};

			void refreshScores();
			std::string getLeaderboard();
			void update(float mFrameTime);
			void draw();
			void drawLevelSelection();
			void drawProfileCreation();
			void drawProfileSelection();
			void drawOptions();
			void setIndex(int mIndex);
			void renderText(const std::string& mString, sf::Text& mText, sf::Vector2f mPosition, unsigned int mSize = 0);

		public:
			MenuGame(HexagonGame& mHexagonGame, ssvs::GameWindow& mGameWindow);

			void init();
			void initAssets();
			void initOptionsMenu();
			void initInput();
			void render(sf::Drawable&);

			ssvs::GameState& getGame();
	};
}

#endif
