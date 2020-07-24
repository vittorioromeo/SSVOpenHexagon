// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ColorData.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <string>

namespace hg 
{
	[[nodiscard]] PulseColor pulse_from_json(const ssvuj::Obj& root) noexcept
	{
		std::vector<int> rawPulseData = ssvuj::getExtr<std::vector<int>>(root, "pulse", {0, 0, 0, 255});
		if (rawPulseData.size() == 4)
		{
			return {rawPulseData[0], rawPulseData[1], rawPulseData[2], rawPulseData[3]};
		}
		return {0, 0, 0, 255};
	}
}
