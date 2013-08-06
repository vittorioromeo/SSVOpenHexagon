// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_MENUGAME
#define HG_MENUGAME

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Data/StyleData.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"

namespace hg
{
	enum class States{ETUser, ETPass, ETEmail, ETLPNew, ETFriend, SLogging, SMain, SLPSelect, MWlcm, MOpts};

	class HexagonGame;

	class MenuGame
	{
		private:
			ssvms::MenuController menuController;

			HGAssets& assets;
			sf::Font* imagine{&assets.get<sf::Font>("imagine.ttf")};

			float w, h;
			std::string lrUser, lrPass, lrEmail;

			HexagonGame& hexagonGame;
			ssvs::GameState game;
			ssvs::GameWindow& window;
			sses::Manager manager;
			ssvs::Camera backgroundCamera{window, {{0, 0}, {Config::getSizeX() * Config::getZoomFactor(), Config::getSizeY() * Config::getZoomFactor()}}};
			ssvs::Camera overlayCamera{window, {{Config::getWidth() / 2.f, Config::getHeight() * Config::getZoomFactor() / 2.f}, {Config::getWidth() * Config::getZoomFactor(), Config::getHeight() * Config::getZoomFactor()}}};
			States state{States::MWlcm};
			ssvms::Menu optionsMenu, welcomeMenu;
			std::string scoresMessage;
			float exitTimer{0}, currentCreditsId{0};
			bool mustTakeScreenshot{false};
			std::string currentLeaderboard, currentPlayerScore, enteredStr, leaderboardString, friendsString;
			std::vector<char> enteredChars;
			std::vector<std::string> creditsIds{"creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png", "creditsBar2d.png", "creditsBar2d.png", "creditsBar2d.png"};

			sf::Sprite titleBar{assets.get<sf::Texture>("titleBar.png")}, creditsBar1{assets.get<sf::Texture>("creditsBar1.png")},
			creditsBar2{assets.get<sf::Texture>("creditsBar2.png")}, bottomBar{assets.get<sf::Texture>("bottomBar.png")};

			std::vector<std::string> levelDataIds;
			std::vector<float> difficultyMultipliers;
			int currentIndex{0}, packIndex{0}, profileIndex{0}, difficultyMultIndex{0};

			const LevelData* levelData;
			LevelStatus levelStatus;
			StyleData styleData;
			sf::Text txtVersion{"", *imagine, 40}, txtProf{"", *imagine, 21}, txtLName{"", *imagine, 65}, txtLDesc{"", *imagine, 32}, txtLAuth{"", *imagine, 20}, txtLMus{"", *imagine, 20}, txtFriends{"", *imagine, 21}, txtPacks{"", *imagine, 14};

			void refreshCamera();
			void refreshFPS();
			void initAssets();
			void initMenus();
			void initInput();
			void update(float mFrameTime);
			void draw();
			void drawLevelSelection();
			void drawProfileCreation();
			void drawProfileSelection();
			void drawOptions();
			void drawWelcome();
			void drawMenu(const ssvms::Menu& mMenu);
			inline void render(sf::Drawable& mDrawable) { window.draw(mDrawable); }
			inline sf::Text& renderTextImpl(const std::string& mStr, sf::Text& mText, ssvs::Vec2f mPosition)
			{
				if(mText.getString() != mStr) mText.setString(mStr);
				mText.setPosition(mPosition); render(mText); return mText;
			}
			inline sf::Text& renderTextImpl(const std::string& mStr, sf::Text& mText, ssvs::Vec2f mPosition, unsigned int mSize)
			{
				unsigned int originalSize{mText.getCharacterSize()};
				mText.setCharacterSize(mSize);
				renderTextImpl(mStr, mText, mPosition);
				mText.setCharacterSize(originalSize);
				return mText;
			}
			inline const sf::Color& getTextColor() const { return (state != States::SMain || Config::getBlackAndWhite()) ? sf::Color::White : styleData.getMainColor(); }
			inline sf::Text& renderText(const std::string& mStr, sf::Text& mText, ssvs::Vec2f mPos)													{ mText.setColor(getTextColor()); return renderTextImpl(mStr, mText, mPos); }
			inline sf::Text& renderText(const std::string& mStr, sf::Text& mText, ssvs::Vec2f mPos, unsigned int mSize)								{ mText.setColor(getTextColor()); return renderTextImpl(mStr, mText, mPos, mSize); }
			inline sf::Text& renderText(const std::string& mStr, sf::Text& mText, ssvs::Vec2f mPos, const sf::Color& mColor)						{ mText.setColor(mColor); return renderTextImpl(mStr, mText, mPos); }
			inline sf::Text& renderText(const std::string& mStr, sf::Text& mText, ssvs::Vec2f mPos, const sf::Color& mColor, unsigned int mSize)	{ mText.setColor(mColor); return renderTextImpl(mStr, mText, mPos, mSize); }
			void setIndex(int mIndex);
			void updateLeaderboard();
			void updateFriends();

			inline bool isEnteringText() { return state == States::ETUser || state == States::ETPass || state == States::ETEmail || state == States::ETLPNew || state == States::ETFriend; }
			inline ssvms::Menu* getCurrentMenu()
			{
				switch(state)
				{
					case States::MWlcm: return &welcomeMenu;
					case States::MOpts: return &optionsMenu;
					default: return nullptr;
				}
			}
			inline bool isInMenu() { return getCurrentMenu() != nullptr; }

		public:
			MenuGame(HGAssets& mAssets, HexagonGame& mHexagonGame, ssvs::GameWindow& mGameWindow);
			void init();
			inline ssvs::GameState& getGame() { return game; }
	};
}

#endif
