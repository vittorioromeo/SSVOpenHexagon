// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_GLOBAL_MINGW
#define HG_GLOBAL_MINGW

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
