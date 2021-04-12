// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "SSVOpenHexagon/ThreadPool/ConsumerQueuePtr.hpp"
#include "SSVOpenHexagon/Utils/MovableAtomic.hpp"

#include <thread>
#include <atomic>
#include <cstdint>

namespace hg::ThreadPool {

/// @brief Wraps an `std::thread`, `consumer_queue_ptr` and atomic control
/// variables.
class worker
{
private:
    enum class state : std::uint8_t
    {
        // Initial state of the `worker`.
        uninitialized = 0,

        // The `worker` is dequeuing and accepting tasks in blocking mode.
        running = 1,

        // The `worker` is dequeuing and accepting tasks in non-blocking mode.
        // Will transition to `state::finished` automatically when there are no
        // more tasks.
        stopped = 2,

        // The `worker` is done. The thread can be joined.
        finished = 3
    };

    /// @brief Worker thread.
    std::thread _thread;

    /// @brief Pointer to queue + consumer token.
    consumer_queue_ptr _queue;

    /// @brief State of the worker, controlled both by the pool and internally.
    Utils::movable_atomic<state> _state;

    /// @brief Signals when the worker is done processing tasks in blocking
    /// mode. Controlled internally, checked by the pool to start posting dummy
    /// tasks.
    Utils::movable_atomic<bool> _done_blocking_processing;

    /// @brief Execution loop of the worker.
    void run();

public:
    worker(task_queue& queue) noexcept;

    worker(worker&&);
    worker& operator=(worker&&);

    void start(std::atomic<unsigned int>& remaining_inits);

    /// @brief Sets the running flag to false, preventing the worker to
    /// accept tasks.
    void stop() noexcept;

    /// @brief Assuming the worker is not running, tries to join the
    /// underlying thread.
    void join() noexcept;

    /// @brief Returns `true` if the worker has exited the processing loop.
    [[nodiscard]] bool finished() const noexcept;

    /// @brief TODO
    [[nodiscard]] bool done_blocking_processing() const noexcept;
};
} // namespace hg::ThreadPool
