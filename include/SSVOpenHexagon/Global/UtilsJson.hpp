// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/System/Vector2.hpp>

namespace ssvu::FileSystem {
class Path;
}

namespace ssvs {
class DefaultAssetManager;
}

namespace ssvs::Input {
class Trigger;
class Combo;
} // namespace ssvs::Input

namespace sf {
class Color;
}

namespace ssvs {

void loadAssetsFromJson(ssvs::DefaultAssetManager& mAM,
    const ssvu::FileSystem::Path& mRootPath, const ssvuj::Obj& mObj);

} // namespace ssvs

namespace ssvuj {

template <>
struct Converter<sf::Vector2f>
{
    using T = sf::Vector2f;

    static void fromObj(const Obj& mObj, T& mValue);
    static void toObj(Obj& mObj, const T& mValue);
};

template <>
struct Converter<sf::Color>
{
    using T = sf::Color;

    static void fromObj(const Obj& mObj, T& mValue);
    static void toObj(Obj& mObj, const T& mValue);
};

template <>
struct Converter<ssvs::Input::Trigger>
{
    using T = ssvs::Input::Trigger;

    static void fromObj(const Obj& mObj, T& mValue);
    static void toObj(Obj& mObj, const T& mValue);
};

template <>
struct Converter<sf::Keyboard::Key>
{
    using T = sf::Keyboard::Key;

    static void fromObj(const Obj& mObj, T& mValue);
    static void toObj(Obj& mObj, const T& mValue);
};

template <>
struct Converter<sf::Mouse::Button>
{
    using T = sf::Mouse::Button;

    static void fromObj(const Obj& mObj, T& mValue);
    static void toObj(Obj& mObj, const T& mValue);
};

template <>
struct Converter<ssvs::Input::Combo>
{
    using T = ssvs::Input::Combo;

    static void fromObj(const Obj& mObj, T& mValue);
    static void toObj(Obj& mObj, const T& mValue);
};

} // namespace ssvuj
