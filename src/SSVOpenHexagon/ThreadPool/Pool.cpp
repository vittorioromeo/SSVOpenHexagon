// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/ThreadPool/Pool.hpp"

#include "SSVOpenHexagon/ThreadPool/Worker.hpp"
#include "SSVOpenHexagon/ThreadPool/Types.hpp"

#include <utility>
#include <chrono>
#include <thread>

namespace hg::ThreadPool {

[[nodiscard]] bool pool::all_workers_done_blocking_processing() const noexcept
{
    for(const worker& w : _workers)
    {
        if(!w.done_blocking_processing())
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool pool::all_workers_finished() const noexcept
{
    for(const worker& w : _workers)
    {
        if(!w.finished())
        {
            return false;
        }
    }

    return true;
}

void pool::post_dummy_task()
{
    post([] {});
}

void pool::initialize_workers(const unsigned int n)
{
    // Create workers.
    for(unsigned int i(0u); i < n; ++i)
    {
        _workers.emplace_back(_queue);
    }

    // Start workers.
    _remaining_inits.store(n);
    for(worker& w : _workers)
    {
        w.start(_remaining_inits);
    }
}

pool::pool()
{
    const unsigned int worker_count = std::thread::hardware_concurrency();
    initialize_workers(worker_count);
}

pool::~pool()
{
    // Busy wait for all workers to be initialized.
    while(_remaining_inits.load() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Signal all workers to exit their processing loops.
    for(worker& w : _workers)
    {
        w.stop();
    }

    // Post dummy tasks until all workers have exited their loops.
    while(!all_workers_done_blocking_processing())
    {
        post_dummy_task();
    }

    // TODO
    while(!all_workers_finished())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Join the workers' threads.
    for(worker& w : _workers)
    {
        w.join();
    }
}

void pool::post(task&& f)
{
    const bool enqueued = _queue.enqueue(std::move(f));
    SSVOH_ASSERT(enqueued);
}

} // namespace hg::ThreadPool
