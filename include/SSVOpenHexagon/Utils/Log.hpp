// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"

namespace hg
{

inline constexpr struct LogEndl
{
} logEndl;

struct LogStream
{
    LogStream& operator<<(const LogEndl&);
    LogStream& operator<<(const sf::base::StringView&);

    template <sf::base::SizeT N>
    LogStream& operator<<(const char (&arr)[N])
    {
        return operator<<(sf::base::StringView(arr, N));
    }

    template <typename T>
    LogStream& operator<<(const T&);

    void flush();

    struct Impl;
    sf::base::InPlacePImpl<Impl, 1024> impl;
};

[[nodiscard]] LogStream& getLogStream();

[[nodiscard]] LogStream& lo();
[[nodiscard]] LogStream& lo(sf::base::StringView title);


} // namespace hg
