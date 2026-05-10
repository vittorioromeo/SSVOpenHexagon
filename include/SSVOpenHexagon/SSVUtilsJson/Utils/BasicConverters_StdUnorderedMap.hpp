// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"

#include <unordered_map>

namespace ssvuj
{

template <typename TKey, typename TValue, typename THash, typename TKeyEqual, typename TAlloc>
struct Converter<std::unordered_map<TKey, TValue, THash, TKeyEqual, TAlloc>> :
    KeyValueMapConverter<std::unordered_map<TKey, TValue, THash, TKeyEqual, TAlloc>>
{
};

} // namespace ssvuj
