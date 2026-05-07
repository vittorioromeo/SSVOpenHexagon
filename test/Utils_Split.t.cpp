// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Split.hpp"
#include "TestUtils.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringStreamOp.hpp"
#include "SFML/Base/Vector.hpp"

void testSplit(const sf::base::String& s, const sf::base::Vector<sf::base::String>& expected)
{
    const sf::base::Vector<sf::base::String> splitted = hg::Utils::split<sf::base::String>(s);

    TEST_ASSERT_EQ(splitted.size(), expected.size());
    for (sf::base::SizeT i = 0; i < expected.size(); ++i)
    {
        TEST_ASSERT_EQ(splitted[i], expected[i]);
    }
}

int main()
{
    testSplit("", {});
    testSplit("hello", {"hello"});
    testSplit("hello world", {"hello", "world"});

    testSplit("hello world goodbye world", {"hello", "world", "goodbye", "world"});
}
