// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/ThreadPool/Pool.hpp"

#include "TestUtils.hpp"

#include <atomic>
#include <thread>
#include <chrono>

#include <iostream>

int main()
{
    std::atomic<int> acc = 0;

    {
        hg::ThreadPool::pool pool;

        for(int i = 0; i < 100; ++i)
        {
            pool.post(
                [&acc, i]
                {
                    acc.fetch_add(1);
                    std::cout << "after add 1" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(i));
                    acc.fetch_add(1);
                    std::cout << "after add 2" << std::endl;
                });
        }
    }

    TEST_ASSERT_EQ(acc.load(), 200);
}
