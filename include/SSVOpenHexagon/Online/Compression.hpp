// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

// From: http://panthema.net/2007/0328-ZLibString.html

#pragma once

#include <string>
#include <zlib.h>

namespace hg
{

std::string getZLibCompress(
    const std::string& mStr, int mCompressionlevel = Z_BEST_COMPRESSION);

std::string getZLibDecompress(const std::string& mStr);

} // namespace hg
