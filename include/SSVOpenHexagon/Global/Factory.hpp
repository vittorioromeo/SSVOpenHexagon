// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_FACTORY
#define HG_FACTORY

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

namespace hg
{
    class HexagonGame;

    class Factory
    {
    private:
        HexagonGame& hexagonGame;
        sses::Manager& manager;
        Vec2f centerPos;

    public:
        Factory(HexagonGame& mHexagonGame, sses::Manager& mManager,
            const Vec2f& mCenterPos)
            : hexagonGame(mHexagonGame), manager(mManager),
              centerPos{mCenterPos}
        {
        }

        void createWall(int mSide, float mThickness,
            const SpeedData& mSpeed, const SpeedData& mCurve = SpeedData{},
            float mHueMod = 0);

        inline sses::Entity& createPlayer()
        {
            auto& result(manager.createEntity());
            result.createComponent<CPlayer>(hexagonGame, centerPos);
            result.setDrawPriority(-1);
            return result;
        }
    };
    
}

#endif
