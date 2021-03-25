// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"

#include "SSVOpenHexagon/Components/CPlayer.hpp"

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

[[nodiscard]] bool CCustomWallManager::checkValidVertexIdx(
    const CCustomWallHandle h, const int vertexIdx, const char* msg)
{
    if(vertexIdx < 0 || vertexIdx > 3)
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
    const int vertexIdx, const sf::Vector2f& pos)
{
    if(!checkValidVertexIdxAndHandle(
           cwHandle, vertexIdx, "set vertex position"))
    {
        return;
    }

    _customWalls[cwHandle].setVertexPos(vertexIdx, pos);
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
    const CCustomWallHandle cwHandle, const int vertexIdx)
{
    if(!checkValidVertexIdxAndHandle(
           cwHandle, vertexIdx, "get vertex position"))
    {
        return sf::Vector2f{0.f, 0.f};
    }

    return _customWalls[cwHandle].getVertexPos(vertexIdx);
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

    _customWalls[cwHandle].setVertexPos(0, p0);
    _customWalls[cwHandle].setVertexPos(1, p1);
    _customWalls[cwHandle].setVertexPos(2, p2);
    _customWalls[cwHandle].setVertexPos(3, p3);
}

void CCustomWallManager::setVertexColor4(const CCustomWallHandle cwHandle,
    const sf::Color& c0, const sf::Color& c1, const sf::Color& c2,
    const sf::Color& c3)
{
    if(!checkValidHandle(cwHandle, "set four vertex color"))
    {
        return;
    }

    _customWalls[cwHandle].setVertexColor(0, c0);
    _customWalls[cwHandle].setVertexColor(1, c1);
    _customWalls[cwHandle].setVertexColor(2, c2);
    _customWalls[cwHandle].setVertexColor(3, c3);
}

void CCustomWallManager::setVertexColor4Same(
    const CCustomWallHandle cwHandle, const sf::Color& color)
{
    if(!checkValidHandle(cwHandle, "set four vertex color same"))
    {
        return;
    }

    _customWalls[cwHandle].setVertexColor(0, color);
    _customWalls[cwHandle].setVertexColor(1, color);
    _customWalls[cwHandle].setVertexColor(2, color);
    _customWalls[cwHandle].setVertexColor(3, color);
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

void CCustomWallManager::draw(Utils::FastVertexVectorQuads& wallQuads)
{
    for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
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
    const float radiusSquared{radius * radius};

    const auto size{static_cast<int>(_customWalls.size())};
    const sf::Vector2f& pPos{mPlayer.getPosition()};

    {
        bool collided{false};
        for(CCustomWallHandle h = 0; h < size; ++h)
        {
            if(_handleAvailable[h] || //
                !_customWalls[h].getCanCollide() || //
                !_customWalls[h].isOverlapping(pPos))
            {
                continue;
            }

            if(mPlayer.getJustSwapped() ||
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
        for(CCustomWallHandle h = 0; h < size; ++h)
        {
            if(_handleAvailable[h] ||               //
                !_customWalls[h].getCanCollide() || //
                !_customWalls[h].isOverlapping(pPos))
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
    for(CCustomWallHandle h = 0; h < size; ++h)
    {
        if(_handleAvailable[h] ||               //
            !_customWalls[h].getCanCollide() || //
            !_customWalls[h].isOverlapping(pPos))
        {
            continue;
        }

        return true;
    }

    return false;
}

} // namespace hg
