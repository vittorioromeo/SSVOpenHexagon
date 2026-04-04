#pragma once

#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/Menu.hpp"

#include <string>

namespace ssvms
{
class Category;

namespace Items
{
class Goto final : public ItemBase
{
private:
    Category& target;

public:
    Goto(Menu& mMenu, Category& mCategory, const std::string& mName,
        Category& mTarget)
        : ItemBase{mMenu, mCategory, mName}, target{mTarget}
    {}

    void exec() override { menu.setCategory(target); }
};
} // namespace Items
} // namespace ssvms
