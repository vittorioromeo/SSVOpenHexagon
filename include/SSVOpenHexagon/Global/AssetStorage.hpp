// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/UniquePtr.hpp"

#include <string>

namespace sf {

class SoundBuffer;
class Texture;
class Font;

} // namespace sf

namespace hg {

class AssetStorage
{
private:
    class AssetStorageImpl;

    Utils::UniquePtr<AssetStorageImpl> _impl;

    [[nodiscard]] const AssetStorageImpl& impl() const noexcept;
    [[nodiscard]] AssetStorageImpl& impl() noexcept;

public:
    explicit AssetStorage();
    ~AssetStorage();

    [[nodiscard]] bool loadTexture(
        const std::string& id, const std::string& path);

    [[nodiscard]] bool loadFont(const std::string& id, const std::string& path);

    [[nodiscard]] bool loadSoundBuffer(
        const std::string& id, const std::string& path);

    [[nodiscard]] sf::Texture* getTexture(const std::string& id) noexcept;
    [[nodiscard]] sf::Font* getFont(const std::string& id) noexcept;
    [[nodiscard]] sf::SoundBuffer* getSoundBuffer(
        const std::string& id) noexcept;

    [[nodiscard]] bool hasTexture(const std::string& id) noexcept;
    [[nodiscard]] bool hasFont(const std::string& id) noexcept;
    [[nodiscard]] bool hasSoundBuffer(const std::string& id) noexcept;
};

} // namespace hg
