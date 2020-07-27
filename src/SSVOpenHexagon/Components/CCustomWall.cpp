// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CCustomWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

namespace hg
{

CCustomWall::CCustomWall()
{
}

void CCustomWall::draw(HexagonGame& mHexagonGame)
{
    mHexagonGame.wallQuads.reserve_more(4);

    mHexagonGame.wallQuads.unsafe_emplace_back(
        vertexPositions[0], vertexColors[0]);

    mHexagonGame.wallQuads.unsafe_emplace_back(
        vertexPositions[1], vertexColors[1]);

    mHexagonGame.wallQuads.unsafe_emplace_back(
        vertexPositions[2], vertexColors[2]);

    mHexagonGame.wallQuads.unsafe_emplace_back(
        vertexPositions[3], vertexColors[3]);
}

void CCustomWall::update(HexagonGame& mHexagonGame, ssvu::FT mFT)
{
    (void)mHexagonGame;
    (void)mFT;
}

void CCustomWall::setVertexPos(
    const int vertexIndex, const sf::Vector2f& pos) noexcept
{
    vertexPositions[vertexIndex] = pos;
}

void CCustomWall::setVertexColor(
    const int vertexIndex, const sf::Color& color) noexcept
{
    vertexColors[vertexIndex] = color;
}

[[nodiscard]] sf::Vector2f CCustomWall::getVertexPos(
    const int vertexIndex) const noexcept
{
    return vertexPositions[vertexIndex];
}

} // namespace hg
