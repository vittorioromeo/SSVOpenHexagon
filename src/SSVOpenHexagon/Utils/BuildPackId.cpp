// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/BuildPackId.hpp"

#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"

#include <string>

namespace hg::Utils {

[[nodiscard]] std::string buildPackId(const std::string& packDisambiguator,
    const std::string& packAuthor, const std::string& packName,
    const int packVersion)
{
    SSVOH_ASSERT(!packDisambiguator.empty());
    SSVOH_ASSERT(!packAuthor.empty());
    SSVOH_ASSERT(!packName.empty());
    SSVOH_ASSERT(packVersion > 0);

    const auto spaceToUnderscore = [](std::string x)
    {
        for(char& c : x)
        {
            if(c == ' ' || c == '\n' || c == '\t')
            {
                c = '_';
            }
        }

        return x;
    };

    return Utils::concat(                     //
        spaceToUnderscore(packDisambiguator), //
        '_',                                  //
        spaceToUnderscore(packAuthor),        //
        '_',                                  //
        spaceToUnderscore(packName),          //
        '_',                                  //
        packVersion                           //
    );
}

} // namespace hg::Utils
