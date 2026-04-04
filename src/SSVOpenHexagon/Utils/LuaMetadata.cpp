// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LuaMetadata.hpp"

#include <SFML/Base/StringView.hpp>
#include <string>

namespace hg::Utils
{

[[nodiscard]] sf::base::SizeT LuaMetadata::getCategoryIndexFromName(const sf::base::StringView fnName)
{
    const sf::base::SizeT underscoreIndex = fnName.find("_");
    if (underscoreIndex == std::string::npos)
    {
        // Return the last index: the miscellaneous index.
        return NUM_CATEGORIES - 1;
    }

    const sf::base::StringView prefix = fnName.substrByPosLen(0, underscoreIndex + 1);

    // Find the category it should be placed in, otherwise it'll be
    // considered Miscellaneous
    for (sf::base::SizeT i = 0; i < prefixCategories.size() - 1; ++i)
    {
        if (prefix == prefixCategories[i])
        {
            return i;
        }
    }

    return NUM_CATEGORIES - 1;
}

void LuaMetadata::addFnEntry(const sf::base::String& fnRet,
                             const sf::base::String& fnName,
                             const sf::base::String& fnArgs,
                             const sf::base::String& fnDocs)
{
    const sf::base::SizeT categoryIndex = getCategoryIndexFromName(fnName);

    fnEntries[categoryIndex].emplaceBack(fnRet, fnName, fnArgs, fnDocs);
}

[[nodiscard]] sf::base::SizeT LuaMetadata::getNumCategories() const noexcept
{
    return NUM_CATEGORIES;
}

} // namespace hg::Utils
