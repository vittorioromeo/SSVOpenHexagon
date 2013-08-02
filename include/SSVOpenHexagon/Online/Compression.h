// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

// From: http://panthema.net/2007/0328-ZLibString.html

#include <string>
#include <zlib.h>

#ifndef HG_ONLINE_COMPRESSION
#define HG_ONLINE_COMPRESSION

namespace hg
{
	std::string getZLibCompress(const std::string& mString, int mCompressionlevel = Z_BEST_COMPRESSION);
	std::string getZLibDecompress(const std::string& mString);
}

#endif
