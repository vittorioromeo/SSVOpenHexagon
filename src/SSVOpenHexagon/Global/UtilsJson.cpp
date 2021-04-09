// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/UtilsJson.hpp"

#include <SFML/System.hpp>

#include <SSVStart/Global/Typedefs.hpp>
#include <SSVStart/Input/Enums.hpp>
#include <SSVStart/Input/Combo.hpp>
#include <SSVStart/Input/Trigger.hpp>
#include <SSVStart/Utils/Input.hpp>
#include <SSVStart/Assets/AssetManager.hpp>

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

namespace ssvs {

void loadAssetsFromJson(ssvs::AssetManager<>& mAM,
    const ssvufs::Path& mRootPath, const ssvuj::Obj& mObj)
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
