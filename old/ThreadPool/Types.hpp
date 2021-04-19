// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/FixedFunction.hpp"

#include "moodycamel/blockingconcurrentqueue.h"

namespace hg::ThreadPool {

using task = Utils::FixedFunction<void(), 128>;
using task_queue = moodycamel::BlockingConcurrentQueue<task>;
using task_queue_consumer_token = moodycamel::ConsumerToken;
using task_queue_producer_token = moodycamel::ProducerToken;

} // namespace hg::ThreadPool
