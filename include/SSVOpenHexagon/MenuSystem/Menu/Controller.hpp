#pragma once

#include "SFML/Base/FixedFunction.hpp"

#include <vector>

namespace ssvms
{
class ItemBase;

namespace Impl
{
class Controller
{
private:
    struct Pair
    {
        ItemBase*                           fst;
        sf::base::FixedFunction<bool(), 64> snd;
    };

    std::vector<Pair> enableWhenPairs;

public:
    void enableItemWhen(ItemBase& mItem, sf::base::FixedFunction<bool(), 64> mPred)
    {
        enableWhenPairs.push_back(Pair{&mItem, std::move(mPred)});
    }

    void update()
    {
        for (Pair& p : enableWhenPairs)
        {
            p.fst->setEnabled(p.snd());
        }
    }
};
} // namespace Impl
} // namespace ssvms
