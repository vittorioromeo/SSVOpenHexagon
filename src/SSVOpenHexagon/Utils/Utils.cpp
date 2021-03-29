// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

#include <SSVStart/Camera/Camera.hpp>

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <fstream>

#include <dirent.h>
#include <sys/stat.h>

namespace hg::Utils
{

std::string getLocalValidator(const std::string& mId, float mDifficultyMult)
{
    return mId + "_m_" + ssvu::toStr(mDifficultyMult);
}

void shakeCamera(ssvu::TimelineManager& mTimelineManager, ssvs::Camera& mCamera)
{
    int s{7};
    sf::Vector2f oldCenter{mCamera.getCenter()};
    ssvu::Timeline& timeline(mTimelineManager.create());

    for(int i{s}; i > 0; --i)
    {
        timeline.append<ssvu::Do>([&mCamera, oldCenter, i] {
            mCamera.setCenter(oldCenter + sf::Vector2f(ssvu::getRndI(-i, i),
                                              ssvu::getRndI(-i, i)));
        });
        timeline.append<ssvu::Wait>(1);
        timeline.append<ssvu::Go>(0, 3);
    }

    timeline.append<ssvu::Do>(
        [&mCamera, oldCenter] { mCamera.setCenter(oldCenter); });
}

} // namespace hg::Utils
