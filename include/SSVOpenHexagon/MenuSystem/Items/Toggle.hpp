#pragma once

#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"

#include "SFML/Base/FixedFunction.hpp"

#include <string>
#include <utility>

namespace ssvms
{
class Menu;
class Category;

namespace Items
{
class Toggle final : public ItemBase
{
private:
    mutable sf::base::FixedFunction<bool(), 64> predicate;
    sf::base::FixedFunction<void(), 128>        activateAction;
    sf::base::FixedFunction<void(), 128>        deactivateAction;

public:
    Toggle(Menu&                                mMenu,
           Category&                            mCategory,
           const std::string&                   mName,
           sf::base::FixedFunction<bool(), 64>  mActivatedPredicate,
           sf::base::FixedFunction<void(), 128> mActivateAction,
           sf::base::FixedFunction<void(), 128> mDeactivateAction) :
        ItemBase{mMenu, mCategory, mName},
        predicate{std::move(mActivatedPredicate)},
        activateAction{std::move(mActivateAction)},
        deactivateAction{std::move(mDeactivateAction)}
    {
        increasable = true;
    }

    template <typename TFuncGet, typename TFuncSet>
    Toggle(Menu& mMenu, Category& mCategory, const std::string& mName, TFuncGet mFuncGet, TFuncSet mFuncSet) :
        ItemBase{mMenu, mCategory, mName},
        predicate{[=] { return mFuncGet(); }},
        activateAction{[=] { mFuncSet(true); }},
        deactivateAction{[=] { mFuncSet(false); }}
    {
        increasable = true;
    }

    Toggle(Menu& mMenu, Category& mCategory, const std::string& mName, bool& mBool) :
        ItemBase{mMenu, mCategory, mName},
        predicate{[&mBool] { return mBool; }},
        activateAction{[&mBool] { mBool = true; }},
        deactivateAction{[&mBool] { mBool = false; }}
    {
        increasable = true;
    }

    void exec() override
    {
        predicate() ? deactivateAction() : activateAction();
    }

    void increase() override
    {
        exec();
    }
    void decrease() override
    {
        exec();
    }

    [[nodiscard]] std::string getName() const override
    {
        return predicate() ? name + ": on" : name + ": off";
    }
};
} // namespace Items
} // namespace ssvms
