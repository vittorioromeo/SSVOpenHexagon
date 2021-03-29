// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/SparseIntegerSet.hpp"

#include "TestUtils.hpp"

int main()
{
    // ------------------------------------------------------------------------
    hg::Utils::sparse_integer_set<int> s;

    TEST_ASSERT(s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 0);
    TEST_ASSERT_EQ(s.sparse_size(), 0);

    // ------------------------------------------------------------------------
    s.clear();

    TEST_ASSERT(s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 0);
    TEST_ASSERT_EQ(s.sparse_size(), 0);

    // ------------------------------------------------------------------------
    s.grow(32);

    TEST_ASSERT(s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 0);
    TEST_ASSERT_EQ(s.sparse_size(), 32);

    for(int x : s)
    {
        (void) x;
        TEST_ASSERT(false);
    }

    for(int i = 0; i < 32; ++i)
    {
        TEST_ASSERT(!s.has(i));
        TEST_ASSERT(!s.erase(i));
    }

    // ------------------------------------------------------------------------
    TEST_ASSERT(s.insert(0));

    TEST_ASSERT(!s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 1);
    TEST_ASSERT_EQ(s.sparse_size(), 32);

    for(int x : s)
    {
        TEST_ASSERT_EQ(x, 0);
    }

    TEST_ASSERT(s.has(0));

    for(int i = 1; i < 32; ++i)
    {
        TEST_ASSERT(!s.has(i));
        TEST_ASSERT(!s.erase(i));
    }

    // ------------------------------------------------------------------------
    TEST_ASSERT(s.erase(0));

    TEST_ASSERT(s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 0);
    TEST_ASSERT_EQ(s.sparse_size(), 32);

    for(int x : s)
    {
        (void) x;
        TEST_ASSERT(false);
    }

    for(int i = 0; i < 32; ++i)
    {
        TEST_ASSERT(!s.has(i));
        TEST_ASSERT(!s.erase(i));
    }

    // ------------------------------------------------------------------------
    TEST_ASSERT(s.insert(5));
    TEST_ASSERT(s.insert(3));
    TEST_ASSERT(s.insert(6));
    TEST_ASSERT(s.insert(2));
    TEST_ASSERT(s.insert(4));

    TEST_ASSERT(!s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 5);
    TEST_ASSERT_EQ(s.sparse_size(), 32);

    {
        int acc = 0;
        for(int x : s)
        {
            acc += x;
        }

        TEST_ASSERT_EQ(acc, 5 + 3 + 6 + 2 + 4);
    }

    TEST_ASSERT(!s.has(0));
    TEST_ASSERT(!s.has(1));
    TEST_ASSERT(s.has(2));
    TEST_ASSERT(s.has(3));
    TEST_ASSERT(s.has(4));
    TEST_ASSERT(s.has(5));
    TEST_ASSERT(s.has(6));
    TEST_ASSERT(!s.has(7));
    TEST_ASSERT(!s.has(8));

    // ------------------------------------------------------------------------
    TEST_ASSERT(s.erase(3));

    TEST_ASSERT(!s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 4);
    TEST_ASSERT_EQ(s.sparse_size(), 32);

    {
        int acc = 0;
        for(int x : s)
        {
            acc += x;
        }

        TEST_ASSERT_EQ(acc, 5 + 6 + 2 + 4);
    }

    TEST_ASSERT(!s.has(0));
    TEST_ASSERT(!s.has(1));
    TEST_ASSERT(s.has(2));
    TEST_ASSERT(!s.has(3));
    TEST_ASSERT(s.has(4));
    TEST_ASSERT(s.has(5));
    TEST_ASSERT(s.has(6));
    TEST_ASSERT(!s.has(7));
    TEST_ASSERT(!s.has(8));

    // ------------------------------------------------------------------------
    TEST_ASSERT(s.erase(4));

    TEST_ASSERT(!s.dense_empty());
    TEST_ASSERT_EQ(s.dense_size(), 3);
    TEST_ASSERT_EQ(s.sparse_size(), 32);

    {
        int acc = 0;
        for(int x : s)
        {
            acc += x;
        }

        TEST_ASSERT_EQ(acc, 5 + 6 + 2);
    }

    TEST_ASSERT(!s.has(0));
    TEST_ASSERT(!s.has(1));
    TEST_ASSERT(s.has(2));
    TEST_ASSERT(!s.has(3));
    TEST_ASSERT(!s.has(4));
    TEST_ASSERT(s.has(5));
    TEST_ASSERT(s.has(6));
    TEST_ASSERT(!s.has(7));
    TEST_ASSERT(!s.has(8));
}
