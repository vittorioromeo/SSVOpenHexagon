// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/HGStatus.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/FPSWatcher.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

namespace hg::Impl
{

template <typename>
struct ArgExtractor;

template <typename R, typename F, typename... Args>
struct ArgExtractor<R (F::*)(Args...)>
{
    using Return = R;
    using Function = F;

    inline static constexpr std::size_t numArgs = sizeof...(Args);

    template <std::size_t I>
    using NthArg = std::tuple_element_t<I, std::tuple<Args...>>;
};

template <typename R, typename F, typename... Args>
struct ArgExtractor<R (F::*)(Args...) const>
{
    using Return = R;
    using Function = F;

    inline static constexpr std::size_t numArgs = sizeof...(Args);

    template <std::size_t I>
    using NthArg = std::tuple_element_t<I, std::tuple<Args...>>;
};

} // namespace hg::Impl

namespace hg
{

class MenuGame;

class HexagonGame
{
    friend MenuGame;

private:
    Steam::steam_manager& steamManager;
    Discord::discord_manager& discordManager;

    HGAssets& assets;
    const LevelData* levelData;

    ssvs::GameState game;
    ssvs::GameWindow& window;

public:
    CPlayer player;
    std::vector<CWall> walls;

private:
    ssvs::Camera backgroundCamera{window,
        {ssvs::zeroVec2f, {Config::getWidth() * Config::getZoomFactor(),
                              Config::getHeight() * Config::getZoomFactor()}}};

    ssvs::Camera overlayCamera{
        window, {{Config::getWidth() / 2.f, Config::getHeight() / 2.f},
                    sf::Vector2f(Config::getWidth(), Config::getHeight())}};

    ssvu::TimelineManager effectTimelineManager;

    const sf::Vector2f centerPos{ssvs::zeroVec2f};

    Lua::LuaContext lua;

    LevelStatus levelStatus;
    MusicData musicData;
    StyleData styleData;

    ssvu::Timeline timeline;
    ssvu::Timeline eventTimeline;
    ssvu::Timeline messageTimeline;

    sf::Text messageText{"", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(38.f / Config::getZoomFactor())};

    ssvs::VertexVector<sf::PrimitiveType::Quads> flashPolygon{4};

    bool firstPlay{true};
    bool restartFirstTime{true};
    bool inputFocused{false};
    bool inputSwap{false};
    bool mustTakeScreenshot{false};
    bool mustChangeSides{false};

    HexagonGameStatus status;
    std::string restartId;
    float difficultyMult{1};
    int inputImplLastMovement;
    int inputMovement{0};
    bool inputImplCW{false};
    bool inputImplCCW{false};
    bool inputImplBothCWCCW{false};
    std::ostringstream os;

    FPSWatcher fpsWatcher;
    sf::Text text{"", assets.get<sf::Font>("forcedsquare.ttf"),
        ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor())};

    const sf::Vector2f txt_pos{8, 8};

    // Color of the polygon in the center.
    CapColor capColor;

    struct CExprVec2f
    {
        float x;
        float y;
    };

    static constexpr std::array txt_offsets{CExprVec2f{-1, -1},
        CExprVec2f{-1, 1}, CExprVec2f{1, -1}, CExprVec2f{1, 1}};

    // LUA-related methods
    void initLua_Utils();
    void initLua_Messages();
    void initLua_MainTimeline();
    void initLua_EventTimeline();
    void initLua_LevelControl();
    void initLua_StyleControl();
    void initLua_WallCreation();
    void initLua_Steam();

    void initLua();
    void runLuaFile(const std::string& mFileName)
    {
        try
        {
            Utils::runLuaFile(lua, mFileName);
        }
        catch(...)
        {
            death();
        }
    }

    // Wall creation
    void createWall(int mSide, float mThickness, const SpeedData& mSpeed,
        const SpeedData& mCurve = SpeedData{}, float mHueMod = 0);

public:
    template <typename T, typename... TArgs>
    T runLuaFunction(const std::string& mName, const TArgs&... mArgs)
    {
        return Utils::runLuaFunction<T, TArgs...>(lua, mName, mArgs...);
    }

    template <typename T, typename... TArgs>
    void runLuaFunctionIfExists(const std::string& mName, const TArgs&... mArgs)
    {
        Utils::runLuaFunctionIfExists<T, TArgs...>(lua, mName, mArgs...);
    }

private:
    void initFlashEffect();

    // Update methods
    void update(FT mFT);
    void updateIncrement();
    void updateEvents(FT mFT);
    void updateLevel(FT mFT);
    void updatePulse(FT mFT);
    void updateBeatPulse(FT mFT);
    void updateRotation(FT mFT);
    void updateFlash(FT mFT);
    void update3D(FT mFT);
    void updateText();

    // Draw methods
    void draw();

    // Gameplay methods
    void incrementDifficulty();
    void sideChange(unsigned int mSideNumber);

    // Draw methods
    void drawText();

    // Data-related methods
    void setLevelData(const LevelData& mLevelData, bool mMusicFirstPlay);
    void playLevelMusic();
    void playLevelMusicAtTime(float mSeconds);
    void stopLevelMusic();

    // Message-related methods
    void addMessage(std::string mMessage, float mDuration, bool mSoundToggle);
    void clearMessages();

    // Level/menu loading/unloading/changing
    void checkAndSaveScore();
    void goToMenu(bool mSendScores = true);
    void changeLevel(const std::string& mId, bool mFirstTime);

    void invalidateScore();

    class LuaMetadata
    {
    private:
        struct FnEntry
        {
            std::string fnRet;
            std::string fnName;
            std::string fnArgs;
            std::string fnDocs;
        };

        std::vector<FnEntry> fnEntries;

    public:
        void addFnEntry(const std::string& fnRet, const std::string& fnName,
            const std::string& fnArgs, const std::string& fnDocs)
        {
            fnEntries.push_back(FnEntry{fnRet, fnName, fnArgs, fnDocs});
        }

        template <typename F>
        void forFnEntries(F&& f)
        {
            for(const auto& [ret, name, args, docs] : fnEntries)
            {
                f(ret, name, args, docs);
            }
        }
    };

    LuaMetadata luaMetadata;

    class LuaMetadataProxy
    {
    private:
        LuaMetadata& luaMetadata;
        std::string name;
        std::string (*erasedRet)(LuaMetadataProxy*);
        std::string (*erasedArgs)(LuaMetadataProxy*);
        std::string docs;
        std::vector<std::string> argNames;

        template <typename T>
        [[nodiscard]] static const char* typeToStr()
        {
            if constexpr(std::is_same_v<T, void>)
            {
                return "void";
            }
            else if constexpr(std::is_same_v<T, bool>)
            {
                return "bool";
            }
            else if constexpr(std::is_same_v<T, int>)
            {
                return "int";
            }
            else if constexpr(std::is_same_v<T, float>)
            {
                return "float";
            }
            else if constexpr(std::is_same_v<T, double>)
            {
                return "double";
            }
            else if constexpr(std::is_same_v<T, std::string>)
            {
                return "string";
            }
            else if constexpr(std::is_same_v<T, unsigned int>)
            {
                return "unsigned int";
            }
            else if constexpr(std::is_same_v<T, std::size_t>)
            {
                return "size_t";
            }
            else
            {
                throw std::runtime_error(
                    std::string{"Unknown type "} + typeid(T).name());

                return "unknown";
            }
        }

        template <typename F>
        [[nodiscard]] static std::string makeArgsString(LuaMetadataProxy* self)
        {
            using AE = hg::Impl::ArgExtractor<decltype(&F::operator())>;

            std::vector<std::string> types;

            const auto addTypeToStr = [&]<typename ArgT>() {
                types.emplace_back(typeToStr<ArgT>());
            };

            [&]<std::size_t... Is>(std::index_sequence<Is...>)
            {
                (addTypeToStr.template
                    operator()<typename AE::template NthArg<Is>>(),
                    ...);
            }
            (std::make_index_sequence<AE::numArgs>{});

            if(types.empty())
            {
                return "";
            }

            std::string res = types.at(0) + " " + self->argNames.at(0);

            if(types.size() == 1)
            {
                return res;
            }

            for(std::size_t i = 1; i < types.size(); ++i)
            {
                res += ", ";
                res += types.at(i);
                res += " ";
                res += self->argNames.at(i);
            }

            return res;
        }

        [[nodiscard]] std::string resolveArgNames(const std::string& docs)
        {
            std::size_t argNameSize = 0;
            for(const auto& argName : argNames)
            {
                argNameSize += argName.size() + 4;
            }

            std::string result;
            result.reserve(docs.size() + argNameSize);

            for(std::size_t i = 0; i < docs.size(); ++i)
            {
                if(docs[i] != '$')
                {
                    result += docs[i];
                    continue;
                }

                const std::size_t index = docs.at(i + 1) - '0';
                result += argNames.at(index);
                ++i;
            }

            return result;
        }

    public:
        template <typename F>
        explicit LuaMetadataProxy(
            F&&, LuaMetadata& mLuaMetadata, const std::string& mName)
            : luaMetadata{mLuaMetadata}, name{mName},
              erasedRet{[](LuaMetadataProxy*) -> std::string {
                  using AE = hg::Impl::ArgExtractor<decltype(
                      &std::decay_t<F>::operator())>;

                  return typeToStr<typename AE::Return>();
              }},
              erasedArgs{[](LuaMetadataProxy* self) {
                  return makeArgsString<std::decay_t<F>>(self);
              }}
        {
        }

        ~LuaMetadataProxy()
        {
            try
            {
                luaMetadata.addFnEntry((*erasedRet)(this), name,
                    (*erasedArgs)(this), resolveArgNames(docs));
            }
            catch(const std::exception& e)
            {
                ssvu::lo("LuaMetadataProxy")
                    << "Failed to generate documentation for " << name << ": "
                    << e.what() << '\n';
            }
            catch(...)
            {
                ssvu::lo("LuaMetadataProxy")
                    << "Failed to generate documentation for " << name << '\n';
            }
        }

        LuaMetadataProxy& arg(const std::string& mArgName)
        {
            argNames.emplace_back(mArgName);
            return *this;
        }

        LuaMetadataProxy& doc(const std::string& mDocs)
        {
            docs = mDocs;
            return *this;
        }
    };

    template <typename F>
    LuaMetadataProxy addLuaFn(const std::string& name, F&& f)
    {
        lua.writeVariable(name, std::forward<F>(f));
        return LuaMetadataProxy{f, luaMetadata, name};
    }

    void printLuaDocs()
    {
        luaMetadata.forFnEntries(
            [](const std::string& ret, const std::string& name,
                const std::string& args, const std::string& docs) {
                std::cout << "* **`" << ret << " " << name << "(" << args
                          << ")`**: " << docs << '\n';
            });
    }


public:
    Utils::FastVertexVector<sf::PrimitiveType::Quads> wallQuads;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> playerTris;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> capTris;
    Utils::FastVertexVector<sf::PrimitiveType::Quads> wallQuads3D;
    Utils::FastVertexVector<sf::PrimitiveType::Triangles> playerTris3D;

    MenuGame* mgPtr;

    HexagonGame(Steam::steam_manager& mSteamManager,
        Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
        ssvs::GameWindow& mGameWindow);

    // Gameplay methods
    void newGame(
        const std::string& mId, bool mFirstPlay, float mDifficultyMult);
    void death(bool mForce = false);

    // Other methods
    void executeEvents(ssvuj::Obj& mRoot, float mTime);

    // Graphics-related methods
    void render(sf::Drawable& mDrawable)
    {
        window.draw(mDrawable);
    }

    // Setters
    void setSides(unsigned int mSides);

    // Getters
    ssvs::GameState& getGame() noexcept
    {
        return game;
    }

    float getRadius() const noexcept
    {
        return status.radius;
    }

    const sf::Color& getColor(int mIdx) const noexcept
    {
        return styleData.getColor(mIdx);
    }

    float getSpeedMultDM() const noexcept
    {
        return levelStatus.speedMult * (std::pow(difficultyMult, 0.65f));
    }

    float getDelayMultDM() const noexcept
    {
        return levelStatus.delayMult / (std::pow(difficultyMult, 0.10f));
    }

    float getRotationSpeed() const noexcept
    {
        return levelStatus.rotationSpeed;
    }

    unsigned int getSides() const noexcept
    {
        return levelStatus.sides;
    }

    float getWallSkewLeft() const noexcept
    {
        return levelStatus.wallSkewLeft;
    }

    float getWallSkewRight() const noexcept
    {
        return levelStatus.wallSkewRight;
    }

    float getWallAngleLeft() const noexcept
    {
        return levelStatus.wallAngleLeft;
    }

    float getWallAngleRight() const noexcept
    {
        return levelStatus.wallAngleRight;
    }

    float get3DEffectMult() const noexcept
    {
        return levelStatus._3dEffectMultiplier;
    }

    HexagonGameStatus& getStatus()
    {
        return status;
    }

    LevelStatus& getLevelStatus()
    {
        return levelStatus;
    }

    HGAssets& getAssets()
    {
        return assets;
    }

    sf::Color getColorMain() const;

    float getMusicDMSyncFactor() const
    {
        return Config::getMusicSpeedDMSync() ? std::pow(difficultyMult, 0.12f)
                                             : 1.f;
    }

    // Input
    bool getInputFocused() const
    {
        return inputFocused;
    }

    bool getInputSwap() const;

    int getInputMovement() const
    {
        return inputMovement;
    }
};

} // namespace hg
