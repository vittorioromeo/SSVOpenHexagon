#pragma once

#include "SFML/Base/String.hpp"


namespace ssvms
{
class Menu;
class Category;

class ItemBase
{
protected:
    Menu&            menu;
    Category&        category;
    sf::base::String name;
    bool             enabled{true};
    bool             increasable{false};
    float            offset{0.f};

public:
    ItemBase(Menu& mMenu, Category& mCategory, const sf::base::String& mName) :
        menu{mMenu},
        category{mCategory},
        name{mName}
    {
    }

    virtual ~ItemBase() = default;

    virtual void exec()
    {
    }
    virtual void increase()
    {
    }
    virtual void decrease()
    {
    }

    void setEnabled(bool mEnabled) noexcept
    {
        enabled = mEnabled;
    }

    [[nodiscard]] auto& getMenu() const noexcept
    {
        return menu;
    }
    [[nodiscard]] virtual sf::base::String getName() const
    {
        return name;
    }
    [[nodiscard]] bool isEnabled() const noexcept
    {
        return enabled;
    }
    [[nodiscard]] bool canIncrease() const noexcept
    {
        return increasable;
    }
    [[nodiscard]] float& getOffset() noexcept
    {
        return offset;
    }
};
} // namespace ssvms
