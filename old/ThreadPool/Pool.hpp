// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include "SSVOpenHexagon/ThreadPool/Types.hpp"

#include <vector>
#include <atomic>

namespace hg::ThreadPool {

class worker;

class pool
{
private:
    task_queue _queue;
    std::vector<worker> _workers;
    std::atomic<unsigned int> _remaining_inits;

    /// @brief  Returns `true` if all workers have finished processing packets
    /// in a blocking manner.
    [[nodiscard]] bool all_workers_done_blocking_processing() const noexcept;

    /// @brief Returns `true` if all workers have finished (exited from
    /// loop).
    [[nodiscard]] bool all_workers_finished() const noexcept;

    /// @brief Posts a dummy empty task, used to unblock workers waiting to
    /// be destroyed.
    void post_dummy_task();

    /// @brief Creates and starts `n` workers, also initializing the
    /// atomic remaining inits counter.
    void initialize_workers(const unsigned int n);

public:
    explicit pool();
    ~pool();

    void post(task&& f);
};

} // namespace hg::ThreadPool
