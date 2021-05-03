// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace Json {

class Value;

}

namespace ssvuj {

using Obj = Json::Value;

}

namespace hg {

class MusicData;
struct GameVersion;
class ProfileData;

} // namespace hg

namespace hg::Utils {

[[nodiscard]] MusicData loadMusicFromJson(const ssvuj::Obj& mRoot);
[[nodiscard]] GameVersion loadVersionFromJson(const ssvuj::Obj& mRoot);
[[nodiscard]] ProfileData loadProfileFromJson(const ssvuj::Obj& mRoot);

} // namespace hg::Utils
