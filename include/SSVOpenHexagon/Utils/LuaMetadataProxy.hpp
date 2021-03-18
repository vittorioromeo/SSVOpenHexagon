// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/ArgExtractor.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadata.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <tuple>

namespace hg::Utils
{

template <typename>
struct TypeWrapper
{
};

class LuaMetadataProxy
{
private:
    LuaMetadata& luaMetadata;
    std::string name;
    std::string (*erasedRet)(LuaMetadataProxy*);
    std::string (*erasedArgs)(LuaMetadataProxy*);
    std::string docs;
    std::vector<std::string> argNames;

    template <typename... Ts>
    [[nodiscard]] static std::string typeToStr(TypeWrapper<std::tuple<Ts...>>) noexcept
    {
        std::string result;

        result += "tuple<";
        ((result += typeToStr(TypeWrapper<Ts>{})), ...);
        result += ">";

        return result;
    }

    template <typename T>
    [[nodiscard]] constexpr static const char* typeToStr(TypeWrapper<T>) noexcept
    {
        if constexpr(std::is_same_v<T, void>)
        {
            return "void";
        }
        else if constexpr(std::is_same_v<T, bool>)
        {
            return "bool";
        }
        else if constexpr(std::is_same_v<T, int>)
        {
            return "int";
        }
        else if constexpr(std::is_same_v<T, float>)
        {
            return "float";
        }
        else if constexpr(std::is_same_v<T, double>)
        {
            return "double";
        }
        else if constexpr(std::is_same_v<T, std::string>)
        {
            return "string";
        }
        else if constexpr(std::is_same_v<T, unsigned int>)
        {
            return "unsigned int";
        }
        else if constexpr(std::is_same_v<T, long>)
        {
            return "long";
        }
        else if constexpr(std::is_same_v<T, unsigned long>)
        {
            return "unsigned long";
        }
        else if constexpr(std::is_same_v<T, long long>)
        {
            return "long long";
        }
        else if constexpr(std::is_same_v<T, unsigned long long>)
        {
            return "unsigned long long";
        }
        else if constexpr(std::is_same_v<T, std::size_t>)
        {
            return "size_t";
        }
        else
        {
            struct fail;
            return fail{};
        }
    }

    template <typename ArgT>
    static void addTypeToStr(std::vector<std::string>& types)
    {
        types.emplace_back(typeToStr(TypeWrapper<std::decay_t<ArgT>>{}));
    }

    template <typename F>
    [[nodiscard]] static std::string makeArgsString(LuaMetadataProxy* self)
    {
        using AE = Utils::ArgExtractor<decltype(&F::operator())>;

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

    [[nodiscard]] std::string resolveArgNames(const std::string& docs)
    {
        std::size_t argNameSize = 0;
        for(const auto& argName : argNames)
        {
            argNameSize += argName.size() + 4;
        }

        std::string result;
        result.reserve(docs.size() + argNameSize);

        for(std::size_t i = 0; i < docs.size(); ++i)
        {
            if(docs[i] != '$')
            {
                result += docs[i];
                continue;
            }

            const std::size_t index = docs.at(i + 1) - '0';
            result += argNames.at(index);
            ++i;
        }

        return result;
    }

public:
    template <typename F>
    explicit LuaMetadataProxy(
        F&&, LuaMetadata& mLuaMetadata, const std::string& mName)
        : luaMetadata{mLuaMetadata}, name{mName},
          erasedRet{[](LuaMetadataProxy*) -> std::string {
              using AE =
                  Utils::ArgExtractor<decltype(&std::decay_t<F>::operator())>;

              return typeToStr(TypeWrapper<std::decay_t<typename AE::Return>>{});
          }},
          erasedArgs{[](LuaMetadataProxy* self) {
              return makeArgsString<std::decay_t<F>>(self);
          }}
    {
    }

    ~LuaMetadataProxy()
    {
        try
        {
            luaMetadata.addFnEntry((*erasedRet)(this), name,
                (*erasedArgs)(this), resolveArgNames(docs));
        }
        catch(const std::exception& e)
        {
            ssvu::lo("LuaMetadataProxy")
                << "Failed to generate documentation for " << name << ": "
                << e.what() << '\n';
        }
        catch(...)
        {
            ssvu::lo("LuaMetadataProxy")
                << "Failed to generate documentation for " << name << '\n';
        }
    }

    LuaMetadataProxy& arg(const std::string& mArgName)
    {
        argNames.emplace_back(mArgName);
        return *this;
    }

    LuaMetadataProxy& doc(const std::string& mDocs)
    {
        docs = mDocs;
        return *this;
    }
};

} // namespace hg::Utils
