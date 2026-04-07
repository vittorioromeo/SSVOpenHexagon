// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Input/Combo.hpp"
#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"
#include "SSVOpenHexagon/Input/Utils.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/BasicConverters.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Main.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/String.hpp"

namespace ssvuj
{

void Converter<sf::Vec2f>::fromObj(const Obj& mObj, T& mValue)
{
    extr(mObj, 0, mValue.x);
    extr(mObj, 1, mValue.y);
}

void Converter<sf::Vec2f>::toObj(Obj& mObj, const T& mValue)
{
    arch(mObj, 0, mValue.x);
    arch(mObj, 1, mValue.y);
}

void Converter<sf::Color>::fromObj(const Obj& mObj, T& mValue)
{
    extr(mObj, 0, mValue.r);
    extr(mObj, 1, mValue.g);
    extr(mObj, 2, mValue.b);
    extr(mObj, 3, mValue.a);
}

void Converter<sf::Color>::toObj(Obj& mObj, const T& mValue)
{
    arch(mObj, 0, mValue.r);
    arch(mObj, 1, mValue.g);
    arch(mObj, 2, mValue.b);
    arch(mObj, 3, mValue.a);
}

void Converter<ssvs::Input::Trigger>::fromObj(const Obj& mObj, T& mValue)
{
    extr(mObj, mValue.getCombos());
}

void Converter<ssvs::Input::Trigger>::toObj(Obj& mObj, const T& mValue)
{
    arch(mObj, mValue.getCombos());
}

void Converter<sf::Keyboard::Key>::fromObj(const Obj& mObj, T& mValue)
{
    mValue = ssvs::getKKey(getExtr<sf::base::String>(mObj));
}

void Converter<sf::Keyboard::Key>::toObj(Obj& mObj, const T& mValue)
{
    if (mValue == T::Unknown)
    {
        sf::base::String empty;
        arch(mObj, empty); // TODO (P2): using `""` seems to be bugged
        return;
    }

    arch(mObj, ssvs::getKKeyName(mValue));
}

void Converter<sf::Mouse::Button>::fromObj(const Obj& mObj, T& mValue)
{
    mValue = ssvs::getMBtn(getExtr<sf::base::String>(mObj));
}

void Converter<sf::Mouse::Button>::toObj(Obj& mObj, const T& mValue)
{
    arch(mObj, ssvs::getMBtnName(mValue));
}

void Converter<ssvs::Input::Combo>::fromObj(const Obj& mObj, T& mValue)
{
    mValue.clearBind();

    sf::base::String str;

    for (const auto& i : mObj)
    {
        str = getExtr<sf::base::String>(i);

        if (str.empty())
        {
            mValue.addKey(sf::Keyboard::Key::Unknown);
        }
        else if (ssvs::isKKeyNameValid(str))
        {
            mValue.addKey(getExtr<sf::Keyboard::Key>(i));
        }
        else if (ssvs::isMBtnNameValid(str))
        {
            mValue.addBtn(getExtr<sf::Mouse::Button>(i));
        }
        else
        {
            hg::lo("ssvs::getInputComboFromJSON")
                << "<" << i
                << "> is not a valid input name, an empty bind has been "
                   "put in its place\n";

            mValue.addKey(sf::Keyboard::Key::Unknown);
        }
    }
}

void Converter<ssvs::Input::Combo>::toObj(Obj& mObj, const T& mValue)
{
    if (mValue.isUnbound())
    {
        arch(mObj, 0, sf::Keyboard::Key::Unknown);
        return;
    }

    auto        i(0u);
    const auto& keys(mValue.getKeys());
    const auto& btns(mValue.getBtns());

    for (int j = 0; j < static_cast<int>(sf::Keyboard::KeyCount); ++j)
    {
        if (ssvs::getKeyBit(keys, sf::Keyboard::Key(j)))
        {
            arch(mObj, i++, sf::Keyboard::Key(j));
        }
    }

    for (int j = 0; j < static_cast<int>(sf::Mouse::ButtonCount); ++j)
    {
        if (ssvs::getBtnBit(btns, sf::Mouse::Button(j)))
        {
            arch(mObj, i++, sf::Mouse::Button(j));
        }
    }
}

} // namespace ssvuj
