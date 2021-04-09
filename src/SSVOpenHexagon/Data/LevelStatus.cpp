// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/LevelStatus.hpp"

namespace hg {

LevelStatus::LevelStatus(
    const bool mSyncMusicToDM, const float mWallSpawnDistance)
    : syncMusicToDM{mSyncMusicToDM}, wallSpawnDistance{mWallSpawnDistance}
{}

[[nodiscard]] bool LevelStatus::hasSpeedMaxLimit() const noexcept
{
    return speedMax > 0.f;
}

[[nodiscard]] bool LevelStatus::hasDelayMaxLimit() const noexcept
{
    return delayMax > 0.f;
}

} // namespace hg
