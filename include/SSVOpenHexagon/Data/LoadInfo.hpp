// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <vector>

namespace hg {

struct LoadInfo
{
    unsigned int packs{0};
    unsigned int levels{0};
    unsigned int assets{0};
    std::vector<std::string> errorMessages;

    void addFormattedError(std::string& error);
};

} // namespace hg
