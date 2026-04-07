// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/CapColor.hpp"
#include "SSVOpenHexagon/Data/ColorData.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"

#include "SFML/Base/String.hpp"


namespace hg
{

[[nodiscard]] CapColor parseCapColor(const ssvuj::Obj& obj) noexcept
{
    if (ssvuj::isObjType<sf::base::String>(obj))
    {
        const auto str = ssvuj::getExtr<sf::base::String>(obj);

        if (str == "main")
        {
            return CapColor{CapColorMode::Main{}};
        }

        if (str == "main_darkened")
        {
            return CapColor{CapColorMode::MainDarkened{}};
        }
    }

    if (ssvuj::isObj(obj))
    {
        const bool legacy = ssvuj::getExtr<bool>(obj, "legacy", true);
        if (legacy)
        {
            const int index = ssvuj::getExtr<int>(obj, "index", 0);
            return CapColor{CapColorMode::ByIndex{index}};
        }
        else
        {
            return CapColor{
                ColorData{false,
                          ssvuj::getExtr<bool>(obj, "dynamic", false),
                          ssvuj::getExtr<bool>(obj, "dynamic_offset", false),
                          ssvuj::getExtr<float>(obj, "dynamic_darkness", 1.f),
                          ssvuj::getExtr<float>(obj, "hue_shift", 0.f),
                          ssvuj::getExtr<float>(obj, "offset", 0.f),
                          ssvuj::getExtr<sf::Color>(obj, "value", sf::Color::White),
                          hg::pulse_from_json(obj)}};
        }
    }

    // Fallback case:
    return CapColor{CapColorMode::ByIndex{0}};
}

} // namespace hg
