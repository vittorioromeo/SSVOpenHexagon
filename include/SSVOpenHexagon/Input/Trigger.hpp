// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Input/Combo.hpp"

#include <initializer_list>
#include <vector>

namespace ssvs::Input
{

class Trigger
{
private:
    std::vector<Combo> combos;

public:
    Trigger() = default;

    Trigger(const std::initializer_list<Combo>& initCombos) noexcept : combos{initCombos}
    {
    }

    [[nodiscard]] bool operator==(const Trigger& rhs) const noexcept
    {
        return combos == rhs.combos;
    }

    [[nodiscard]] bool operator!=(const Trigger& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    [[nodiscard]] auto& getCombos() noexcept
    {
        return combos;
    }

    [[nodiscard]] const auto& getCombos() const noexcept
    {
        return combos;
    }
};

} // namespace ssvs::Input
