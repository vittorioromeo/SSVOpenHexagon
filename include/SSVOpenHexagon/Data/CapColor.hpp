// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <variant>

namespace hg
{

namespace CapColorMode
{

// clang-format off
struct Main         { };
struct MainDarkened { };
struct ByIndex      { int index; };
// clang-format on

} // namespace CapColorMode

using CapColor = std::variant<  //
    CapColorMode::Main,         //
    CapColorMode::MainDarkened, //
    CapColorMode::ByIndex       //
    >;

[[nodiscard]] CapColor parseCapColor(const ssvuj::Obj& obj) noexcept;

} // namespace hg
