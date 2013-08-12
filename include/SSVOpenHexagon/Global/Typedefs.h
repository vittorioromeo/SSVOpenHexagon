// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_GLOBAL_TYPEDEFS
#define HG_GLOBAL_TYPEDEFS

#include <SSVUtils/SSVUtils.h>

namespace hg
{
	using Path = ssvu::FileSystem::Path;
	template<typename T, typename TDeleter = std::default_delete<T>> using Uptr = ssvu::Uptr<T, TDeleter>;
}

#endif
