// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstddef>
#include <string_view>

namespace hg::Utils {

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
    static constexpr std::size_t NUM_CATEGORIES = 11;
    std::array<std::vector<FnEntry>, NUM_CATEGORIES> fnEntries;
    static constexpr std::array<std::string_view, NUM_CATEGORIES>
        prefixCategories{"u_", "a_", "t_", "e_", "l_", "s_", "w_", "cw_", "ct_",
            "shdr_", "Miscellaneous"};

    [[nodiscard]] std::size_t getCategoryIndexFromName(
        const std::string_view fnName);

public:
    static constexpr std::array<std::string_view, NUM_CATEGORIES> prefixHeaders{
        "## Utility Functions (u_)\n\n"

        "Below are the utility functions, which can be identified with the "
        "\"u_\" prefix. These are overall functions that either help utilize "
        "the game engine, be incredibly beneficial to pack developers, or help "
        "simplify complex calculations.",

        "## Audio Functions (a_)\n\n"

        "Below are the audio functions, which can be identified with the "
        "\"a_\" prefix. This library is capable of controlling everything "
        "audio with the game, including the level music and sounds. Set the "
        "music, play a specific sound, all of this is in the domain of "
        "this prefix.",

        "## Main Timeline Functions (t_)\n\n"

        "Below are the main timeline functions, which can be identified "
        "with the \"t_\" prefix. These are functions that have effects on "
        "the main timeline itself, but they mainly consist of waiting "
        "functions. Using these functions helps time out your patterns and "
        "space them in the first place.",

        "## Event Timeline Functions (e_)\n\n"

        "Below are the event timeline functions, which can be identified "
        "with the \"e_\" prefix. These are functions "
        "that are similar to the Main Timeline functions, but they instead "
        "are for the event timeline as opposed to "
        "the main timeline. Use these functions to help set up basic "
        "events for the game, such as handling messages. Use ``e_eval`` to "
        "do more advanced events.",

        "## Level Functions (l_)\n\n"

        "Below are the level functions, which can be identified with the "
        "\"l_\" prefix. These are functions that "
        "have a role in altering the level mechanics themselves, including "
        "all level properties and attributes. "
        "These typically get called en masse in `onInit` to initialize "
        "properties.",

        "## Style Functions (s_)\n\n"

        "Below are the style functions, which can be identified with the "
        "\"s_\" prefix. These are functions that "
        "have a role in altering the attributes of the current style that "
        "is on the level. Style attributes, unlike "
        "level attributes, do not get initialized in Lua and rather are "
        "premade in a JSON file (but this is subject "
        "to change).",

        "## Wall Functions (w_)\n\n"

        "Below are the basic wall functions, which can be identified with "
        "the \"w_\" prefix. These are the functions "
        "sole responsible for wall creation in the levels. There are a "
        "variety of walls that can be made with different "
        "degrees of complexity, all of which can be used to construct your "
        "own patterns.",

        "## Custom Wall Functions (cw_)\n\n"

        "Below are the custom wall functions, which can be identified with the "
        "\"cw_\" prefix. These are functions with foundations of "
        "[Object-oriented "
        "programming](https://en.wikipedia.org/wiki/"
        "Object-oriented_programming) to allow pack developers to customize "
        "individual walls and their properties and make the most out of them.",

        "## Custom Timeline Functions (ct_)\n\n"

        "Below are the custom timeline functions, which can be identified with "
        "the \"ct_\" prefix. These are functions with foundations of "
        "[Object-oriented "
        "programming](https://en.wikipedia.org/wiki/"
        "Object-oriented_programming) to allow pack developers to create and "
        "manage independent timelines.",

        "## Shader Functions (shdr_)\n\n"

        "Below are the shader functions, which can be identified with the "
        "\"shdr_\" prefix. These are functions that enable graphical "
        "manipulation of the rendered game scene via GLSL shaders.",

        "## Miscellaneous Functions\n\n"

        "Below are the miscellaneous functions, which can have a variable "
        "prefix or no prefix at all. These are other functions "
        "that are listed that cannot qualify for one of the above eight "
        "categories and achieve some other purpose, with some "
        "functions not meant to be used by pack developers at all."};

    void addFnEntry(const std::string& fnRet, const std::string& fnName,
        const std::string& fnArgs, const std::string& fnDocs);

    [[nodiscard]] std::size_t getNumCategories() const noexcept;

    template <typename F>
    void forFnEntries([[maybe_unused]] F&& f,
        [[maybe_unused]] const std::size_t categoryIndex)
    {
#ifdef SSVOH_PRODUCE_LUA_METADATA
        for(const auto& [ret, name, args, docs] : fnEntries.at(categoryIndex))
        {
            f(ret, name, args, docs);
        }
#endif
    }
};

} // namespace hg::Utils
