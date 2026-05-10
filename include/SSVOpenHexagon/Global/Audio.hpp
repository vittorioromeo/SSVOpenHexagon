// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"

namespace sf
{
class PlaybackDevice;
class SoundBuffer;
} // namespace sf

namespace hg
{

class Audio
{
public:
    using SoundBufferGetter = sf::base::FixedFunction<sf::SoundBuffer*(const sf::base::String&), 64>;

    using MusicPathGetter = sf::base::FixedFunction<const sf::base::String*(const sf::base::String&), 64>;

private:
    class AudioImpl;

    sf::base::UniquePtr<AudioImpl> _impl;

    [[nodiscard]] const AudioImpl& impl() const noexcept;
    [[nodiscard]] AudioImpl&       impl() noexcept;

public:
    explicit Audio(sf::PlaybackDevice&      playbackDevice,
                   const SoundBufferGetter& soundBufferGetter,
                   const MusicPathGetter&   musicPathGetter);

    ~Audio();

    void setSoundVolume(const float volume);
    void setMusicVolume(const float volume);

    void resumeMusic();
    void pauseMusic();
    void stopMusic();

    void setMusicPlayingOffsetSeconds(const float seconds);
    void setMusicPlayingOffsetMilliseconds(const int milliseconds);

    [[nodiscard]] float getMusicPlayingOffsetSeconds() const;
    [[nodiscard]] int   getMusicPlayingOffsetMilliseconds() const;

    void stopSounds();

    void playSoundOverride(const sf::base::String& id);
    void playPackSoundOverride(const sf::base::String& packId, const sf::base::String& id);

    void playSoundAbort(const sf::base::String& id);
    void playPackSoundAbort(const sf::base::String& packId, const sf::base::String& id);

    [[nodiscard]] bool loadAndPlayMusic(const sf::base::String& packId,
                                        const sf::base::String& id,
                                        const float             playingOffsetSeconds);

    void setCurrentMusicPitch(const float pitch);
};

} // namespace hg
