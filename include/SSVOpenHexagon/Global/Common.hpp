// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_GLOBAL_COMMON
#define HG_GLOBAL_COMMON

// MinGW-related threading fixes.
#include "SSVOpenHexagon/Global/MinGW.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.hpp>
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include <SSVStart/SSVStart.hpp>
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include <SSVEntitySystem/SSVEntitySystem.hpp>
#include <SSVMenuSystem/SSVMenuSystem.hpp>
#include <SSVLuaWrapper/SSVLuaWrapper.hpp>

namespace hg
{
    // Typedefs
    using ssvufs::Path;
    using ssvs::Vec2;
    using ssvs::UPtr;
    using ssvs::Vec2i;
    using ssvs::Vec2f;
    using ssvs::Vec2u;
    using ssvu::FT;
    using ssvu::Ticker;
    using ssvu::SizeT;

    // Game enums
    enum HGGroup
    {
        Wall
    };
}

#endif
