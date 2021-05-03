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

    template <typename ArgT>
    static void addTypeToStr(std::vector<std::string>& types)
    {
        types.emplace_back(typeToStr(TypeWrapper<std::decay_t<ArgT>>{}));
    }

    template <typename FOp>
    [[nodiscard]] static std::string makeArgsString(LuaMetadataProxy* self)
    {
        using AE = Utils::ArgExtractor<FOp>;

        std::vector<std::string> types;

        [&]<std::size_t... Is>(std::index_sequence<Is...>)
        {
            (addTypeToStr<typename AE::template NthArg<Is>>(types), ...);
        }
        (std::make_index_sequence<AE::numArgs>{});

        if(types.empty())
        {
            return "";
        }

        std::string res = types.at(0) + " " + self->argNames.at(0);

        if(types.size() == 1)
        {
            return res;
        }

        for(std::size_t i = 1; i < types.size(); ++i)
        {
            res += ", ";
            res += types.at(i);
            res += " ";
            res += self->argNames.at(i);
        }

        return res;
    }

    [[nodiscard]] std::string resolveArgNames(const std::string& docs);

    template <typename FOp>
    [[nodiscard]] static std::string makeErasedRet(LuaMetadataProxy*)
    {
        using AE = Utils::ArgExtractor<FOp>;
        return typeToStr(TypeWrapper<std::decay_t<typename AE::Return>>{});
    }

public:
    template <typename F, typename FOp = decltype(&std::decay_t<F>::operator())>
    explicit LuaMetadataProxy(
        TypeWrapper<F>, LuaMetadata& mLuaMetadata, const std::string& mName)
        : luaMetadata{mLuaMetadata},
          name{mName},
          erasedRet{&makeErasedRet<FOp>},
          erasedArgs{&makeArgsString<FOp>}
    {}

    ~LuaMetadataProxy();

    LuaMetadataProxy& arg(const std::string& mArgName);
    LuaMetadataProxy& doc(const std::string& mDocs);
};

} // namespace hg::Utils
