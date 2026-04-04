// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SFML/Base/Array.hpp>
#include <SFML/Base/IntTypes.hpp>
#include <SFML/Base/SizeT.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vec2.hpp>
#include <vector>

namespace hg
{

class CPlayer;

class CCustomWallManager
{
    std::vector<CCustomWall>       _customWalls;
    std::vector<CCustomWallHandle> _freeHandles;
    std::vector<bool>              _handleAvailable;
    CCustomWallHandle              _nextFreeHandle{0};
    sf::base::SizeT                _count{0};
    std::vector<CCustomWallHandle> _tempAliveHandles;

    [[nodiscard]] bool isValidHandle(const CCustomWallHandle h) const noexcept;

    [[nodiscard]] bool checkValidHandle(const CCustomWallHandle h, const char* msg);

    [[nodiscard]] bool checkValidVertexIdx(const CCustomWallHandle h, const int vertexIdx, const char* msg);

    [[nodiscard]] bool checkValidVertexIdxAndHandle(const CCustomWallHandle h, const int vertexIdx, const char* msg);

    void destroyUnchecked(const CCustomWallHandle cwHandle);

public:
    [[nodiscard]] CCustomWallHandle create(void (*fAfterCreate)(CCustomWall&));

    void destroy(const CCustomWallHandle cwHandle);

    void destroyAllOutOfBounds(const sf::Vec2f bounds);

    void setVertexPos(const CCustomWallHandle cwHandle, const int vertexIdx, const sf::Vec2f pos);

    void moveVertexPos(const CCustomWallHandle cwHandle, const int vertexIdx, const sf::Vec2f offset);

    void moveVertexPos4Same(const CCustomWallHandle cwHandle, const sf::Vec2f offset);

    void setCanCollide(const CCustomWallHandle cwHandle, const bool collide);

    void setDeadly(const CCustomWallHandle cwHandle, const bool deadly);

    void setKillingSide(const CCustomWallHandle cwHandle, const sf::base::U8 killingSide);

    void setVertexColor(const CCustomWallHandle cwHandle, const int vertexIdx, const sf::Color& color);

    void setVertexPos4(const CCustomWallHandle cwHandle,
                       const sf::Vec2f         p0,
                       const sf::Vec2f         p1,
                       const sf::Vec2f         p2,
                       const sf::Vec2f         p3);

    void setVertexColor4(const CCustomWallHandle cwHandle,
                         const sf::Color&        c0,
                         const sf::Color&        c1,
                         const sf::Color&        c2,
                         const sf::Color&        c3);

    void setVertexColor4Same(const CCustomWallHandle cwHandle, const sf::Color& color);

    [[nodiscard]] sf::Vec2f getVertexPos(const CCustomWallHandle cwHandle, const int vertexIdx);

    [[nodiscard]] const sf::base::Array<sf::Vec2f, 4>& getVertexPos4(const CCustomWallHandle cwHandle);

    [[nodiscard]] bool getCanCollide(const CCustomWallHandle cwHandle);

    [[nodiscard]] bool getDeadly(const CCustomWallHandle cwHandle);

    [[nodiscard]] sf::base::U8 getKillingSide(const CCustomWallHandle cwHandle);

    void clear();
    void draw(Utils::FastVertexVectorTris& wallQuads);

    [[nodiscard]] bool handleCollision(const int movement, const float radius, CPlayer& mPlayer, float mFT);

    [[nodiscard]] sf::base::SizeT count() const noexcept
    {
        return _count;
    }

    [[nodiscard]] sf::base::SizeT maxHandles() const noexcept
    {
        return _customWalls.size();
    }
};

} // namespace hg
