// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/ArgExtractor.hpp"
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
    LuaMetadata& luaMetadata;
    std::string name;
    std::string (*erasedRet)(LuaMetadataProxy*);
    std::string (*erasedArgs)(LuaMetadataProxy*);
    std::string docs;
    std::vector<std::string> argNames;

    template <typename T>
    [[nodiscard]] static const char* typeToStr(TypeWrapper<T>) noexcept;

    template <typename... Ts>
    [[nodiscard]] static std::string typeToStr(TypeWrapper<std::tuple<Ts...>>);

    template <typename FOp>
    [[nodiscard]] static std::string makeArgsString(LuaMetadataProxy* self)
    {
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
                (( //
                     res += typeToStr(TypeWrapper<
                         std::decay_t<typename AE::template NthArg<Is>>>{}), //
                     res += ' ',                                             //
                     res += self->argNames.at(Is),                           //
                     res += ", "),
                    ...);
            }
            (std::make_index_sequence<AE::numArgs>{});

            res.pop_back();
            res.pop_back();

            return res;
        }
    }

    [[nodiscard]] std::string resolveArgNames(const std::string& docs);

    template <typename Ret>
    [[nodiscard]] static std::string makeErasedRet(LuaMetadataProxy*)
    {
        return typeToStr(TypeWrapper<std::decay_t<Ret>>{});
    }

public:
    template <typename F, typename FOp = decltype(&std::decay_t<F>::operator())>
    explicit LuaMetadataProxy(
        TypeWrapper<F>, LuaMetadata& mLuaMetadata, const std::string& mName)
        : luaMetadata{mLuaMetadata},
          name{mName},
          erasedRet{&makeErasedRet<typename Utils::ArgExtractor<FOp>::Return>},
          erasedArgs{&makeArgsString<FOp>}
    {}

    ~LuaMetadataProxy();

    LuaMetadataProxy& arg(const std::string& mArgName);
    LuaMetadataProxy& doc(const std::string& mDocs);
};

} // namespace hg::Utils
