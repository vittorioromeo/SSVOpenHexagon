// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/BuildPackId.hpp"

#include "TestUtils.hpp"

int main()
{
    TEST_ASSERT_EQ(                                                         //
        hg::Utils::buildPackId("ohvrvanilla", "vittorio romeo", "cube", 1), //
        "ohvrvanilla_vittorio_romeo_cube_1"                                 //
    );

    TEST_ASSERT_EQ(                                                         //
        hg::Utils::buildPackId("ohvrvanilla", "vittorio romeo", "base", 1), //
        "ohvrvanilla_vittorio_romeo_base_1"                                 //
    );

    TEST_ASSERT_EQ(                                       //
        hg::Utils::buildPackId("a b", "c d", "e f", 123), //
        "a_b_c_d_e_f_123"                                 //
    );

    TEST_ASSERT_EQ(                                       //
        hg::Utils::buildPackId(" a ", " b ", " c ", 123), //
        "_a___b___c__123"                                    //
    );
}
