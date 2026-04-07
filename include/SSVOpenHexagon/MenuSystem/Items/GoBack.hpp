#pragma once

#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/Menu.hpp"

#include "SFML/Base/String.hpp"


namespace ssvms
{
class Category;

namespace Items
{
struct GoBack final : public ItemBase
{
    GoBack(Menu& mMenu, Category& mCategory, const sf::base::String& mName) : ItemBase{mMenu, mCategory, mName}
    {
    }

    void exec() override
    {
        menu.goBack();
    }
};
} // namespace Items
} // namespace ssvms
