#pragma once

#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"

#include <SFML/Base/FixedFunction.hpp>
#include <SSVUtils/Core/String/ToStr.hpp>
#include <SSVUtils/Core/Utils/Math.hpp>

#include <string>
#include <utility>

namespace ssvms
{
class Menu;
class Category;

namespace Items
{
class Slider final : public ItemBase
{
private:
    using ValueGetter = sf::base::FixedFunction<std::string(), 128>;

    mutable ValueGetter valueGetter;
    sf::base::FixedFunction<void(), 128> increaseAction;
    sf::base::FixedFunction<void(), 128> decreaseAction;

public:
    Slider(Menu& mMenu, Category& mCategory, const std::string& mName,
        ValueGetter mValueGetter,
        sf::base::FixedFunction<void(), 128> mIncreaseAction,
        sf::base::FixedFunction<void(), 128> mDecreaseAction)
        : ItemBase{mMenu, mCategory, mName},
          valueGetter{std::move(mValueGetter)},
          increaseAction{std::move(mIncreaseAction)},
          decreaseAction{std::move(mDecreaseAction)}
    {
        increasable = true;
    }

    template <typename T, typename TFuncGet, typename TFuncSet>
    Slider(Menu& mMenu, Category& mCategory, const std::string& mName,
        TFuncGet mFuncGet, TFuncSet mFuncSet, T mMin, T mMax, T mIncrement)
        : ItemBase{mMenu, mCategory, mName},
          valueGetter{[=]
              {
                  return ssvu::toStr(mFuncGet());
              }},
          increaseAction{[=]
              {
                  mFuncSet(ssvu::getClamped(
                      mFuncGet() + mIncrement, mMin, mMax));
              }},
          decreaseAction{[=]
              {
                  mFuncSet(ssvu::getClamped(
                      mFuncGet() - mIncrement, mMin, mMax));
              }}
    {
        increasable = true;
    }

    void increase() override { increaseAction(); }
    void decrease() override { decreaseAction(); }

    [[nodiscard]] std::string getName() const override
    {
        return name + ": " + valueGetter();
    }
};
} // namespace Items
} // namespace ssvms
