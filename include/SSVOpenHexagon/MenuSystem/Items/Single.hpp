#pragma once

#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"

#include <SFML/Base/FixedFunction.hpp>

#include <string>

namespace ssvms
{
class Menu;
class Category;

namespace Items
{
class Single final : public ItemBase
{
private:
    sf::base::FixedFunction<void(), 128> action;

public:
    Single(Menu& mMenu, Category& mCategory, const std::string& mName,
        sf::base::FixedFunction<void(), 128> mAction)
        : ItemBase{mMenu, mCategory, mName}, action{std::move(mAction)}
    {}

    void exec() override { action(); }
};
} // namespace Items
} // namespace ssvms
