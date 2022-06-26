// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LuaMetadata.hpp"

#include <string>
#include <cstddef>
#include <string_view>

namespace hg::Utils {

[[nodiscard]] std::size_t LuaMetadata::getCategoryIndexFromName(
    const std::string_view fnName)
{
    const std::size_t underscoreIndex = fnName.find("_");
    if(underscoreIndex == std::string::npos)
    {
        // Return the last index: the miscellaneous index.
        return NUM_CATEGORIES - 1;
    }

    const std::string_view prefix = fnName.substr(0, underscoreIndex + 1);

    // Find the category it should be placed in, otherwise it'll be
    // considered Miscellaneous
    for(std::size_t i = 0; i < prefixCategories.size() - 1; ++i)
    {
        if(prefix == prefixCategories[i])
        {
            return i;
        }
    }

    return NUM_CATEGORIES - 1;
}

void LuaMetadata::addFnEntry(const std::string& fnRet,
    const std::string& fnName, const std::string& fnArgs,
    const std::string& fnDocs)
{
    const std::size_t categoryIndex = getCategoryIndexFromName(fnName);

    fnEntries.at(categoryIndex)
        .push_back(FnEntry{fnRet, fnName, fnArgs, fnDocs});
}

[[nodiscard]] std::size_t LuaMetadata::getNumCategories() const noexcept
{
    return NUM_CATEGORIES;
}

} // namespace hg::Utils
