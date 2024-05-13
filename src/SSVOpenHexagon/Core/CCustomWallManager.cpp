// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"

#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Utils/Containers.hpp>

namespace hg {

[[nodiscard]] bool CCustomWallManager::isValidHandle(
    const CCustomWallHandle h) const noexcept
{
    return h >= 0 &&                                    //
           h < static_cast<int>(_customWalls.size()) && //
           h < static_cast<int>(_handleAvailable.size());
}

[[nodiscard]] bool CCustomWallManager::checkValidHandle(
    const CCustomWallHandle h, const char* msg)
{
    if(_handleAvailable[h]) [[unlikely]]
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to " << msg << " of invalid custom wall " << h
            << '\n';

        SSVOH_ASSERT(ssvu::contains(_freeHandles, h));
        return false;
    }

    SSVOH_ASSERT(isValidHandle(h));
    return true;
}

[[nodiscard]] bool CCustomWallManager::checkValidVertexIdx(
    const CCustomWallHandle h, const int vertexIdx, const char* msg)
{
    if(vertexIdx < 0 || vertexIdx > 3) [[unlikely]]
    {
        ssvu::lo("CustomWallManager")
            << "Invalid vertex index " << vertexIdx << " for custom wall " << h
            << " while attempting to " << msg << '\n';

        return false;
    }

    return true;
}

[[nodiscard]] bool CCustomWallManager::checkValidVertexIdxAndHandle(
    const CCustomWallHandle h, const int vertexIdx, const char* msg)
{
    return checkValidVertexIdx(h, vertexIdx, msg) && checkValidHandle(h, msg);
}

[[nodiscard]] CCustomWallHandle CCustomWallManager::create(
    void (*fAfterCreate)(CCustomWall&))
{
    if(_freeHandles.empty()) [[unlikely]]
    {
        const std::size_t reserveSize = 32 + _nextFreeHandle * 2;
        const std::size_t maxHandleIndex = _nextFreeHandle + reserveSize;

        _freeHandles.reserve(maxHandleIndex);
        _customWalls.resize(maxHandleIndex);
        _handleAvailable.resize(maxHandleIndex);

        for(std::size_t i = 0; i < reserveSize; ++i)
        {
            _freeHandles.emplace_back(_nextFreeHandle + i);
            _handleAvailable[_nextFreeHandle + i] = true;
        }

        _nextFreeHandle = maxHandleIndex;
    }

    const auto res = _freeHandles.back();

    _freeHandles.pop_back();
    _handleAvailable[res] = false;
    ++_count;

    // Restore default state
    CCustomWall& cw = _customWalls[res];
    cw.reset();

    fAfterCreate(cw);

    return res;
}

void CCustomWallManager::destroyUnchecked(const CCustomWallHandle cwHandle)
{
    SSVOH_ASSERT(!_handleAvailable[cwHandle]);
    SSVOH_ASSERT(isValidHandle(cwHandle));

    _handleAvailable[cwHandle] = true;
    --_count;

    SSVOH_ASSERT(!ssvu::contains(_freeHandles, cwHandle));
    _freeHandles.emplace_back(cwHandle);
}

void CCustomWallManager::destroy(const CCustomWallHandle cwHandle)
{
    if(_handleAvailable[cwHandle]) [[unlikely]]
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to destroy invalid wall " << cwHandle << '\n';

        return;
    }

    destroyUnchecked(cwHandle);
}

void CCustomWallManager::setVertexPos(const CCustomWallHandle cwHandle,
    const int vertexIdx, const sf::Vector2f& pos)
{
    if(!checkValidVertexIdxAndHandle(cwHandle, vertexIdx, "set vertex pos"))
    {
        return;
    }

    _customWalls[cwHandle].setVertexPos(vertexIdx, pos);
}

void CCustomWallManager::moveVertexPos(const CCustomWallHandle cwHandle,
    const int vertexIdx, const sf::Vector2f& offset)
{
    if(!checkValidVertexIdxAndHandle(cwHandle, vertexIdx, "add vertex pos"))
    {
        return;
    }

    _customWalls[cwHandle].moveVertexPos(vertexIdx, offset);
}

void CCustomWallManager::moveVertexPos4Same(
    const CCustomWallHandle cwHandle, const sf::Vector2f& offset)
{
    if(!checkValidHandle(cwHandle, "add four vertex pos same"))
    {
        return;
    }

    _customWalls[cwHandle].moveVertexPos4Same(offset);
}

void CCustomWallManager::setCanCollide(
    const CCustomWallHandle cwHandle, const bool collide)
{
    if(!checkValidHandle(cwHandle, "set collision"))
    {
        return;
    }

    _customWalls[cwHandle].setCanCollide(collide);
}

void CCustomWallManager::setDeadly(
    const CCustomWallHandle cwHandle, const bool deadly)
{
    if(!checkValidHandle(cwHandle, "set deadly status"))
    {
        return;
    }

    _customWalls[cwHandle].setDeadly(deadly);
}

void CCustomWallManager::setKillingSide(
    const CCustomWallHandle cwHandle, const std::uint8_t side)
{
    if(side > 3u) [[unlikely]]
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to set killing side with invalid value " << side
            << ", acceptable values are 0 to 3\n";

        return;
    }

    if(!checkValidHandle(cwHandle, "set killing side"))
    {
        return;
    }

    _customWalls[cwHandle].setKillingSide(side);
}

[[nodiscard]] const sf::Vector2f& CCustomWallManager::getVertexPos(
    const CCustomWallHandle cwHandle, const int vertexIdx)
{
    if(!checkValidVertexIdxAndHandle(cwHandle, vertexIdx, "get vertex pos"))
    {
        return ssvs::zeroVec2f;
    }

    return _customWalls[cwHandle].getVertexPos(vertexIdx);
}

static const std::array<sf::Vector2f, 4> zeroArr{
    ssvs::zeroVec2f, ssvs::zeroVec2f, ssvs::zeroVec2f, ssvs::zeroVec2f};

[[nodiscard]] const std::array<sf::Vector2f, 4>&
CCustomWallManager::getVertexPos4(const CCustomWallHandle cwHandle)
{
    if(!checkValidHandle(cwHandle, "get four vertex pos"))
    {
        return zeroArr;
    }

    return _customWalls[cwHandle].getVertexPositions();
}

[[nodiscard]] bool CCustomWallManager::getCanCollide(
    const CCustomWallHandle cwHandle)
{
    if(!checkValidHandle(cwHandle, "get collision"))
    {
        return false;
    }

    return _customWalls[cwHandle].getCanCollide();
}

[[nodiscard]] bool CCustomWallManager::getDeadly(
    const CCustomWallHandle cwHandle)
{
    if(!checkValidHandle(cwHandle, "get deadly status"))
    {
        return false;
    }

    return _customWalls[cwHandle].getDeadly();
}

[[nodiscard]] std::uint8_t CCustomWallManager::getKillingSide(
    const CCustomWallHandle cwHandle)
{
    if(!checkValidHandle(cwHandle, "get killing side"))
    {
        return -1;
    }

    return _customWalls[cwHandle].getKillingSide();
}

void CCustomWallManager::setVertexColor(const CCustomWallHandle cwHandle,
    const int vertexIdx, const sf::Color& color)
{
    if(!checkValidVertexIdxAndHandle(cwHandle, vertexIdx, "set vertex color"))
    {
        return;
    }

    _customWalls[cwHandle].setVertexColor(vertexIdx, color);
}

void CCustomWallManager::setVertexPos4(const CCustomWallHandle cwHandle,
    const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::Vector2f& p2,
    const sf::Vector2f& p3)
{
    if(!checkValidHandle(cwHandle, "set four vertex pos"))
    {
        return;
    }

    CCustomWall& customWall = _customWalls[cwHandle];
    customWall.setVertexPos(0, p0);
    customWall.setVertexPos(1, p1);
    customWall.setVertexPos(2, p2);
    customWall.setVertexPos(3, p3);
}

void CCustomWallManager::setVertexColor4(const CCustomWallHandle cwHandle,
    const sf::Color& c0, const sf::Color& c1, const sf::Color& c2,
    const sf::Color& c3)
{
    if(!checkValidHandle(cwHandle, "set four vertex color"))
    {
        return;
    }

    CCustomWall& customWall = _customWalls[cwHandle];
    customWall.setVertexColor(0, c0);
    customWall.setVertexColor(1, c1);
    customWall.setVertexColor(2, c2);
    customWall.setVertexColor(3, c3);
}

void CCustomWallManager::setVertexColor4Same(
    const CCustomWallHandle cwHandle, const sf::Color& color)
{
    if(!checkValidHandle(cwHandle, "set four vertex color same"))
    {
        return;
    }

    CCustomWall& customWall = _customWalls[cwHandle];
    customWall.setVertexColor(0, color);
    customWall.setVertexColor(1, color);
    customWall.setVertexColor(2, color);
    customWall.setVertexColor(3, color);
}

void CCustomWallManager::clear()
{
    _freeHandles.clear();
    _customWalls.clear();
    _handleAvailable.clear();
    _nextFreeHandle = 0;
    _count = 0;
}

void CCustomWallManager::draw(Utils::FastVertexVectorTris& wallQuads)
{
    for(CCustomWallHandle h = 0; h < static_cast<int>(_customWalls.size()); ++h)
    {
        if(!_handleAvailable[h])
        {
            _customWalls[h].draw(wallQuads);
        }
    }
}

[[nodiscard]] bool CCustomWallManager::handleCollision(
    const int movement, const float radius, CPlayer& mPlayer, ssvu::FT mFT)
{
    // ------------------------------------------------------------------------
    // Get all alive walls
    _tempAliveHandles.clear();

    for(CCustomWallHandle h = 0; h < static_cast<int>(_customWalls.size()); ++h)
    {
        if(!_handleAvailable[h] && _customWalls[h].getCanCollide())
        {
            _tempAliveHandles.emplace_back(h);
        }
    }

    const float radiusSquared{radius * radius};
    const sf::Vector2f& pPos{mPlayer.getPosition()};

    {
        bool collided{false};
        for(const CCustomWallHandle h : _tempAliveHandles)
        {
            if(!_customWalls[h].isOverlapping(pPos))
            {
                continue;
            }

            if(mPlayer.getJustSwapped() || _customWalls[h].getDeadly() ||
                mPlayer.push(
                    movement, radius, _customWalls[h], radiusSquared, mFT))
            {
                return true;
            }

            collided = true;
        }

        if(!collided)
        {
            return false;
        }
    }

    // Recheck collision on all walls.
    {
        bool collided{false};
        for(const CCustomWallHandle h : _tempAliveHandles)
        {
            if(!_customWalls[h].isOverlapping(pPos))
            {
                continue;
            }

            if(mPlayer.push(
                   movement, radius, _customWalls[h], radiusSquared, mFT))
            {
                return true;
            }

            collided = true;
        }

        if(!collided)
        {
            return false;
        }
    }

    // Last round with no push.
    for(const CCustomWallHandle h : _tempAliveHandles)
    {
        if(!_customWalls[h].isOverlapping(pPos))
        {
            continue;
        }

        return true;
    }

    return false;
}

} // namespace hg
