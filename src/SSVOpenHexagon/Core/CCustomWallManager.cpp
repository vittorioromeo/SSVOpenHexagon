// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Utils/Containers.hpp>

namespace hg
{

[[nodiscard]] bool CCustomWallManager::isValidHandle(
    const CCustomWallHandle h) const noexcept
{
    return h >= 0 &&                       //
           h < (int)_customWalls.size() && //
           h < (int)_handleAvailable.size();
}

[[nodiscard]] CCustomWallHandle CCustomWallManager::create()
{
    if(_freeHandles.empty())
    {
        constexpr std::size_t reserveSize = 255;
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

    if(_handleAvailable[cwHandle])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to set vertex position of invalid custom wall "
            << cwHandle << '\n';

        SSVU_ASSERT(ssvu::contains(_freeHandles, cwHandle));
        return;
    }

    SSVU_ASSERT(isValidHandle(cwHandle));

    _customWalls[cwHandle].setVertexPos(vertexIndex, pos);
}

void CCustomWallManager::setCanCollide(
    const CCustomWallHandle cwHandle, const bool collide)
{
    if(_handleAvailable[cwHandle])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to set collision of invalid custom wall " << cwHandle
            << '\n';

        SSVU_ASSERT(ssvu::contains(_freeHandles, cwHandle));
        return;
    }

    SSVU_ASSERT(isValidHandle(cwHandle));

    _customWalls[cwHandle].setCanCollide(collide);
}

// void CCustomWallManager::setRenderOrder(
//     const CCustomWallHandle cwHandle, const int8_t order)
// {
//     if(_handleAvailable[cwHandle])
//     {
//         ssvu::lo("CustomWallManager")
//             << "Attempted to set render order of invalid custom wall "
//             << cwHandle << '\n';

//         SSVU_ASSERT(ssvu::contains(_freeHandles, cwHandle));
//         return;
//     }

//     SSVU_ASSERT(isValidHandle(cwHandle));

//     _customWalls[cwHandle].setRenderOrder(order);
// }

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

    if(_handleAvailable[cwHandle])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to get vertex position of invalid custom wall "
            << cwHandle << '\n';

        SSVU_ASSERT(ssvu::contains(_freeHandles, cwHandle));
        return sf::Vector2f{0.f, 0.f};
    }

    SSVU_ASSERT(isValidHandle(cwHandle));

    return _customWalls[cwHandle].getVertexPos(vertexIndex);
}

[[nodiscard]] bool CCustomWallManager::getCanCollide(
    const CCustomWallHandle cwHandle)
{
    if(_handleAvailable[cwHandle])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to get collision of invalid custom wall " << cwHandle
            << '\n';

        SSVU_ASSERT(ssvu::contains(_freeHandles, cwHandle));
        return false;
    }

    SSVU_ASSERT(isValidHandle(cwHandle));

    return _customWalls[cwHandle].getCanCollide();
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

    if(_handleAvailable[cwHandle])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to set vertex color of invalid custom wall "
            << cwHandle << '\n';

        SSVU_ASSERT(ssvu::contains(_freeHandles, cwHandle));
        return;
    }

    SSVU_ASSERT(isValidHandle(cwHandle));

    _customWalls[cwHandle].setVertexColor(vertexIndex, color);
}

// TODO:
[[nodiscard]] bool CCustomWallManager::isOverlappingPlayer(
    const CCustomWallHandle cwHandle)
{
    if(_handleAvailable[cwHandle])
    {
        ssvu::lo("CustomWallManager")
            << "Attempted to check player overlap of invalid custom wall "
            << cwHandle << '\n';

        SSVU_ASSERT(ssvu::contains(_freeHandles, cwHandle));
        return false;
    }

    SSVU_ASSERT(isValidHandle(cwHandle));

    // TODO:
    return false; // _customWalls[cwHandle].isOverlappingPlayer();
}

void CCustomWallManager::cleanup()
{
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

} // namespace hg
