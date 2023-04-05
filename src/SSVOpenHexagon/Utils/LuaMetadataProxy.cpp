// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LuaMetadataProxy.hpp"

#include "SSVOpenHexagon/Utils/LuaMetadata.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <tuple>
#include <cstddef>

namespace hg::Utils {

template <typename T>
[[nodiscard]] const char* LuaMetadataProxy::typeToStr(TypeWrapper<T>) noexcept
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
#define RETURN_T_STR(type)    \
    (std::is_same_v<T, type>) \
    {                         \
        return #type;         \
    }

    // clang-format off
         if constexpr RETURN_T_STR(void)
    else if constexpr RETURN_T_STR(bool)
    else if constexpr RETURN_T_STR(int)
    else if constexpr RETURN_T_STR(float)
    else if constexpr RETURN_T_STR(double)
    else if constexpr RETURN_T_STR(unsigned int)
    else if constexpr RETURN_T_STR(long)
    else if constexpr RETURN_T_STR(unsigned long)
    else if constexpr RETURN_T_STR(long long)
    else if constexpr RETURN_T_STR(unsigned long long)
    else if constexpr RETURN_T_STR(std::string)
    else
    {
        struct fail;
        return fail{};
    }
// clang-format on
#else
    return "";
#endif
}

#ifdef SSVOH_PRODUCE_LUA_METADATA
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<void>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<bool>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<int>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<float>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<double>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<unsigned int>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<long>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<unsigned long>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<long long>);
template const char* LuaMetadataProxy::typeToStr(
    TypeWrapper<unsigned long long>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<std::string>);
#endif

// ----------------------------------------------------------------------------

template <typename... Ts>
[[nodiscard]] std::string LuaMetadataProxy::typeToStr(
    TypeWrapper<std::tuple<Ts...>>)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    std::string result;

    result += "tuple<";
    if constexpr(sizeof...(Ts) > 0)
    {
        (((result += typeToStr(TypeWrapper<Ts>{})), result += ", "), ...);
        result.pop_back();
        result.pop_back();
    }
    result += ">";

    return result;
#else
    return "";
#endif
}

#ifdef SSVOH_PRODUCE_LUA_METADATA
template std::string LuaMetadataProxy::typeToStr(
    TypeWrapper<std::tuple<int, int, int, int>>);

template std::string LuaMetadataProxy::typeToStr(
    TypeWrapper<std::tuple<float, float>>);

template std::string LuaMetadataProxy::typeToStr(TypeWrapper<
    std::tuple<float, float, float, float, float, float, float, float>>);
#endif

// ----------------------------------------------------------------------------

[[nodiscard]] std::string LuaMetadataProxy::resolveArgNames(
    [[maybe_unused]] const std::string& docs)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
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

        ++i;

        std::size_t j = i;
        for(; j < docs.size(); ++j)
        {
            const char next = docs.at(j);
            if(next < '0' || next > '9')
            {
                break;
            }
        }

        // Range `[i, j)` is now the position of the argument.
        // Parse into integer.

        std::size_t indexAcc = 0;
        std::size_t tens = 1;

        for(std::size_t k = j - 1; k >= i; --k)
        {
            indexAcc += tens * (docs.at(k) - '0');
            tens *= 10;
        }

        result += argNames.at(indexAcc);
        i = j - 1;
    }

    return result;
#else
    return "";
#endif
}

LuaMetadataProxy::~LuaMetadataProxy()
#ifdef SSVOH_PRODUCE_LUA_METADATA
try
{
    luaMetadata.addFnEntry(
        (*erasedRet)(this), name, (*erasedArgs)(this), resolveArgNames(docs));
}
catch(const std::exception& e)
{
    ssvu::lo("LuaMetadataProxy")
        << "Failed to generate documentation: " << e.what() << '\n';
}
catch(...)
{
    ssvu::lo("LuaMetadataProxy") << "Failed to generate documentation\n";
}
#else
{
}
#endif

LuaMetadataProxy& LuaMetadataProxy::arg(
    [[maybe_unused]] const std::string& mArgName)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    argNames.emplace_back(mArgName);
#endif

    return *this;
}

LuaMetadataProxy& LuaMetadataProxy::doc(
    [[maybe_unused]] const std::string& mDocs)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    docs = mDocs;
#endif

    return *this;
}

} // namespace hg::Utils
