// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/UtilsJson.hpp"

#include <SSVStart/Assets/AssetManager.hpp>
#include <SSVStart/Global/Typedefs.hpp>
#include <SSVStart/Input/Combo.hpp>
#include <SSVStart/Input/Enums.hpp>
#include <SSVStart/Input/Trigger.hpp>
#include <SSVStart/Utils/Input.hpp>

#include <SSVUtils/Core/Log/Log.hpp>

#include <SSVUtils/Core/FileSystem/Path.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Color.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/System/Vector2.hpp>

namespace ssvs {

void loadAssetsFromJson(ssvs::DefaultAssetManager& mAM,
    const ssvu::FileSystem::Path& mRootPath, const ssvuj::Obj& mObj)
{
    for(const auto& f : ssvuj::getExtr<std::vector<std::string>>(mObj, "fonts"))
    {
        mAM.template load<sf::Font>(f, mRootPath + f);
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "images"))
    {
        mAM.template load<sf::Image>(f, mRootPath + f);
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "textures"))
    {
        mAM.template load<sf::Texture>(f, mRootPath + f);
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "soundBuffers"))
    {
        mAM.template load<sf::SoundBuffer>(f, mRootPath + f);
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "musics"))
    {
        mAM.template load<sf::Music>(f, mRootPath + f);
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "shadersVertex"))
    {
        mAM.template load<sf::Shader>(
            f, mRootPath + f, sf::Shader::Type::Vertex, Impl::ShaderFromPath{});
    }

    for(const auto& f :
        ssvuj::getExtr<std::vector<std::string>>(mObj, "shadersFragment"))
    {
        mAM.template load<sf::Shader>(f, mRootPath + f,
            sf::Shader::Type::Fragment, Impl::ShaderFromPath{});
    }
}

} // namespace ssvs

namespace ssvuj {

void Converter<sf::Vector2f>::fromObj(const Obj& mObj, T& mValue)
{
    extr(mObj, 0, mValue.x);
    extr(mObj, 1, mValue.y);
}

void Converter<sf::Vector2f>::toObj(Obj& mObj, const T& mValue)
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

void Converter<ssvs::KKey>::fromObj(const Obj& mObj, T& mValue)
{
    mValue = ssvs::getKKey(getExtr<std::string>(mObj));
}

void Converter<ssvs::KKey>::toObj(Obj& mObj, const T& mValue)
{
    if(mValue == T::Unknown)
    {
        std::string empty;
        arch(mObj, empty); // TODO (P2): using `""` seems to be bugged
        return;
    }

    arch(mObj, ssvs::getKKeyName(mValue));
}

void Converter<ssvs::MBtn>::fromObj(const Obj& mObj, T& mValue)
{
    mValue = ssvs::getMBtn(getExtr<std::string>(mObj));
}

void Converter<ssvs::MBtn>::toObj(Obj& mObj, const T& mValue)
{
    arch(mObj, ssvs::getMBtnName(mValue));
}

void Converter<ssvs::Input::Combo>::fromObj(const Obj& mObj, T& mValue)
{
    mValue.clearBind();

    std::string str;

    for(const auto& i : mObj)
    {
        str = getExtr<std::string>(i);

        if(str.empty())
        {
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
                   "put in its place\n";

            mValue.addKey(ssvs::KKey::Unknown);
        }
    }
}

void Converter<ssvs::Input::Combo>::toObj(Obj& mObj, const T& mValue)
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

} // namespace ssvuj
