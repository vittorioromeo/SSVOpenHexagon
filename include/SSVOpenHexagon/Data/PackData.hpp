// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>

namespace hg
{

struct PackData
{
    std::string folderPath;
    std::string id;
    std::string disambiguator;
    std::string name;
    std::string author;
    std::string description;
    int version;
    float priority;
};

} // namespace hg
