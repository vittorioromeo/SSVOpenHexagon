// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/ThreadPool/Worker.hpp"

#include "SSVOpenHexagon/ThreadPool/ConsumerQueuePtr.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"

#include <thread>
#include <atomic>

namespace hg::ThreadPool {

void worker::run()
{
    SSVOH_ASSERT(_state.load() == state::running);

    // Next task buffer.
    task t;

    // While the worker is running...
    while(_state.load() == state::running)
    {
        // ...dequeue a task (blocking).
        _queue->wait_dequeue(_queue.ctok(), t);

        // Execute the task.
        t();
    }

    // Signal the thread pool to send dummy final tasks.
    SSVOH_ASSERT(_state.load() == state::stopped);
    _done_blocking_processing.store(true);

    // While the worker is being stopped...
    while(_state.load() == state::stopped)
    {
        // ...try dequeueing a task (non-blocking).
        if(!_queue->try_dequeue(_queue.ctok(), t))
        {
            // Break if there are no more tasks.
            break;
        }

        // Execute the task if available.
        t();
    }

    // Signal the thread pool to join.
    SSVOH_ASSERT(_state.load() == state::stopped);
    _state.store(state::finished);
}

worker::worker(task_queue& queue) noexcept
    : _queue{queue},
      _state{state::uninitialized},
      _done_blocking_processing{false}
{}

worker::worker(worker&&) = default;
worker& worker::operator=(worker&&) = default;

void worker::start(std::atomic<unsigned int>& remaining_inits)
{
    SSVOH_ASSERT(_state.load() == state::uninitialized);

    // Start the worker thread.
    _thread = std::thread(
        [this, &remaining_inits]
        {
            // Set the running flag and signal the pool the thread has been
            // initialized.
            _state.store(state::running);
            remaining_inits.fetch_sub(1);

            run();
        });
}

void worker::stop() noexcept
{
    SSVOH_ASSERT(_state.load() == state::running);

    _state.store(state::stopped);
}

void worker::join() noexcept
{
    SSVOH_ASSERT(_thread.joinable());
    SSVOH_ASSERT(_state.load() == state::finished);

    _thread.join();
}

[[nodiscard]] bool worker::finished() const noexcept
{
    return _state.load() == state::finished;
}

[[nodiscard]] bool worker::done_blocking_processing() const noexcept
{
    return _done_blocking_processing.load();
}

} // namespace hg::ThreadPool
