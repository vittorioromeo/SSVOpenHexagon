// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

namespace hg
{

enum class States
{
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
    HGAssets& assets;
    sf::Font& imagine = assets.get<sf::Font>(
        "imagine.ttf"); // G++ bug (cannot initialize with curly braces)

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
    States state{States::MWlcm};
    ssvms::Menu optionsMenu, welcomeMenu;
    std::string scoresMessage;
    float exitTimer{0}, currentCreditsId{0};
    bool mustTakeScreenshot{false};
    std::string currentLeaderboard, enteredStr, leaderboardString,
        friendsString;
    std::vector<char> enteredChars;
    std::vector<std::string> creditsIds{"creditsBar2.png", "creditsBar2b.png",
        "creditsBar2c.png", "creditsBar2d.png", "creditsBar2d.png",
        "creditsBar2d.png"};

    sf::Sprite titleBar{assets.get<sf::Texture>("titleBar.png")},
        creditsBar1{assets.get<sf::Texture>("creditsBar1.png")},
        creditsBar2{assets.get<sf::Texture>("creditsBar2.png")},
        bottomBar{assets.get<sf::Texture>("bottomBar.png")};

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

    void leftAction();
    void rightAction();
    void upAction();
    void downAction();
    void okAction();

    float touchDelay{0.f};

    void refreshCamera();
    void initAssets();
    void initMenus();
    void initInput();
    void update(FT mFT);
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
        const std::string& mStr, sf::Text& mText, const Vec2f& mPosition)
    {
        if(mText.getString() != mStr) mText.setString(mStr);
        mText.setPosition(mPosition);
        render(mText);
        return mText;
    }
    sf::Text& renderTextImpl(const std::string& mStr, sf::Text& mText,
        const Vec2f& mPosition, unsigned int mSize)
    {
        auto originalSize(mText.getCharacterSize());
        mText.setCharacterSize(mSize);
        renderTextImpl(mStr, mText, mPosition);
        mText.setCharacterSize(originalSize);
        return mText;
    }
    const sf::Color& getTextColor() const
    {
        return (state != States::SMain || Config::getBlackAndWhite())
                   ? sf::Color::White
                   : styleData.getMainColor();
    }
    sf::Text& renderText(
        const std::string& mStr, sf::Text& mText, const Vec2f& mPos)
    {
        mText.setFillColor(getTextColor());
        return renderTextImpl(mStr, mText, mPos);
    }
    sf::Text& renderText(const std::string& mStr, sf::Text& mText,
        const Vec2f& mPos, unsigned int mSize)
    {
        mText.setFillColor(getTextColor());
        return renderTextImpl(mStr, mText, mPos, mSize);
    }
    sf::Text& renderText(const std::string& mStr, sf::Text& mText,
        const Vec2f& mPos, const sf::Color& mColor)
    {
        mText.setFillColor(mColor);
        return renderTextImpl(mStr, mText, mPos);
    }
    sf::Text& renderText(const std::string& mStr, sf::Text& mText,
        const Vec2f& mPos, const sf::Color& mColor, unsigned int mSize)
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
    MenuGame(HGAssets& mAssets, HexagonGame& mHexagonGame,
        ssvs::GameWindow& mGameWindow);
    void init();
    ssvs::GameState& getGame() noexcept
    {
        return game;
    }
};

} // namespace hg
