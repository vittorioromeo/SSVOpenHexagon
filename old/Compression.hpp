// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// From: https://panthema.net/2007/0328-ZLibString.html

#pragma once

#include <string>

namespace hg {

[[nodiscard]] std::string getZLibCompress(const std::string& mStr);
[[nodiscard]] std::string getZLibDecompress(const std::string& mStr);

} // namespace hg
