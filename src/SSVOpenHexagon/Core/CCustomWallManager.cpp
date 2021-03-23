// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Utils/Containers.hpp>

namespace hg
{

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
    if(_handleAvailable[h])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to " << msg << " of invalid custom wall " << h
            << '\n';

        SSVU_ASSERT(ssvu::contains(_freeHandles, h));
        return false;
    }

    SSVU_ASSERT(isValidHandle(h));
    return true;
}

[[nodiscard]] CCustomWallHandle CCustomWallManager::create()
{
    if(_freeHandles.empty())
    {
        constexpr std::size_t reserveSize = 32;
        const std::size_t maxHandleIndex = _nextFreeHandle + reserveSize;

        _freeHandles.reserve(maxHandleIndex);
        _customWalls.resize(maxHandleIndex);
        _handleAvailable.resize(maxHandleIndex);

        for(std::size_t i = 0; i < reserveSize; ++i)
        {
            _freeHandles.emplace_back(_nextFreeHandle);
            _handleAvailable[_nextFreeHandle] = true;
            ++_nextFreeHandle;
        }
    }

    const auto res = _freeHandles.back();

    _freeHandles.pop_back();
    _handleAvailable[res] = false;
    ++_count;

    return res;
}

void CCustomWallManager::destroy(const CCustomWallHandle cwHandle)
{
    if(_handleAvailable[cwHandle])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to destroy invalid wall " << cwHandle << '\n';

        return;
    }

    SSVU_ASSERT(isValidHandle(cwHandle));

    _handleAvailable[cwHandle] = true;
    --_count;

    SSVU_ASSERT(!ssvu::contains(_freeHandles, cwHandle));
    _freeHandles.emplace_back(cwHandle);
}

void CCustomWallManager::setVertexPos(const CCustomWallHandle cwHandle,
    const int vertexIndex, const sf::Vector2f& pos)
{
    if(vertexIndex < 0 || vertexIndex > 3)
    {
        ssvu::lo("CustomWallManager")
            << "Vertex index " << vertexIndex
            << "out of bounds while attempting to set vertex position of "
               "invalid custom wall "
            << cwHandle << '\n';

        return;
    }

    if(!checkValidHandle(cwHandle, "set vertex position"))
    {
        return;
    }

    _customWalls[cwHandle].setVertexPos(vertexIndex, pos);
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
    if(side > 3u)
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

[[nodiscard]] sf::Vector2f CCustomWallManager::getVertexPos(
    const CCustomWallHandle cwHandle, const int vertexIndex)
{
    if(vertexIndex < 0 || vertexIndex > 3)
    {
        ssvu::lo("CustomWallManager")
            << "Vertex index " << vertexIndex
            << "out of bounds while attempting to get vertex position of "
               "invalid custom wall "
            << cwHandle << '\n';

        return sf::Vector2f{0.f, 0.f};
    }

    if(!checkValidHandle(cwHandle, "get vertex position"))
    {
        return sf::Vector2f{0.f, 0.f};
    }

    return _customWalls[cwHandle].getVertexPos(vertexIndex);
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
    const int vertexIndex, const sf::Color& color)
{
    if(vertexIndex < 0 || vertexIndex > 3)
    {
        ssvu::lo("CustomWallManager")
            << "Vertex index " << vertexIndex
            << "out of bounds while attempting to set vertex position of "
               "invalid custom wall "
            << cwHandle << '\n';

        return;
    }

    if(!checkValidHandle(cwHandle, "set vertex color"))
    {
        return;
    }

    _customWalls[cwHandle].setVertexColor(vertexIndex, color);
}

// TODO:
[[nodiscard]] bool CCustomWallManager::isOverlappingPlayer(
    const CCustomWallHandle cwHandle)
{
    if(!checkValidHandle(cwHandle, "Attempted to check player overlap"))
    {
        return false;
    }

    // TODO:
    return false; // _customWalls[cwHandle].isOverlappingPlayer();
}

void CCustomWallManager::clear()
{
    _freeHandles.clear();
    _customWalls.clear();
    _handleAvailable.clear();
    _nextFreeHandle = 0;
    _count = 0;
}

void CCustomWallManager::draw(HexagonGame& hexagonGame)
{
    for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
    {
        if(!_handleAvailable[h])
        {
            _customWalls[h].draw(hexagonGame);
        }
    }
}

[[nodiscard]] bool CCustomWallManager::handleCollision(
    HexagonGame& mHexagonGame, CPlayer& mPlayer, ssvu::FT mFT)
{
    const float radius{mHexagonGame.getRadius()};
    const float radiusSquared{radius * radius};

    const auto size{static_cast<int>(_customWalls.size())};
    const sf::Vector2f& pPos{mPlayer.getPosition()};

    {
        bool collided{false};
        for(CCustomWallHandle h = 0; h < size; ++h)
        {
            if(_handleAvailable[h] || //
                !_customWalls[h].getCanCollide())
            {
                continue;
            }

            // Broad-phase AABB collision optimization.
            _customWalls[h].updateOutOfPlayerRadius(pPos);

            if(!_customWalls[h].isOverlapping(pPos))
            {
                continue;
            }

            if(mPlayer.getJustSwapped())
            {
                mPlayer.kill(mHexagonGame);
                return true;
            }

            if(mPlayer.push(mHexagonGame, _customWalls[h], radiusSquared, mFT))
            {
                mPlayer.kill(mHexagonGame);
                return false;
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
        bool collided = false;
        for(CCustomWallHandle h = 0; h < size; ++h)
        {
            if(_handleAvailable[h] ||               //
                !_customWalls[h].getCanCollide() || //
                !_customWalls[h].isOverlapping(pPos))
            {
                continue;
            }

            if(mPlayer.push(mHexagonGame, _customWalls[h], radiusSquared, mFT))
            {
                mPlayer.kill(mHexagonGame);
                return false;
            }

            collided = true;
        }

        if(!collided)
        {
            return false;
        }
    }

    // Last round with no push.
    for(CCustomWallHandle h = 0; h < size; ++h)
    {
        if(_handleAvailable[h] ||               //
            !_customWalls[h].getCanCollide() || //
            !_customWalls[h].isOverlapping(pPos))
        {
            continue;
        }

        mPlayer.kill(mHexagonGame);
        return false;
    }

    return false;
}

} // namespace hg
