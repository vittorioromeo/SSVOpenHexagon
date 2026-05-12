// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LuaMetadataProxy.hpp"
#include "SSVOpenHexagon/Utils/TypeWrapper.hpp"

#include "SFML/Base/String.hpp"

#include <tuple>


namespace hg::Utils
{

template <typename T>
[[nodiscard]] const char* LuaMetadataProxy::typeToStr(TypeWrapper<T>) noexcept
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    #define RETURN_T_STR(type)       \
        (SFML_BASE_IS_SAME(T, type)) \
        {                            \
            return #type;            \
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
    else if constexpr RETURN_T_STR(sf::base::String)
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
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<unsigned long long>);
template const char* LuaMetadataProxy::typeToStr(TypeWrapper<sf::base::String>);
#endif

// ----------------------------------------------------------------------------

template <typename... Ts>
[[nodiscard]] sf::base::String LuaMetadataProxy::typeToStr(TypeWrapper<std::tuple<Ts...>>)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    sf::base::String result;

    result += "tuple<";
    if constexpr (sizeof...(Ts) > 0)
    {
        (((result += typeToStr(TypeWrapper<Ts>{})), result += ", "), ...);
        result.popBack();
        result.popBack();
    }
    result += ">";

    return result;
#else
    return "";
#endif
}

#ifdef SSVOH_PRODUCE_LUA_METADATA
template sf::base::String LuaMetadataProxy::typeToStr(TypeWrapper<std::tuple<int, int, int, int>>);

template sf::base::String LuaMetadataProxy::typeToStr(TypeWrapper<std::tuple<float, float>>);

template sf::base::String LuaMetadataProxy::typeToStr(
    TypeWrapper<std::tuple<float, float, float, float, float, float, float, float>>);
#endif

// ----------------------------------------------------------------------------

[[nodiscard]] sf::base::String LuaMetadataProxy::resolveArgNames([[maybe_unused]] const sf::base::String& docs)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    sf::base::SizeT argNameSize = 0;
    for (const auto& argName : argNames)
    {
        argNameSize += argName.size() + 4;
    }

    sf::base::String result;
    result.reserve(docs.size() + argNameSize);

    for (sf::base::SizeT i = 0; i < docs.size(); ++i)
    {
        if (docs[i] != '$')
        {
            result += docs[i];
            continue;
        }

        ++i;

        sf::base::SizeT j = i;
        for (; j < docs.size(); ++j)
        {
            const char next = docs.at(j);
            if (next < '0' || next > '9')
            {
                break;
            }
        }

        // Range `[i, j)` is now the position of the argument.
        // Parse into integer.

        sf::base::SizeT indexAcc = 0;
        sf::base::SizeT tens     = 1;

        for (sf::base::SizeT k = j - 1; k >= i; --k)
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
    luaMetadata.addFnEntry((*erasedRet)(this), name, (*erasedArgs)(this), resolveArgNames(docs));
} catch (const std::exception& e)
{
    hg::lo("LuaMetadataProxy") << "Failed to generate documentation: " << e.what() << '\n';
} catch (...)
{
    hg::lo("LuaMetadataProxy") << "Failed to generate documentation\n";
}
#else
{
}
#endif

LuaMetadataProxy& LuaMetadataProxy::arg([[maybe_unused]] const sf::base::String& mArgName)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    argNames.emplaceBack(mArgName);
#endif

    return *this;
}

LuaMetadataProxy& LuaMetadataProxy::doc([[maybe_unused]] const sf::base::String& mDocs)
{
#ifdef SSVOH_PRODUCE_LUA_METADATA
    docs = mDocs;
#endif

    return *this;
}

} // namespace hg::Utils
