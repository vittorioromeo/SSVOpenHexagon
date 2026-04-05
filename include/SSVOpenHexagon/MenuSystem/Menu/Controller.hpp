#pragma once

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/Vector.hpp"

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

    sf::base::Vector<Pair> enableWhenPairs;

public:
    void enableItemWhen(ItemBase& mItem, sf::base::FixedFunction<bool(), 64> mPred)
    {
        enableWhenPairs.pushBack(Pair{&mItem, std::move(mPred)});
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
