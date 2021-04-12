// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/FixedFunction.hpp"

#include "TestUtils.hpp"

int main()
{
    {
        hg::Utils::FixedFunction<int(), 64> ff = [] { return 10; };
        TEST_ASSERT_EQ(ff(), 10);
    }

    {
        int i = 10;
        hg::Utils::FixedFunction<int(), 64> ff = [i] { return i; };
        TEST_ASSERT_EQ(ff(), 10);
    }

    {
        int i = 10;
        hg::Utils::FixedFunction<int(), 64> ff0 = [i] { return i; };
        auto ff1 = std::move(ff0);
        TEST_ASSERT_EQ(ff1(), 10);
    }

    {
        int i = 10;
        int j = 5;
        hg::Utils::FixedFunction<int(), 64> ff0 = [i, &j] { return i + j; };
        auto ff1 = std::move(ff0);
        TEST_ASSERT_EQ(ff1(), 10 + 5);
    }
}
