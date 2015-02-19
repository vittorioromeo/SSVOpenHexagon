// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_GLOBAL_COMMON
#define HG_GLOBAL_COMMON

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
	using Path = ssvufs::Path;
	template<typename T> using Vec2 = ssvs::Vec2<T>;
	template<typename T, typename TD = ssvu::DefDel<T>> using UPtr = ssvs::UPtr<T, TD>;
	using Vec2i = ssvs::Vec2i;
	using Vec2f = ssvs::Vec2f;
	using Vec2u = ssvs::Vec2u;
	using FT = ssvu::FT;
	using Ticker = ssvu::Ticker;

	// Game enums
	enum HGGroup{Wall};
}

#endif
