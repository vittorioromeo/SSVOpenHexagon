// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#ifdef SSVOH_PRODUCE_LUA_METADATA
#include "SSVOpenHexagon/Utils/ArgExtractor.hpp"
#endif

#include "SSVOpenHexagon/Utils/TypeWrapper.hpp"

#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <tuple>
#include <cstddef>

namespace hg::Utils {

class LuaMetadata;

class LuaMetadataProxy
{
private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
    [[maybe_unused]] LuaMetadata& luaMetadata;
    [[maybe_unused]] std::string name;
    [[maybe_unused]] std::string (*erasedRet)(LuaMetadataProxy*);
    [[maybe_unused]] std::string (*erasedArgs)(LuaMetadataProxy*);
#pragma GCC diagnostic pop

    std::string docs;
    std::vector<std::string> argNames;

    template <typename T>
    [[nodiscard]] static const char* typeToStr(TypeWrapper<T>) noexcept;

    template <typename... Ts>
    [[nodiscard]] static std::string typeToStr(TypeWrapper<std::tuple<Ts...>>);

    template <typename FOp>
    [[nodiscard]] static std::string makeArgsString(
        [[maybe_unused]] LuaMetadataProxy* self)
    {
#ifdef SSVOH_PRODUCE_LUA_METADATA
        using AE = Utils::ArgExtractor<FOp>;

        if constexpr(AE::numArgs == 0)
        {
            return "";
        }
        else if constexpr(AE::numArgs == 1)
        {
            std::string res;

            res += typeToStr(
                TypeWrapper<std::decay_t<typename AE::template NthArg<0>>>{});

            res += ' ';
            res += self->argNames.at(0);

            return res;
        }
        else
        {
            std::string res;

            [&]<std::size_t... Is>(std::index_sequence<Is...>)
            {
                ((                                                           //
                     res += typeToStr(TypeWrapper<
                         std::decay_t<typename AE::template NthArg<Is>>>{}), //
                     res += ' ',                                             //
                     res += self->argNames.at(Is),                           //
                     res += ", "),
                    ...);
            }(std::make_index_sequence<AE::numArgs>{});

            res.pop_back();
            res.pop_back();

            return res;
        }
#else
        return "";
#endif
    }

    [[nodiscard]] std::string resolveArgNames(const std::string& docs);

    template <typename Ret>
    [[nodiscard]] static std::string makeErasedRet(LuaMetadataProxy*)
    {
#ifdef SSVOH_PRODUCE_LUA_METADATA
        return typeToStr(TypeWrapper<std::decay_t<Ret>>{});
#else
        return "";
#endif
    }

public:
#ifdef SSVOH_PRODUCE_LUA_METADATA
    template <typename F, typename FOp = decltype(&std::decay_t<F>::operator())>
    explicit LuaMetadataProxy(
        TypeWrapper<F>, LuaMetadata& mLuaMetadata, const std::string& mName)
        : luaMetadata{mLuaMetadata},
          name{mName},
          erasedRet{&makeErasedRet<typename Utils::ArgExtractor<FOp>::Return>},
          erasedArgs{&makeArgsString<FOp>}
    {}
#else
    template <typename F>
    explicit LuaMetadataProxy(TypeWrapper<F>, LuaMetadata& mLuaMetadata,
        [[maybe_unused]] const std::string& mName)
        : luaMetadata{mLuaMetadata},
          name{""},
          erasedRet{nullptr},
          erasedArgs{nullptr}
    {}
#endif

    ~LuaMetadataProxy();

    LuaMetadataProxy& arg(const std::string& mArgName);
    LuaMetadataProxy& doc(const std::string& mDocs);
};

} // namespace hg::Utils
