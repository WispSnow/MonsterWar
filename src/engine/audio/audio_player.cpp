#include "audio_player.h"
#include "engine/resource/resource_manager.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

namespace engine::audio {
AudioPlayer::~AudioPlayer() {
    if (music_track_) {
        MIX_DestroyTrack(music_track_);
        music_track_ = nullptr;
    }
}

AudioPlayer::AudioPlayer(engine::resource::ResourceManager* resource_manager)
    : resource_manager_(resource_manager) {
    if (!resource_manager_) {
        throw std::runtime_error("AudioPlayer 构造失败: 提供的 ResourceManager 指针为空。");
    }
    mixer_ = resource_manager_->getMixer();
    music_track_ = MIX_CreateTrack(mixer_);
    if (!music_track_) {
        throw std::runtime_error("AudioPlayer 构造失败: 无法创建音乐轨道: " + std::string(SDL_GetError()));
    }
}

int AudioPlayer::playSound(entt::id_type sound_id) {

    MIX_Audio* audio = resource_manager_->getSound(sound_id); // 通过 ResourceManager 获取资源
    if (!audio) {
        spdlog::error("AudioPlayer: 无法获取音效 '{}' 播放。", sound_id);
        return -1;
    }

    if (!MIX_PlayAudio(mixer_, audio)) {    // 即发即忘方式播放音效
        spdlog::error("AudioPlayer: 无法播放音效 id: '{}': {}", sound_id, SDL_GetError());
        return -1;
    }
    spdlog::trace("AudioPlayer: 播放音效 id: '{}'。", sound_id);
    return 0;
}

int AudioPlayer::playSound(entt::hashed_string hashed_path) {
    MIX_Audio* audio = resource_manager_->getSound(hashed_path, hashed_path.data()); // 通过 ResourceManager 获取资源
    if (!audio) {
        spdlog::error("AudioPlayer: 无法获取音效 id: {}, path: {} 播放。", hashed_path.value(), hashed_path.data());
        return -1;
    }

    if (!MIX_PlayAudio(mixer_, audio)) {    // 即发即忘方式播放音效
        spdlog::error("AudioPlayer: 无法播放音效 id: {}, path: {}: {}", hashed_path.value(), hashed_path.data(), SDL_GetError());
        return -1;
    }
    spdlog::trace("AudioPlayer: 播放音效 id: {}, path: {}。", hashed_path.value(), hashed_path.data());
    return 0;
}

bool AudioPlayer::playMusic(entt::id_type music_id, int loops, int fade_in_ms) {
    if (music_id == current_music_id_) return true;      // 如果当前音乐已经在播放，则不重复播放
    current_music_id_ = music_id;
    MIX_Audio* music = resource_manager_->getMusic(music_id); // 通过 ResourceManager 获取资源
    if (!music) {
        spdlog::error("AudioPlayer: 无法获取音乐 '{}' 播放。", music_id);
        return false;
    }

    MIX_StopTrack(music_track_, 0);         // 立即停止之前的音乐
    MIX_SetTrackAudio(music_track_, music); // 设置音乐轨道的音频源

    // 配置播放参数（循环次数、淡入时长）
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
    if (fade_in_ms > 0) {
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fade_in_ms);
    }
    bool result = MIX_PlayTrack(music_track_, props);
    SDL_DestroyProperties(props);

    if (!result) {
        spdlog::error("AudioPlayer: 无法播放音乐 '{}': {}", music_id, SDL_GetError());
    } else {
        spdlog::trace("AudioPlayer: 播放音乐 '{}'。", music_id);
    }
    return result;
}

bool AudioPlayer::playMusic(entt::hashed_string hashed_path, int loops, int fade_in_ms) {
    if (hashed_path.value() == current_music_id_) return true;      // 如果当前音乐已经在播放，则不重复播放
    current_music_id_ = hashed_path;
    MIX_Audio* music = resource_manager_->getMusic(hashed_path, hashed_path.data()); // 通过 ResourceManager 获取资源
    if (!music) {
        spdlog::error("AudioPlayer: 无法获取音乐 id: {}, path: {} 播放。", hashed_path.value(), hashed_path.data());
        return false;
    }

    MIX_StopTrack(music_track_, 0);         // 立即停止之前的音乐
    MIX_SetTrackAudio(music_track_, music); // 设置音乐轨道的音频源

    // 配置播放参数（循环次数、淡入时长）
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
    if (fade_in_ms > 0) {
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fade_in_ms);
    }
    bool result = MIX_PlayTrack(music_track_, props);
    SDL_DestroyProperties(props);

    if (!result) {
        spdlog::error("AudioPlayer: 无法播放音乐 id: {}, path: {} 播放。error: {}", hashed_path.value(), hashed_path.data(), SDL_GetError());
    } else {
        spdlog::trace("AudioPlayer: 播放音乐 id: {}, path: {}。", hashed_path.value(), hashed_path.data());
    }
    return result;
}

void AudioPlayer::stopMusic(int fade_out_ms) {
    Sint64 fade_frames = (fade_out_ms > 0) ? MIX_TrackMSToFrames(music_track_, fade_out_ms) : 0;
    MIX_StopTrack(music_track_, fade_frames);
    spdlog::trace("AudioPlayer: 停止音乐。");
}

void AudioPlayer::pauseMusic() {
    MIX_PauseTrack(music_track_);
    spdlog::trace("AudioPlayer: 暂停音乐。");
}

void AudioPlayer::resumeMusic() {
    MIX_ResumeTrack(music_track_);
    spdlog::trace("AudioPlayer: 恢复音乐。");
}

void AudioPlayer::setSoundVolume(float volume) {
    // 通过混音器整体增益控制音效音量（0.0-1.0）
    MIX_SetMixerGain(mixer_, volume);
    spdlog::trace("AudioPlayer: 设置音效音量为 {:.2f}。", volume);
}

void AudioPlayer::setMusicVolume(float volume) {
    MIX_SetTrackGain(music_track_, volume);
    spdlog::trace("AudioPlayer: 设置音乐音量为 {:.2f}。", volume);
}

float AudioPlayer::getMusicVolume() {
    return MIX_GetTrackGain(music_track_);
}

float AudioPlayer::getSoundVolume() {
    return MIX_GetMixerGain(mixer_);
}

} // namespace engine::audio
