// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_FACTORY
#define HG_FACTORY

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

namespace hg
{
    class HexagonGame;

    class Factory
    {
    private:
        HexagonGame& hexagonGame;
        Vec2f centerPos;

    public:
        Factory(HexagonGame& mHexagonGame,
            const Vec2f& mCenterPos)
            : hexagonGame(mHexagonGame),
              centerPos{mCenterPos}
        {
        }

        void createWall(int mSide, float mThickness,
            const SpeedData& mSpeed, const SpeedData& mCurve = SpeedData{},
            float mHueMod = 0);
    };
    
}

#endif
