// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg
{

template <typename>
class Delegate;

template <typename TReturn, typename... TArgs>
class Delegate<TReturn(TArgs...)>
{
private:
    using FuncType = sf::base::FixedFunction<TReturn(TArgs...), 64>;
    sf::base::Vector<FuncType> funcs;

public:
    Delegate& operator+=(auto mFunc)
    {
        funcs.emplaceBack(mFunc);
        return *this;
    }

    inline void operator()(TArgs... mArgs)
    {
        for (auto& f : funcs)
            f(mArgs...);
    }

    /// @brief Clears all the functions from the delegate.
    inline void clear() noexcept
    {
        funcs.clear();
    }

    /// @brief Returns whether the delegate has any callback or not.
    inline bool isEmpty() const noexcept
    {
        return funcs.empty();
    }
};

} // namespace hg
