// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include <vector>
#include <cstdint>
#include <cassert>
#include <cstddef>

namespace hg
{

class CPlayer;

class CCustomWallManager
{
    // TODO: consider using a sparse integer set

    std::vector<CCustomWall> _customWalls;
    std::vector<CCustomWallHandle> _freeHandles;
    std::vector<bool> _handleAvailable;
    CCustomWallHandle _nextFreeHandle{0};
    std::size_t _count{0};

    [[nodiscard]] bool isValidHandle(const CCustomWallHandle h) const noexcept;

    [[nodiscard]] bool checkValidHandle(
        const CCustomWallHandle h, const char* msg);

public:
    [[nodiscard]] CCustomWallHandle create();

    void destroy(const CCustomWallHandle cwHandle);

    void setVertexPos(const CCustomWallHandle cwHandle, const int vertexIndex,
        const sf::Vector2f& pos);

    void setCanCollide(const CCustomWallHandle cwHandle, const bool collide);

    void setDeadly(const CCustomWallHandle cwHandle, const bool deadly);

    void setKillingSide(
        const CCustomWallHandle cwHandle, const std::uint8_t killingSide);

    void setVertexColor(const CCustomWallHandle cwHandle, const int vertexIndex,
        const sf::Color& color);

    [[nodiscard]] sf::Vector2f getVertexPos(
        const CCustomWallHandle cwHandle, const int vertexIndex);

    [[nodiscard]] bool getCanCollide(const CCustomWallHandle cwHandle);

    [[nodiscard]] bool getDeadly(const CCustomWallHandle cwHandle);

    [[nodiscard]] std::uint8_t getKillingSide(const CCustomWallHandle cwHandle);

    [[nodiscard]] bool isOverlappingPlayer(const CCustomWallHandle cwHandle);

    void clear();
    void draw(Utils::FastVertexVectorQuads& wallQuads);

    [[nodiscard]] bool handleCollision(
        const int movement, const float radius, CPlayer& mPlayer, ssvu::FT mFT);

    template <typename F>
    void forCustomWalls(F&& f)
    {
        for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
        {
            if(!_handleAvailable[h])
            {
                f(_customWalls[h]);
            }
        }
    }

    template <typename F>
    [[nodiscard]] bool anyCustomWall(F&& f)
    {
        for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
        {
            if(!_handleAvailable[h])
            {
                if(f(_customWalls[h]))
                {
                    return true;
                }
            }
        }

        return false;
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return _count;
    }

    [[nodiscard]] std::size_t maxHandles() const noexcept
    {
        return _customWalls.size();
    }
};

} // namespace hg
