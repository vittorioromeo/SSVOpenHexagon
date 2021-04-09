// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>

namespace hg::Utils {

[[nodiscard]] std::string buildPackId(const std::string& packDisambiguator,
    const std::string& packAuthor, const std::string& packName,
    const int packVersion);

} // namespace hg::Utils
