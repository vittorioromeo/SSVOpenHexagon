// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <sstream>
#include <set>
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"

#include <cctype>

namespace hg::Utils
{

// TODO: move
template <sf::PrimitiveType TPrimitive>
struct FastVertexVector : public sf::Drawable
{
private:
    std::unique_ptr<sf::Vertex[]> _data{};
    std::size_t _size{};
    std::size_t _capacity{};

public:
    [[gnu::always_inline]] void reserve_more(const std::size_t n)
    {
        reserve(_size + n);
    }

    void reserve(const std::size_t n)
    {
        if(SSVU_LIKELY(_capacity >= n))
        {
            return;
        }

        auto new_data = std::make_unique<sf::Vertex[]>(n);
        std::memcpy(new_data.get(), _data.get(), sizeof(sf::Vertex) * _size);
        _data = std::move(new_data);

        _capacity = n;
    }

    [[gnu::always_inline]] void unsafe_emplace_other(
        const FastVertexVector& rhs) noexcept
    {
        SSVU_ASSERT(_size + rhs._size < _capacity);
        std::memcpy(_data.get() + _size, rhs._data.get(),
            sizeof(sf::Vertex) * rhs._size);
        _size += rhs._size;
    }

    [[gnu::always_inline]] void clear() noexcept
    {
        _size = 0;
    }

    [[gnu::always_inline, nodiscard]] std::size_t size() const noexcept
    {
        return _size;
    }

    template <typename... Ts>
    [[gnu::always_inline]] void unsafe_emplace_back(Ts&&... xs)
    {
        SSVU_ASSERT(_size < _capacity);
        new(&_data[_size++]) sf::Vertex{std::forward<Ts>(xs)...};
    }

    template <typename... Ts>
    [[gnu::always_inline]] void batch_unsafe_emplace_back(
        const sf::Color& color, Ts&&... positions)
    {
        SSVU_ASSERT(_size + sizeof...(positions) < _capacity);
        ((new(&_data[_size++]) sf::Vertex{positions, color}), ...);
    }

    void draw(sf::RenderTarget& mRenderTarget,
        sf::RenderStates mRenderStates) const override
    {
        mRenderTarget.draw(_data.get(), _size, TPrimitive, mRenderStates);
    }

    [[gnu::always_inline, nodiscard]] sf::Vertex& operator[](
        const std::size_t i) noexcept
    {
        SSVU_ASSERT(i < _size);
        return _data[i];
    }

    [[gnu::always_inline, nodiscard]] const sf::Vertex& operator[](
        const std::size_t i) const noexcept
    {
        SSVU_ASSERT(i < _size);
        return _data[i];
    }
};

inline void uppercasify(std::string& s)
{
    for(auto& c : s)
    {
        c = std::toupper(c);
    }
}

[[gnu::const]] inline float getSaturated(float mValue)
{
    return std::max(0.f, std::min(1.f, mValue));
}

[[gnu::const]] inline float getSmootherStep(float edge0, float edge1, float x)
{
    x = getSaturated((x - edge0) / (edge1 - edge0));
    return x * x * x * (x * (x * 6 - 15) + 10);
}

MusicData loadMusicFromJson(const ssvuj::Obj& mRoot);
ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot);

std::string getLocalValidator(const std::string& mId, float mDifficultyMult);

void shakeCamera(
    ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera);

std::set<std::string> getIncludedLuaFileNames(const std::string& mLuaScript);

void recursiveFillIncludedLuaFileNames(std::set<std::string>& mLuaScriptNames,
    const Path& mPackPath, const std::string& mLuaScript);

[[gnu::const]] sf::Color transformHue(const sf::Color& in, float H);

inline void runLuaFile(Lua::LuaContext& mLua, const std::string& mFileName)
{
    std::ifstream s{mFileName};

    try
    {
        mLua.executeCode(s);
    }
    catch(std::runtime_error& mError)
    {
        ssvu::lo("hg::Utils::runLuaFile") << "Fatal lua error"
                                          << "\n";
        ssvu::lo("hg::Utils::runLuaFile") << "Filename: " << mFileName << "\n";
        ssvu::lo("hg::Utils::runLuaFile") << "Error: " << mError.what() << "\n"
                                          << std::endl;
    }
}

template <typename T, typename... TArgs>
T runLuaFunction(
    Lua::LuaContext& mLua, const std::string& mName, const TArgs&... mArgs)
{
    try
    {
        return mLua.callLuaFunction<T>(mName, ssvu::mkTpl(mArgs...));
    }
    catch(std::runtime_error& mError)
    {
        std::cout << mName << "\n"
                  << "LUA runtime error: "
                  << "\n"
                  << ssvu::toStr(mError.what()) << "\n"
                  << std::endl;
    }

    return T();
}

template <typename T, typename... TArgs>
void runLuaFunctionIfExists(
    Lua::LuaContext& mLua, const std::string& mName, const TArgs&... mArgs)
{
    if(!mLua.doesVariableExist(mName))
    {
        return;
    }

    runLuaFunction<T>(mLua, mName, mArgs...);
}

} // namespace hg::Utils
