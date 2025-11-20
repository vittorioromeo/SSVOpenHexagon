// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#define SSVOH_TO_SIZET(...) static_cast<decltype(sizeof(int))>(__VA_ARGS__)
