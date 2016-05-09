// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_GLOBAL_COMMON
#define HG_GLOBAL_COMMON

// TODO: move mingw stuff to separate header and include
#ifdef SSVOPENHEXAGON_MINGW_STD_THREADS

#include <thread>
#include <mutex>
#include <condition_variable>

// https://github.com/meganz/mingw-std-threads
#include <mingw.thread.h>
#include <mingw.mutex.h>
#include <mingw.condition_variable.h>

#define BOOST_THREAD_PROVIDES_FUTURE 1
#include <boost/thread/future.hpp>

namespace std
{
    using boost::future;
    using boost::launch;

    template <typename... Ts>
    auto async(Ts&&... xs)
    {
        return boost::async(std::forward<Ts>(xs)...);
    }
}

#define _GLIBCXX_FUTURE 1

#endif

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
