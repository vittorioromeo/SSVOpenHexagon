// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Audio.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

#include <SSVStart/SoundPlayer/SoundPlayer.hpp>
#include <SSVStart/MusicPlayer/MusicPlayer.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include <string>
#include <memory>

namespace hg
{

class Audio::AudioImpl
{
private:
    // TODO (P2): remove these, roll own system
    ssvs::SoundPlayer _soundPlayer;
    ssvs::MusicPlayer _musicPlayer;

    // TODO (P2): cleaner way of doing this
    SoundBufferGetter _soundBufferGetter;
    MusicGetter _musicGetter;

    void playSoundImpl(
        const std::string& assetId, const ssvs::SoundPlayer::Mode mode)
    {
        if(sf::SoundBuffer* soundBuffer = _soundBufferGetter(assetId);
            soundBuffer != nullptr)
        {
            _soundPlayer.play(*soundBuffer, mode);
        }
    }

public:
    explicit AudioImpl(const SoundBufferGetter& soundBufferGetter,
        const MusicGetter& musicGetter)
        : _soundBufferGetter{soundBufferGetter}, _musicGetter{musicGetter}
    {
        SSVOH_ASSERT(static_cast<bool>(_soundBufferGetter));
        SSVOH_ASSERT(static_cast<bool>(_musicGetter));
    }

    void setSoundVolume(const float volume)
    {
        SSVOH_ASSERT(volume >= 0.f && volume <= 100.f);
        _soundPlayer.setVolume(volume);
    }

    void setMusicVolume(const float volume)
    {
        SSVOH_ASSERT(volume >= 0.f && volume <= 100.f);
        _musicPlayer.setVolume(volume);
    }

    void resumeMusic()
    {
        _musicPlayer.resume();
    }

    void pauseMusic()
    {
        _musicPlayer.pause();
    }

    void stopMusic()
    {
        _musicPlayer.stop();
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

    void playMusic(const std::string& packId, const std::string& id,
        const float playingOffsetSeconds)
    {
        const std::string assetId = Utils::concat(packId, '_', id);

        if(sf::Music* music = _musicGetter(assetId); music != nullptr)
        {
            _musicPlayer.play(*music, sf::seconds(playingOffsetSeconds));
        }
    }

    void setCurrentMusicPitch(const float pitch)
    {
        if(sf::Music* currentMusic = _musicPlayer.getCurrent();
            currentMusic != nullptr)
        {
            currentMusic->setPitch(pitch);
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

Audio::Audio(
    const SoundBufferGetter& soundBufferGetter, const MusicGetter& musicGetter)
    : _impl{std::make_unique<AudioImpl>(soundBufferGetter, musicGetter)}
{
}

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

void Audio::playMusic(const std::string& packId, const std::string& id,
    const float playingOffsetSeconds)
{
    impl().playMusic(packId, id, playingOffsetSeconds);
}

void Audio::setCurrentMusicPitch(const float pitch)
{
    impl().setCurrentMusicPitch(pitch);
}

} // namespace hg
