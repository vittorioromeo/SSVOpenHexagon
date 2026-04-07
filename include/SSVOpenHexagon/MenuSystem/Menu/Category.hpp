#pragma once

#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <SSVUtils/Core/Common/Casts.hpp>
#include <algorithm>
#include <utility>

namespace ssvms
{
class Menu;
class ItemBase;

class Category
{
    friend Menu;

private:
    Menu&                                           menu;
    sf::base::String                                name;
    sf::base::Vector<sf::base::UniquePtr<ItemBase>> items;
    int                                             index{0};
    float                                           offset{0.f};

    void wrapIndex()
    {
        if (index > ssvu::toInt(items.size() - 1))
        {
            index = 0;
        }
        else if (index < 0)
        {
            index = items.size() - 1;
        }
    }

public:
    Category(Menu& mMenu, const sf::base::String& mName) : menu{mMenu}, name{mName}
    {
    }

    template <typename T, typename... TArgs>
    T& create(const sf::base::String& mName, TArgs&&... mArgs)
    {
        items.pushBack(sf::base::makeUnique<T>(menu, *this, mName, std::forward<TArgs>(mArgs)...));
        return static_cast<T&>(*items.back());
    }

    void remove()
    {
        items.erase(items.begin() + index);
        index = std::min(index, int(items.size()) - 1);
    }

    void sortByName()
    {
        const auto sortFunc = [](const sf::base::UniquePtr<ItemBase>& a, const sf::base::UniquePtr<ItemBase>& b)
        { return a->getName() < b->getName(); };

        std::sort(items.begin(), items.end(), sortFunc);
    }

    void next()
    {
        ++index;
        wrapIndex();
    }

    void previous()
    {
        --index;
        wrapIndex();
    }

    [[nodiscard]] const auto& getName() const noexcept
    {
        return name;
    }
    [[nodiscard]] auto& getItem() const
    {
        return *items[index];
    }
    [[nodiscard]] auto& getItems() noexcept
    {
        return items;
    }
    [[nodiscard]] const auto& getItems() const noexcept
    {
        return items;
    }
    [[nodiscard]] int getIdx() const noexcept
    {
        return index;
    }
    [[nodiscard]] float& getOffset() noexcept
    {
        return offset;
    }
};
} // namespace ssvms
