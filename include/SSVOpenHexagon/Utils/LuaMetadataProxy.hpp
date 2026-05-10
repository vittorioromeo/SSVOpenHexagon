// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#ifdef SSVOH_PRODUCE_LUA_METADATA
    #include "SSVOpenHexagon/Utils/ArgExtractor.hpp"
#endif

#include "SSVOpenHexagon/Utils/TypeWrapper.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <tuple>

namespace hg::Utils
{

class LuaMetadata;

class LuaMetadataProxy
{
private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
    [[maybe_unused]] LuaMetadata&     luaMetadata;
    [[maybe_unused]] sf::base::String name;
    [[maybe_unused]] sf::base::String (*erasedRet)(LuaMetadataProxy*);
    [[maybe_unused]] sf::base::String (*erasedArgs)(LuaMetadataProxy*);
#pragma GCC diagnostic pop

    sf::base::String                   docs;
    sf::base::Vector<sf::base::String> argNames;

    template <typename T>
    [[nodiscard]] static const char* typeToStr(TypeWrapper<T>) noexcept;

    template <typename... Ts>
    [[nodiscard]] static sf::base::String typeToStr(TypeWrapper<std::tuple<Ts...>>);

    template <typename FOp>
    [[nodiscard]] static sf::base::String makeArgsString([[maybe_unused]] LuaMetadataProxy* self)
    {
#ifdef SSVOH_PRODUCE_LUA_METADATA
        using AE = Utils::ArgExtractor<FOp>;

        if constexpr (AE::numArgs == 0)
        {
            return "";
        }
        else if constexpr (AE::numArgs == 1)
        {
            sf::base::String res;

            res += typeToStr(TypeWrapper < SFML_BASE_DECAY(typename AE::template NthArg < 0 >>){});

            res += ' ';
            res += self->argNames.at(0);

            return res;
        }
        else
        {
            sf::base::String res;

            [&]<sf::base::SizeT... Is>(std::index_sequence<Is...>)
            {
                ((                                                                                             //
                     res += typeToStr(TypeWrapper < SFML_BASE_DECAY(typename AE::template NthArg < Is >>>){}), //
                     res += ' ',                                                                               //
                     res += self->argNames.at(Is),                                                             //
                     res += ", "),
                 ...);
            }(std::make_index_sequence<AE::numArgs>{});

            res.popBack();
            res.popBack();

            return res;
        }
#else
        return "";
#endif
    }

    [[nodiscard]] sf::base::String resolveArgNames(const sf::base::String& docs);

    template <typename Ret>
    [[nodiscard]] static sf::base::String makeErasedRet(LuaMetadataProxy*)
    {
#ifdef SSVOH_PRODUCE_LUA_METADATA
        return typeToStr(TypeWrapper<SFML_BASE_DECAY(Ret)>{});
#else
        return "";
#endif
    }

public:
#ifdef SSVOH_PRODUCE_LUA_METADATA
    template <typename F, typename FOp = decltype(&SFML_BASE_DECAY(F)::operator())>
    explicit LuaMetadataProxy(TypeWrapper<F>, LuaMetadata& mLuaMetadata, const sf::base::String& mName) :
        luaMetadata{mLuaMetadata},
        name{mName},
        erasedRet{&makeErasedRet<typename Utils::ArgExtractor<FOp>::Return>},
        erasedArgs{&makeArgsString<FOp>}
    {
    }
#else
    template <typename F>
    explicit LuaMetadataProxy(TypeWrapper<F>, LuaMetadata& mLuaMetadata, [[maybe_unused]] const sf::base::String& mName) :
        luaMetadata{mLuaMetadata},
        name{""},
        erasedRet{nullptr},
        erasedArgs{nullptr}
    {
    }
#endif

    ~LuaMetadataProxy();

    LuaMetadataProxy& arg(const sf::base::String& mArgName);
    LuaMetadataProxy& doc(const sf::base::String& mDocs);
};

} // namespace hg::Utils
