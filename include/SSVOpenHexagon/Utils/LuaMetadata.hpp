// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <vector>

namespace hg::Utils
{

class LuaMetadata
{
private:
    struct FnEntry
    {
        std::string fnRet;
        std::string fnName;
        std::string fnArgs;
        std::string fnDocs;
    };

    std::vector<FnEntry> fnEntries;

public:
    void addFnEntry(const std::string& fnRet, const std::string& fnName,
        const std::string& fnArgs, const std::string& fnDocs)
    {
        fnEntries.push_back(FnEntry{fnRet, fnName, fnArgs, fnDocs});
    }

    template <typename F>
    void forFnEntries(F&& f)
    {
        for(const auto& [ret, name, args, docs] : fnEntries)
        {
            f(ret, name, args, docs);
        }
    }
};

} // namespace hg::Utils
