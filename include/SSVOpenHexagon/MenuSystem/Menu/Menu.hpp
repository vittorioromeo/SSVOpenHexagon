#pragma once

#include "SSVOpenHexagon/MenuSystem/Menu/Category.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/Controller.hpp"

#include <SFML/Base/FixedFunction.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <stack>
#include <string>
#include <vector>

namespace ssvms
{
class Menu
{
    friend Category;

private:
    using Controller = Impl::Controller;

    std::vector<sf::base::UniquePtr<Category>> categories;
    Category* category{nullptr};
    std::stack<Category*> lastCategories;
    Controller controller;

public:
    [[nodiscard]] auto& createCategory(const std::string& mName)
    {
        categories.push_back(sf::base::makeUnique<Category>(*this, mName));

        Category& result{*categories.back()};
        if(category == nullptr)
        {
            setCategory(result);
        }

        return result;
    }

    void setCategory(Category& mCategory)
    {
        lastCategories.emplace(&mCategory);
        category = &mCategory;
    }

    void clear() noexcept
    {
        categories.clear();
        category = nullptr;
        lastCategories = {};
    }

    void update() { controller.update(); }

    [[nodiscard]] bool canGoBack() const { return lastCategories.size() > 1; }
    [[nodiscard]] Category& getCategory() const { return *category; }
    [[nodiscard]] auto& getCategories() const noexcept { return categories; }

    [[nodiscard]] auto& getCategoryByName(const std::string& mName) const
    {
        for(const sf::base::UniquePtr<Category>& c : categories)
        {
            if(c->getName() == mName)
            {
                return *c;
            }
        }

        return *category;
    }

    [[nodiscard]] ItemBase& getItem() const { return category->getItem(); }

    [[nodiscard]] std::vector<sf::base::UniquePtr<ItemBase>>& getItems()
    {
        return category->getItems();
    }

    [[nodiscard]] const std::vector<sf::base::UniquePtr<ItemBase>>& getItems()
        const
    {
        return category->getItems();
    }

    [[nodiscard]] int getIdx() const { return category->getIdx(); }
    [[nodiscard]] auto& getMenuController() noexcept { return controller; }

    void goBack()
    {
        lastCategories.pop();
        category = lastCategories.top();
    }

    void next() { category->next(); }
    void previous() { category->previous(); }

    void exec()
    {
        if(getItem().isEnabled())
        {
            getItem().exec();
        }
    }

    void increase()
    {
        if(getItem().isEnabled())
        {
            getItem().increase();
        }
    }

    void decrease()
    {
        if(getItem().isEnabled())
        {
            getItem().decrease();
        }
    }
};

inline ItemBase& operator|(
    ItemBase& mLhs, sf::base::FixedFunction<bool(), 64> mRhs)
{
    mLhs.getMenu().getMenuController().enableItemWhen(mLhs, std::move(mRhs));
    return mLhs;
}
} // namespace ssvms
