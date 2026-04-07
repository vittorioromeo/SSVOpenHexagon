// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/UniquePtr.hpp"


namespace sf
{

class Font;
class SoundBuffer;
class Texture;

} // namespace sf

namespace sf::base
{

class String;

} // namespace sf::base

namespace hg
{

class AssetStorage
{
private:
    class AssetStorageImpl;

    sf::base::UniquePtr<AssetStorageImpl> _impl;

    [[nodiscard]] const AssetStorageImpl& impl() const noexcept;
    [[nodiscard]] AssetStorageImpl&       impl() noexcept;

public:
    explicit AssetStorage();
    ~AssetStorage();

    [[nodiscard]] bool loadTexture(const sf::base::String& id, const sf::base::String& path);

    [[nodiscard]] bool loadFont(const sf::base::String& id, const sf::base::String& path);

    [[nodiscard]] bool loadSoundBuffer(const sf::base::String& id, const sf::base::String& path);

    [[nodiscard]] sf::Texture*     getTexture(const sf::base::String& id) noexcept;
    [[nodiscard]] sf::Font*        getFont(const sf::base::String& id) noexcept;
    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(const sf::base::String& id) noexcept;

    [[nodiscard]] bool hasTexture(const sf::base::String& id) noexcept;
    [[nodiscard]] bool hasFont(const sf::base::String& id) noexcept;
    [[nodiscard]] bool hasSoundBuffer(const sf::base::String& id) noexcept;
};

} // namespace hg
