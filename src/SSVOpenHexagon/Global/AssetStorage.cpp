// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/AssetStorage.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"

#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/UniquePtr.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Audio/SoundBuffer.hpp>

#include <string>
#include <unordered_map>

namespace hg {

template <typename Map, typename Key, typename F>
[[nodiscard]] static decltype(auto) tryEmplaceAndThen(
    Map& map, const Key& key, F&& f)
{
    auto [it, inserted] =
        map.emplace(std::piecewise_construct, std::tuple{key}, std::tuple{});

    return inserted && f(it->second);
}

template <typename Map, typename Key>
[[nodiscard]] static decltype(auto) tryEmplaceAndThenLoadFromFile(
    Map& map, const Key& key, const std::string& path)
{
    return tryEmplaceAndThen(
        map, key, [&path](auto& value) { return value.loadFromFile(path); });
}

template <typename Map, typename Key>
[[nodiscard]] static auto* getAsPtr(Map& map, const Key& key) noexcept
{
    auto it = map.find(key);
    return it == map.end() ? nullptr : &it->second;
}

class AssetStorage::AssetStorageImpl
{
private:
    std::unordered_map<std::string, sf::Texture> _textures;
    std::unordered_map<std::string, sf::Font> _fonts;
    std::unordered_map<std::string, sf::SoundBuffer> _soundBuffers;

public:
    [[nodiscard]] bool loadTexture(
        const std::string& id, const std::string& path)
    {
        return tryEmplaceAndThenLoadFromFile(_textures, id, path);
    }

    [[nodiscard]] bool loadFont(const std::string& id, const std::string& path)
    {
        return tryEmplaceAndThenLoadFromFile(_fonts, id, path);
    }

    [[nodiscard]] bool loadSoundBuffer(
        const std::string& id, const std::string& path)
    {
        return tryEmplaceAndThenLoadFromFile(_soundBuffers, id, path);
    }

    [[nodiscard]] sf::Texture* getTexture(const std::string& id) noexcept
    {
        return getAsPtr(_textures, id);
    }

    [[nodiscard]] sf::Font* getFont(const std::string& id) noexcept
    {
        return getAsPtr(_fonts, id);
    }

    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(
        const std::string& id) noexcept
    {
        return getAsPtr(_soundBuffers, id);
    }

    [[nodiscard]] bool hasTexture(const std::string& id) noexcept
    {
        return _textures.find(id) != _textures.end();
    }

    [[nodiscard]] bool hasFont(const std::string& id) noexcept
    {
        return _fonts.find(id) != _fonts.end();
    }

    [[nodiscard]] bool hasSoundBuffer(const std::string& id) noexcept
    {
        return _soundBuffers.find(id) != _soundBuffers.end();
    }
};

[[nodiscard]] const AssetStorage::AssetStorageImpl&
AssetStorage::impl() const noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

[[nodiscard]] AssetStorage::AssetStorageImpl& AssetStorage::impl() noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

AssetStorage::AssetStorage() : _impl{Utils::makeUnique<AssetStorageImpl>()}
{}

AssetStorage::~AssetStorage() = default;

[[nodiscard]] bool AssetStorage::loadTexture(
    const std::string& id, const std::string& path)
{
    return impl().loadTexture(id, path);
}

[[nodiscard]] bool AssetStorage::loadFont(
    const std::string& id, const std::string& path)
{
    return impl().loadFont(id, path);
}

[[nodiscard]] bool AssetStorage::loadSoundBuffer(
    const std::string& id, const std::string& path)
{
    return impl().loadSoundBuffer(id, path);
}

[[nodiscard]] sf::Texture* AssetStorage::getTexture(
    const std::string& id) noexcept
{
    return impl().getTexture(id);
}

[[nodiscard]] sf::Font* AssetStorage::getFont(const std::string& id) noexcept
{
    return impl().getFont(id);
}

[[nodiscard]] sf::SoundBuffer* AssetStorage::getSoundBuffer(
    const std::string& id) noexcept
{
    return impl().getSoundBuffer(id);
}

[[nodiscard]] bool AssetStorage::hasTexture(const std::string& id) noexcept
{
    return impl().hasTexture(id);
}

[[nodiscard]] bool AssetStorage::hasFont(const std::string& id) noexcept
{
    return impl().hasFont(id);
}

[[nodiscard]] bool AssetStorage::hasSoundBuffer(const std::string& id) noexcept
{
    return impl().hasSoundBuffer(id);
}

} // namespace hg
