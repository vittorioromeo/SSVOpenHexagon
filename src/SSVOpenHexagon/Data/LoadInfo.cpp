// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/LoadInfo.hpp"

#include <string>
#include <vector>
#include <cstddef>

namespace hg {

void LoadInfo::addFormattedError(std::string& error)
{
    if(error.empty())
    {
        return;
    }

    // Remove the first two characters
    error.erase(0, 2);

    // Replace first newline with '-', place a space before it,
    // and remove a space after it.
    std::size_t i = error.find('\n');
    error.insert(i, " ");
    error[++i] = '-';
    error.erase(++i, 1);

    // Remove all other newlines.
    while((i = error.find('\n', i)) != std::string::npos)
    {
        error.erase(i, 1);
    }

    errorMessages.emplace_back(error);
}

} // namespace hg
