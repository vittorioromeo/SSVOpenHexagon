// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Data/ColorData.hpp"

#include "SSVOpenHexagon/Utils/TinyVariant.hpp"

namespace Json {
class Value;
}

namespace ssvuj {
using Obj = Json::Value;
}

namespace hg {

namespace CapColorMode {

// clang-format off
struct Main         { };
struct MainDarkened { };
struct ByIndex      { int _index; };
// clang-format on

} // namespace CapColorMode

using CapColor = vittorioromeo::tinyvariant< //
    CapColorMode::Main,                      //
    CapColorMode::MainDarkened,              //
    CapColorMode::ByIndex,                   //
    ColorData                                //
    >;

[[nodiscard]] CapColor parseCapColor(const ssvuj::Obj& obj) noexcept;

} // namespace hg
