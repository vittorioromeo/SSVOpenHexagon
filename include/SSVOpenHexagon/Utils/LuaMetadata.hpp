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
    
    // There are 9 categories.
    constexpr static std::size_t NUM_CATEGORIES = 9;
    std::array<std::vector<FnEntry>, NUM_CATEGORIES> fnEntries;
    constexpr static std::array<std::string_view, NUM_CATEGORIES> prefixCategories = {"u_", "m_", "t_", "e_", "l_", "s_", "w_", "cw_", "Miscellaneous"};

    [[nodiscard]] std::size_t getCategoryIndexFromName(const std::string_view fnName) 
    {
        const std::size_t underscoreIndex = fnName.find("_");
        if (underscoreIndex == std::string::npos) {
            return NUM_CATEGORIES - 1; // Return the last index: the miscellaneous index.
        }
        const std::string_view prefix = fnName.substr(0, underscoreIndex + 1);
        
        // Find the category it should be placed in, otherwise it'll be considered Miscellaneous
        for (std::size_t i = 0; i < prefixCategories.size() - 1; ++i) 
        {
            if (prefix == prefixCategories[i]) 
            {
                return i;
            }
        }
        return NUM_CATEGORIES - 1;
    }

public:

    std::array<std::string, NUM_CATEGORIES> prefixHeaders = 
        {"## Utility Functions (u_)\n\n"

         "Below are the utility functions, which can be identified with the \"u_\" prefix. These are overall functions "
         "that either help utilize the game engine, be incredibly beneficial to pack developers, or help simplify complex "
         "calculations.", 

         "## Message Functions (m_)\n\n"

         "Below are the message functions, which can be identified with the \"m_\" prefix. These functions are capable "
         "of displaying custom messages on the screen which can help pack developers communicate additional info or "
         "anything else useful (e.g Song Lyrics) to the players. For keeping track of statistics, please look at "
         "`l_addTracked`.", 

         "## Main Timeline Functions (t_)\n\n"
         
         "Below are the main timeline functions, which can be identified with the \"t_\" prefix. These are functions "
         "that have effects on the main timeline itself, but they mainly consist of waiting functions. Using these "
         "functions helps time out your patterns and space them in the first place.", 

         "## Event Timeline Functions (e_)\n\n"
         
         "Below are the event timeline functions, which can be identified with the \"e_\" prefix. These are functions "
         "that are similar to the Main Timeline functions, but they instead are for the event timeline as opposed to "
         "the main timeline. Use these functions to help set up basic events for the game. More advanced events must "
         "be done with pure Lua.", 
         
         "## Level Functions (l_)\n\n"
         
         "Below are the level functions, which can be identified with the \"l_\" prefix. These are functions that "
         "have a role in altering the level mechanics themselves, including all level properties and attributes. "
         "These typically get called en masse in `onInit` to initialize properties.", 

         "## Style Functions (s_)\n\n"
         
         "Below are the style functions, which can be identified with the \"s_\" prefix. These are functions that "
         "have a role in altering the attributes of the current style that is on the level. Style attributes, unlike "
         "level attributes, do not get initialized in Lua and rather are premade in a JSON file (but this is subject "
         "to change).",

         "## Wall Functions (w_)\n\n"
         
         "Below are the basic wall functions, which can be identified with the \"w_\" prefix. These are the functions "
         "sole responsible for wall creation in the levels. There are a variety of walls that can be made with different "
         "degrees of complexity, all of which can be used to construct your own patterns.", 

         "## Custom Wall Functions (cw_)\n\n"
         
         "Below are the custom wall functions, which can be identified with the \"cw_\" prefix. These are 2.0 exclusive "
         "functions with foundations of [Object-oriented programming](https://en.wikipedia.org/wiki/Object-oriented_programming) "
         "to allow pack developers to customize individual walls and their properties and make the most out of them. ", 
         
         "## Miscellaneous Functions\n\n"
         
         "Below are the miscellaneous functions, which can have a variable prefix or no prefix at all. These are other functions "
         "that are listed that cannot qualify for one of the above eight categories and achieve some other purpose, with some "
         "functions not meant to be used by pack developers at all."};

    void addFnEntry(const std::string& fnRet, const std::string& fnName,
        const std::string& fnArgs, const std::string& fnDocs)
    {
        const int category = getCategoryFromName(fnName);
        fnEntries.at(category).push_back(FnEntry{fnRet, fnName, fnArgs, fnDocs});
    }

    [[nodiscard]] std::size_t getNumCategories() {
        return NUM_CATEGORIES;
    }

    template <typename F>
    void forFnEntries(F&& f, const int category)
    {
        for(const auto& [ret, name, args, docs] : fnEntries.at(category))
        {
            f(ret, name, args, docs);
        }
    }
};

} // namespace hg::Utils
