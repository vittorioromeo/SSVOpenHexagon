// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

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

void CCustomWall::draw3D(HexagonGame& mHexagonGame, const sf::Color& mColor)
{
    draw(mHexagonGame);

    const sf::Vector2f& offset3D{mHexagonGame.get3DOffset()};
    mHexagonGame.wallQuads3D.reserve_more(4 * 4);

    for(unsigned int i{0}, j{3}; i < 4u; j = i++)
    {
        mHexagonGame.wallQuads3D.unsafe_emplace_back(
            vertexPositions[i], mColor);

        mHexagonGame.wallQuads3D.unsafe_emplace_back(
            vertexPositions[j], mColor);

        mHexagonGame.wallQuads3D.unsafe_emplace_back(
            sf::Vector2f{vertexPositions[j].x + offset3D.x,
                vertexPositions[j].y + offset3D.y},
            mColor);

        mHexagonGame.wallQuads3D.unsafe_emplace_back(
            sf::Vector2f{vertexPositions[i].x + offset3D.x,
                vertexPositions[i].y + offset3D.y},
            mColor);
    }
}

void CCustomWall::update(HexagonGame& mHexagonGame, ssvu::FT mFT)
{
    (void)mHexagonGame;
    (void)mFT;
}

} // namespace hg
