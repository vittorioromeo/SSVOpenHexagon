// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include <SSVStart/GameSystem/GameSystem.hpp>
#include <SSVStart/Camera/Camera.hpp>
#include <SSVStart/VertexVector/VertexVector.hpp>
#include <SSVStart/Utils/Vector2.hpp>

#include <SSVMenuSystem/SSVMenuSystem.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <cctype>

namespace hg
{

enum class States
{
    EpilepsyWarning,
    ETUser,
    ETPass,
    ETEmail,
    ETLPNew,
    ETFriend,
    SLogging,
    SMain,
    SLPSelect,
    MWlcm,
    MOpts
};

class HexagonGame;

class MenuGame
{
private:
    Steam::steam_manager& steamManager;
    Discord::discord_manager& discordManager;

    HGAssets& assets;
    sf::Font& imagine = assets.get<sf::Font>(
        "forcedsquare.ttf"); // G++ bug (cannot initialize with curly braces)

    float wheelProgress{0.f};

    float w, h;
    std::string lrUser, lrPass, lrEmail;

    HexagonGame& hexagonGame;
    ssvs::GameState game;
    ssvs::GameWindow& window;
    ssvs::Camera backgroundCamera{window,
        {ssvs::zeroVec2f, {Config::getSizeX() * Config::getZoomFactor(),
                              Config::getSizeY() * Config::getZoomFactor()}}};
    ssvs::Camera overlayCamera{
        window, {{Config::getWidth() / 2.f,
                     Config::getHeight() * Config::getZoomFactor() / 2.f},
                    {Config::getWidth() * Config::getZoomFactor(),
                        Config::getHeight() * Config::getZoomFactor()}}};

    // TODO: change this to MWlcm when leaderboards are enabled
    States state{States::EpilepsyWarning};

    ssvms::Menu optionsMenu;
    ssvms::Menu welcomeMenu;

    std::string scoresMessage;
    float exitTimer{0}, currentCreditsId{0};
    bool mustTakeScreenshot{false};
    std::string currentLeaderboard, enteredStr, leaderboardString,
        friendsString;
    std::vector<char> enteredChars;
    std::vector<std::string> creditsIds{
        "creditsBar2.png", "creditsBar2b.png", "creditsBar2c.png"};

    sf::Sprite titleBar{assets.get<sf::Texture>("titleBar.png")},
        creditsBar1{assets.get<sf::Texture>("creditsBar1.png")},
        creditsBar2{assets.get<sf::Texture>("creditsBar2.png")},
        bottomBar{assets.get<sf::Texture>("bottomBar.png")},
        epilepsyWarning{assets.get<sf::Texture>("epilepsyWarning.png")};

    std::vector<std::string> levelDataIds;
    std::vector<float> diffMults;
    int currentIndex{0}, packIdx{0}, profileIdx{0}, diffMultIdx{0};

    const LevelData* levelData;
    LevelStatus levelStatus;
    StyleData styleData;
    sf::Text txtVersion{"", imagine, 40}, txtProf{"", imagine, 21},
        txtLName{"", imagine, 65}, txtLDesc{"", imagine, 32},
        txtLAuth{"", imagine, 20}, txtLMus{"", imagine, 20},
        txtFriends{"", imagine, 21}, txtPacks{"", imagine, 14};

    void playLocally();

    void leftAction();
    void rightAction();
    void upAction();
    void downAction();
    void okAction();
    void createProfileAction();
    void selectProfileAction();
    void openOptionsAction();
    void selectPackAction();

    float touchDelay{0.f};

    void refreshCamera();
    void initAssets();
    void initMenus();
    void initInput();
    void update(ssvu::FT mFT);
    void draw();
    void drawLevelSelection();
    void drawEnteringText();
    void drawProfileSelection();
    void drawOptions();
    void drawWelcome();
    void drawMenu(const ssvms::Menu& mMenu);

    void render(sf::Drawable& mDrawable)
    {
        window.draw(mDrawable);
    }

    sf::Text& renderTextImpl(
        std::string mStr, sf::Text& mText, const sf::Vector2f& mPosition)
    {
        if(Config::getDrawTextOutlines() && state == States::SMain)
        {
            mText.setOutlineColor(styleData.getColor(0));
            mText.setOutlineThickness(1.f);
        }
        else
        {
            mText.setOutlineThickness(0.f);
        }

        Utils::uppercasify(mStr);

        if(mText.getString() != mStr)
        {
            mText.setString(mStr);
        }

        mText.setPosition(mPosition);
        render(mText);
        return mText;
    }

    sf::Text& renderTextImpl(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPosition, unsigned int mSize)
    {
        auto originalSize(mText.getCharacterSize());
        mText.setCharacterSize(mSize);
        renderTextImpl(mStr, mText, mPosition);
        mText.setCharacterSize(originalSize);
        return mText;
    }

    [[nodiscard]] sf::Color getTextColor() const
    {
        return (state != States::SMain || Config::getBlackAndWhite())
                   ? sf::Color::White
                   : styleData.getTextColor();
    }

    sf::Text& renderText(
        const std::string& mStr, sf::Text& mText, const sf::Vector2f& mPos)
    {
        mText.setFillColor(getTextColor());
        return renderTextImpl(mStr, mText, mPos);
    }

    sf::Text& renderText(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, unsigned int mSize)
    {
        mText.setFillColor(getTextColor());
        return renderTextImpl(mStr, mText, mPos, mSize);
    }

    sf::Text& renderText(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const sf::Color& mColor)
    {
        mText.setFillColor(mColor);
        return renderTextImpl(mStr, mText, mPos);
    }

    sf::Text& renderText(const std::string& mStr, sf::Text& mText,
        const sf::Vector2f& mPos, const sf::Color& mColor, unsigned int mSize)
    {
        mText.setFillColor(mColor);
        return renderTextImpl(mStr, mText, mPos, mSize);
    }

    void setIndex(int mIdx);
    void updateLeaderboard();
    void updateFriends();
    void initLua(Lua::LuaContext& mLua);

    bool isEnteringText()
    {
        return state == States::ETUser || state == States::ETPass ||
               state == States::ETEmail || state == States::ETLPNew ||
               state == States::ETFriend;
    }

    ssvms::Menu* getCurrentMenu() noexcept
    {
        switch(state)
        {
            case States::MWlcm: return &welcomeMenu;
            case States::MOpts: return &optionsMenu;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
            default: return nullptr;
#pragma GCC diagnostic pop
        }
    }

    bool isInMenu() noexcept
    {
        return getCurrentMenu() != nullptr;
    }

public:
    MenuGame(Steam::steam_manager& mSteamManager,
        Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
        HexagonGame& mHexagonGame, ssvs::GameWindow& mGameWindow);

    void init(bool mErrored);

    ssvs::GameState& getGame() noexcept
    {
        return game;
    }
};

} // namespace hg
