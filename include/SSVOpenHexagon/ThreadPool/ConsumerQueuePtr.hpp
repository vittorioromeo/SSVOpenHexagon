// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/ThreadPool/Types.hpp"

namespace hg::ThreadPool {

/// @brief Wraps a `task_queue` pointer and a consumer token.
class consumer_queue_ptr
{
private:
    task_queue* _queue;
    task_queue_consumer_token _ctok;

public:
    explicit consumer_queue_ptr(task_queue& queue) noexcept;

    consumer_queue_ptr(const consumer_queue_ptr&) = delete;
    consumer_queue_ptr& operator=(const consumer_queue_ptr&) = delete;

    consumer_queue_ptr(consumer_queue_ptr&& rhs) noexcept;
    consumer_queue_ptr& operator=(consumer_queue_ptr&& rhs) noexcept;

    [[nodiscard]] task_queue_consumer_token& ctok() noexcept;

    [[nodiscard]] task_queue* operator->() noexcept;
    [[nodiscard]] task_queue* operator->() const noexcept;
};

} // namespace hg::ThreadPool
