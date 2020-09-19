// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <SSVStart/Tileset/Tileset.hpp>
#include <SSVStart/Input/Enums.hpp>
#include <SSVStart/Input/Combo.hpp>
#include <SSVStart/Input/Trigger.hpp>
#include <SSVStart/Utils/Input.hpp>
#include <SSVStart/Global/Typedefs.hpp>
#include <SSVStart/Assets/AssetManager.hpp>

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

namespace ssvs
{

void loadAssetsFromJson(ssvs::AssetManager<>& mAM,
    const ssvufs::Path& mRootPath, const ssvuj::Obj& mObj);

} // namespace ssvs

namespace ssvuj
{

template <typename T>
SSVUJ_CNV_SIMPLE(sf::Vector2<T>, mObj, mV)
{
    ssvuj::convertArray(mObj, mV.x, mV.y);
}
SSVUJ_CNV_SIMPLE_END();

template <>
SSVUJ_CNV_SIMPLE(sf::Color, mObj, mV)
{
    ssvuj::convertArray(mObj, mV.r, mV.g, mV.b, mV.a);
}
SSVUJ_CNV_SIMPLE_END();

template <>
SSVUJ_CNV_SIMPLE(ssvs::Input::Trigger, mObj, mV)
{
    ssvuj::convert(mObj, mV.getCombos());
}
SSVUJ_CNV_SIMPLE_END();

template <>
struct Converter<ssvs::KKey>
{
    using T = ssvs::KKey;

    static void fromObj(const Obj& mObj, T& mValue)
    {
        mValue = ssvs::getKKey(getExtr<std::string>(mObj));
    }

    static void toObj(Obj& mObj, const T& mValue)
    {
        if(mValue == T::Unknown)
        {
            arch(mObj, "");
            return;
        }

        arch(mObj, ssvs::getKKeyName(mValue));
    }
};

template <>
struct Converter<ssvs::MBtn>
{
    using T = ssvs::MBtn;

    static void fromObj(const Obj& mObj, T& mValue)
    {
        mValue = ssvs::getMBtn(getExtr<std::string>(mObj));
    }

    static void toObj(Obj& mObj, const T& mValue)
    {
        arch(mObj, ssvs::getMBtnName(mValue));
    }
};

template <>
struct Converter<ssvs::Input::Combo>
{
    using T = ssvs::Input::Combo;

    static void fromObj(const Obj& mObj, T& mValue)
    {
        std::string str;

        for(const auto& i : mObj)
        {
            str = getExtr<std::string>(i);

            if(str.empty())
            {
                mValue.addKey(ssvs::KKey::Unknown);
            }
            else if(ssvs::isKKeyHardcoded(str))
            {
                ssvu::lo("ssvs::getInputComboFromJSON")
                    << "<" << i
                    << "> is an hardcoded key bind, an empty bind has been put "
                       "in its place"
                    << std::endl;

                mValue.addKey(ssvs::KKey::Unknown);
            }
            else if(ssvs::isKKeyNameValid(str))
            {
                mValue.addKey(getExtr<ssvs::KKey>(i));
            }
            else if(ssvs::isMBtnNameValid(str))
            {
                mValue.addBtn(getExtr<ssvs::MBtn>(i));
            }
            else
            {
                ssvu::lo("ssvs::getInputComboFromJSON")
                    << "<" << i
                    << "> is not a valid input name, an empty bind has been "
                       "put in its place"
                    << std::endl;

                mValue.addKey(ssvs::KKey::Unknown);
            }
        }
    }

    static void toObj(Obj& mObj, const T& mValue)
    {
        if(mValue.isUnbound())
        {
            arch(mObj, 0, ssvs::KKey(-1));
            return;
        }

        auto i(0u);
        const auto& keys(mValue.getKeys());
        const auto& btns(mValue.getBtns());

        for(auto j(0u); j < ssvs::kKeyCount; ++j)
        {
            if(ssvs::getKeyBit(keys, ssvs::KKey(j)))
            {
                arch(mObj, i++, ssvs::KKey(j));
            }
        }

        for(auto j(0u); j < ssvs::mBtnCount; ++j)
        {
            if(ssvs::getBtnBit(btns, ssvs::MBtn(j)))
            {
                arch(mObj, i++, ssvs::MBtn(j));
            }
        }
    }
};

} // namespace ssvuj
