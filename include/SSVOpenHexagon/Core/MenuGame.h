// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_MENUGAME
#define HG_MENUGAME

#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>
#include <SSVStart/SSVStart.h>
#include <SSVMenuSystem/SSVMenuSystem.h>
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Data/StyleData.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"

namespace hg
{
	enum class States{Welcome, LRUser, LRPass, LREmail, Logging, Main, LocalProfileNew, LocalProfileSelect, Options, FriendAdd};

	class HexagonGame;

	class MenuGame
	{
		private:
			using Predicate = std::function<bool()>;
			std::vector<std::pair<ssvms::ItemBase&, Predicate>> menuController;

			HGAssets& assets;

			float fw, fh, fmin, w, h;
			std::string lrUser, lrPass;

			HexagonGame& hexagonGame;
			ssvs::GameState game;
			ssvs::GameWindow& window;
			sses::Manager manager;
			ssvs::Camera backgroundCamera{window, {{0, 0}, {getSizeX() * getZoomFactor(), getSizeY() * getZoomFactor()}}};
			ssvs::Camera overlayCamera{window, {{getWidth() / 2.f, getHeight() * getZoomFactor() / 2.f}, {getWidth() * getZoomFactor(), getHeight() * getZoomFactor()}}};
			States state{States::Welcome};
			ssvms::Menu optionsMenu, welcomeMenu;
			std::string scoresMessage;
			float exitTimer{0}, currentCreditsId{0};
			bool mustTakeScreenshot{false};
			std::string currentLeaderboard, currentPlayerScore, enteredString, leaderboardString, friendsString;
			std::vector<char> enteredChars;
			std::vector<std::string> creditsIds{"creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png", "creditsBar2d.png", "creditsBar2d.png", "creditsBar2d.png"};

			sf::Sprite titleBar{assets().get<sf::Texture>("titleBar.png")}, creditsBar1{assets().get<sf::Texture>("creditsBar1.png")},
			creditsBar2{assets().get<sf::Texture>("creditsBar2.png")}, bottomBar{assets().get<sf::Texture>("bottomBar.png")};

			std::vector<std::string> levelDataIds;
			std::vector<float> difficultyMultipliers;
			int currentIndex{0}, packIndex{0}, profileIndex{0}, difficultyMultIndex{0};

			bool wasOverloaded{false};

			const LevelData* levelData;
			LevelStatus levelStatus;
			StyleData styleData;
			sf::Text versionText{"", assets().get<sf::Font>("imagine.ttf"), 40}, cProfText{"", assets().get<sf::Font>("imagine.ttf"), 21}, levelName{"", assets().get<sf::Font>("imagine.ttf"), 65},
				levelDesc{"", assets().get<sf::Font>("imagine.ttf"), 32}, levelAuth{"", assets().get<sf::Font>("imagine.ttf"), 20}, levelMusc{"", assets().get<sf::Font>("imagine.ttf"), 20},
				friendsText{"", assets().get<sf::Font>("imagine.ttf"), 21}, packsText{"", assets().get<sf::Font>("imagine.ttf"), 14};

			void refreshCamera();
			void initAssets();
			void initWelcomeMenu();
			void initOptionsMenu();
			void initInput();
			void update(float mFrameTime);
			void draw();
			void drawLevelSelection();
			void drawProfileCreation();
			void drawProfileSelection();
			void drawOptions();
			void drawWelcome();
			void render(sf::Drawable&);
			sf::Text& renderText(const std::string& mString, sf::Text& mText, ssvs::Vec2f mPosition, unsigned int mSize = 0);
			sf::Text& renderText(const std::string& mString, sf::Text& mText, ssvs::Vec2f mPosition, const sf::Color& mColor, unsigned int mSize = 0);
			void setIndex(int mIndex);
			void refreshScores();
			void updateLeaderboard();
			void updateFriends();

			bool isEnteringText() { return state == States::LRUser || state == States::LRPass|| state == States::LREmail || state == States::LocalProfileNew || state == States::FriendAdd; }

		public:
			MenuGame(HGAssets& mAssets, HexagonGame& mHexagonGame, ssvs::GameWindow& mGameWindow);
			void init();
			ssvs::GameState& getGame();
	};
}

#endif
