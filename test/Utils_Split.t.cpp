// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Split.hpp"

#include "TestUtils.hpp"

#include <string>
#include <vector>

void testSplit(const std::string& s, const std::vector<std::string>& expected)
{
    const std::vector<std::string> splitted = hg::Utils::split<std::string>(s);

    TEST_ASSERT_EQ(splitted.size(), expected.size());
    for(std::size_t i = 0; i < expected.size(); ++i)
    {
        TEST_ASSERT_EQ(splitted.at(i), expected.at(i));
    }
}

int main()
{
    testSplit("", {});
    testSplit("hello", {"hello"});
    testSplit("hello world", {"hello", "world"});

    testSplit(
        "hello world goodbye world", {"hello", "world", "goodbye", "world"});
}
