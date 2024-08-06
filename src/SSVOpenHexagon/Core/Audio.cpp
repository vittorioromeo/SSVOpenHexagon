// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Audio.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"

#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/UniquePtr.hpp"

#include <SFML/System/Time.hpp>
#include <SSVStart/SoundPlayer/SoundPlayer.hpp>

#include <SSVUtils/Core/Log/Log.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

#include <SFML/Base/Optional.hpp>

#include <SFML/Base/Optional.hpp>
#include <string>

namespace hg {

class Audio::AudioImpl
{
private:
    // TODO (P2): cleaner way of doing this
    sf::PlaybackDevice& _playbackDevice;
    SoundBufferGetter _soundBufferGetter;
    MusicPathGetter _musicPathGetter;

    // TODO (P2): remove these, roll own system
    ssvs::SoundPlayer _soundPlayer;

    sf::base::Optional<sf::Music> _music;
    float _musicVolume;
    std::string _lastLoadedMusicPath;


    void playSoundImpl(
        const std::string& assetId, const ssvs::SoundPlayer::Mode mode)
    {
        if (sf::SoundBuffer* soundBuffer = _soundBufferGetter(assetId);
            soundBuffer != nullptr)
        {
            _soundPlayer.play(_playbackDevice, *soundBuffer, mode);
        }
    }

public:
    explicit AudioImpl(sf::PlaybackDevice& playbackDevice,
        const SoundBufferGetter& soundBufferGetter,
        const MusicPathGetter& musicPathGetter)
        : _playbackDevice{playbackDevice},
          _soundBufferGetter{soundBufferGetter},
          _musicPathGetter{musicPathGetter},
          _soundPlayer{},
          _music{},
          _musicVolume{100.f},
          _lastLoadedMusicPath{}
    {
        SSVOH_ASSERT(static_cast<bool>(_soundBufferGetter));
    }

    void setSoundVolume(const float volume)
    {
        SSVOH_ASSERT(volume >= 0.f && volume <= 100.f);
        _soundPlayer.setVolume(volume);
    }

    void setMusicVolume(const float volume)
    {
        SSVOH_ASSERT(volume >= 0.f && volume <= 100.f);
        _musicVolume = volume;

        if (_music.hasValue())
        {
            _music->setVolume(_musicVolume);
        }
    }

    void resumeMusic()
    {
        if (_music.hasValue())
        {
            _music->setVolume(_musicVolume);
            _music->play(_playbackDevice);
        }
    }

    void pauseMusic()
    {
        if (_music.hasValue())
        {
            _music->pause();
        }
    }

    void stopMusic()
    {
        if (_music.hasValue())
        {
            _music->stop();
        }
    }

    void setMusicPlayingOffsetSeconds(const float seconds)
    {
        if (_music.hasValue())
        {
            _music->setPlayingOffset(sf::seconds(seconds));
        }
    }

    void setMusicPlayingOffsetMilliseconds(const int milliseconds)
    {
        if (_music.hasValue())
        {
            _music->setPlayingOffset(sf::milliseconds(milliseconds));
        }
    }

    [[nodiscard]] float getMusicPlayingOffsetSeconds() const
    {
        if (_music.hasValue())
        {
            return _music->getPlayingOffset().asSeconds();
        }

        return 0.f;
    }

    [[nodiscard]] int getMusicPlayingOffsetMilliseconds() const
    {
        if (_music.hasValue())
        {
            return _music->getPlayingOffset().asMilliseconds();
        }

        return 0;
    }

    void stopSounds()
    {
        _soundPlayer.stop();
    }

    void playSoundOverride(const std::string& id)
    {
        playSoundImpl(id, ssvs::SoundPlayer::Mode::Override);
    }

    void playPackSoundOverride(const std::string& packId, const std::string& id)
    {
        playSoundImpl(
            Utils::concat(packId, '_', id), ssvs::SoundPlayer::Mode::Override);
    }

    void playSoundAbort(const std::string& id)
    {
        playSoundImpl(id, ssvs::SoundPlayer::Mode::Abort);
    }

    void playPackSoundAbort(const std::string& packId, const std::string& id)
    {
        playSoundImpl(
            Utils::concat(packId, '_', id), ssvs::SoundPlayer::Mode::Abort);
    }

    [[nodiscard]] bool loadAndPlayMusic(const std::string& packId,
        const std::string& id, const float playingOffsetSeconds)
    {
        const std::string assetId = Utils::concat(packId, '_', id);
        const std::string* path = _musicPathGetter(assetId);

        if (path == nullptr)
        {
            ssvu::lo("hg::AudioImpl::loadAndPlayMusic")
                << "No path for music id '" << assetId << "'\n";

            return false;
        }

        if (_lastLoadedMusicPath != *path)
        {
            if (!(_music = sf::Music::openFromFile(*path)))
            {
                ssvu::lo("hg::AudioImpl::loadAndPlayMusic")
                    << "Failed loading music file '" << path << "'\n";

                _music.reset();
                return false;
            }

            _lastLoadedMusicPath = *path;
        }

        SSVOH_ASSERT(_music.hasValue());

        _music->setLoop(true);
        setMusicPlayingOffsetSeconds(playingOffsetSeconds);
        resumeMusic();

        return true;
    }

    void setCurrentMusicPitch(const float pitch)
    {
        if (_music.hasValue())
        {
            _music->setPitch(pitch);
        }
    }
};

[[nodiscard]] const Audio::AudioImpl& Audio::impl() const noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

[[nodiscard]] Audio::AudioImpl& Audio::impl() noexcept
{
    SSVOH_ASSERT(_impl != nullptr);
    return *_impl;
}

Audio::Audio(sf::PlaybackDevice& playbackDevice,
    const SoundBufferGetter& soundBufferGetter,
    const MusicPathGetter& musicPathGetter)
    : _impl{Utils::makeUnique<AudioImpl>(
          playbackDevice, soundBufferGetter, musicPathGetter)}
{}

Audio::~Audio() = default;

void Audio::setSoundVolume(const float volume)
{
    impl().setSoundVolume(volume);
}

void Audio::setMusicVolume(const float volume)
{
    impl().setMusicVolume(volume);
}

void Audio::resumeMusic()
{
    impl().resumeMusic();
}

void Audio::pauseMusic()
{
    impl().pauseMusic();
}

void Audio::stopMusic()
{
    impl().stopMusic();
}

void Audio::setMusicPlayingOffsetSeconds(const float seconds)
{
    impl().setMusicPlayingOffsetSeconds(seconds);
}

void Audio::setMusicPlayingOffsetMilliseconds(const int milliseconds)
{
    impl().setMusicPlayingOffsetMilliseconds(milliseconds);
}

[[nodiscard]] float Audio::getMusicPlayingOffsetSeconds() const
{
    return impl().getMusicPlayingOffsetSeconds();
}

[[nodiscard]] int Audio::getMusicPlayingOffsetMilliseconds() const
{
    return impl().getMusicPlayingOffsetMilliseconds();
}

void Audio::stopSounds()
{
    impl().stopSounds();
}

void Audio::playSoundOverride(const std::string& id)
{
    impl().playSoundOverride(id);
}

void Audio::playPackSoundOverride(
    const std::string& packId, const std::string& id)
{
    impl().playPackSoundOverride(packId, id);
}

void Audio::playSoundAbort(const std::string& id)
{
    impl().playSoundAbort(id);
}

void Audio::playPackSoundAbort(const std::string& packId, const std::string& id)
{
    impl().playPackSoundAbort(packId, id);
}

[[nodiscard]] bool Audio::loadAndPlayMusic(const std::string& packId,
    const std::string& id, const float playingOffsetSeconds)
{
    return impl().loadAndPlayMusic(packId, id, playingOffsetSeconds);
}

void Audio::setCurrentMusicPitch(const float pitch)
{
    impl().setCurrentMusicPitch(pitch);
}

} // namespace hg
