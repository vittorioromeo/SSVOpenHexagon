// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/AssetStorage.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <cstring>

namespace hg
{

template <typename Map, typename Key>
[[nodiscard]] static auto* getAsPtr(Map& map, const Key& key) noexcept
{
    auto it = map.find(key);
    return it == map.end() ? nullptr : it->second.get();
}

class AssetStorage::AssetStorageImpl
{
private:
    // `UniquePtr` indirection: returned `T*` from `get*()` must stay valid
    // across subsequent insertions (which can rehash the table).
    ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<sf::Texture>>     _textures;
    ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<sf::Font>>        _fonts;
    ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<sf::SoundBuffer>> _soundBuffers;

public:
    [[nodiscard]] bool loadTexture(const sf::base::String& id, const sf::base::String& path)
    {
        sf::base::Optional texture = sf::Texture::loadFromFile(path);

        if (!texture.hasValue())
        {
            return false;
        }

        auto [it, inserted] = _textures.emplace(id, sf::base::makeUnique<sf::Texture>(*SFML_BASE_MOVE(texture)));
        return inserted;
    }

    [[nodiscard]] bool loadFont(const sf::base::String& id, const sf::base::String& path)
    {
        sf::base::Optional font = sf::Font::openFromFile(path);

        if (!font.hasValue())
        {
            return false;
        }

        auto [it, inserted] = _fonts.emplace(id, sf::base::makeUnique<sf::Font>(*SFML_BASE_MOVE(font)));
        return inserted;
    }

    [[nodiscard]] bool loadSoundBuffer(const sf::base::String& id, const sf::base::String& path)
    {
        sf::base::Optional soundBuffer = sf::SoundBuffer::loadFromFile(path);

        if (!soundBuffer.hasValue())
        {
            return false;
        }

        auto [it,
              inserted] = _soundBuffers.emplace(id, sf::base::makeUnique<sf::SoundBuffer>(*SFML_BASE_MOVE(soundBuffer)));
        return inserted;
    }

    [[nodiscard]] sf::Texture* getTexture(const sf::base::String& id) noexcept
    {
        return getAsPtr(_textures, id);
    }

    [[nodiscard]] sf::Font* getFont(const sf::base::String& id) noexcept
    {
        return getAsPtr(_fonts, id);
    }

    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(const sf::base::String& id) noexcept
    {
        return getAsPtr(_soundBuffers, id);
    }

    [[nodiscard]] bool hasTexture(const sf::base::String& id) noexcept
    {
        return _textures.find(id) != _textures.end();
    }

    [[nodiscard]] bool hasFont(const sf::base::String& id) noexcept
    {
        return _fonts.find(id) != _fonts.end();
    }

    [[nodiscard]] bool hasSoundBuffer(const sf::base::String& id) noexcept
    {
        return _soundBuffers.find(id) != _soundBuffers.end();
    }

    void removeByPackPrefix(const sf::base::String& packIdPrefix)
    {
        // Erase every key starting with `packIdPrefix` from a single map.
        const auto sweep = [&](auto& map)
        {
            for (auto it = map.begin(); it != map.end();)
            {
                const sf::base::String& key = it->first;

                const bool matches = key.size() >= packIdPrefix.size() &&
                                     std::memcmp(key.data(), packIdPrefix.data(), packIdPrefix.size()) == 0;

                if (matches)
                    it = map.erase(it);
                else
                    ++it;
            }
        };

        sweep(_textures);
        sweep(_fonts);
        sweep(_soundBuffers);
    }
};

[[nodiscard]] const AssetStorage::AssetStorageImpl& AssetStorage::impl() const noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

[[nodiscard]] AssetStorage::AssetStorageImpl& AssetStorage::impl() noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

AssetStorage::AssetStorage() : _impl{sf::base::makeUnique<AssetStorageImpl>()}
{
}

AssetStorage::~AssetStorage() = default;

[[nodiscard]] bool AssetStorage::loadTexture(const sf::base::String& id, const sf::base::String& path)
{
    return impl().loadTexture(id, path);
}

[[nodiscard]] bool AssetStorage::loadFont(const sf::base::String& id, const sf::base::String& path)
{
    return impl().loadFont(id, path);
}

[[nodiscard]] bool AssetStorage::loadSoundBuffer(const sf::base::String& id, const sf::base::String& path)
{
    return impl().loadSoundBuffer(id, path);
}

[[nodiscard]] sf::Texture* AssetStorage::getTexture(const sf::base::String& id) noexcept
{
    return impl().getTexture(id);
}

[[nodiscard]] sf::Font* AssetStorage::getFont(const sf::base::String& id) noexcept
{
    return impl().getFont(id);
}

[[nodiscard]] sf::SoundBuffer* AssetStorage::getSoundBuffer(const sf::base::String& id) noexcept
{
    return impl().getSoundBuffer(id);
}

[[nodiscard]] bool AssetStorage::hasTexture(const sf::base::String& id) noexcept
{
    return impl().hasTexture(id);
}

[[nodiscard]] bool AssetStorage::hasFont(const sf::base::String& id) noexcept
{
    return impl().hasFont(id);
}

[[nodiscard]] bool AssetStorage::hasSoundBuffer(const sf::base::String& id) noexcept
{
    return impl().hasSoundBuffer(id);
}

void AssetStorage::removeByPackPrefix(const sf::base::String& packIdPrefix)
{
    impl().removeByPackPrefix(packIdPrefix);
}

} // namespace hg
