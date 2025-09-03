#include "audio_player.h"
#include "../resource/resource_manager.h"
#include <SDL3_mixer/SDL_mixer.h> 
#include <spdlog/spdlog.h>
#include <glm/common.hpp>
#include <entt/core/hashed_string.hpp>

namespace engine::audio {
AudioPlayer::~AudioPlayer() = default;

AudioPlayer::AudioPlayer(engine::resource::ResourceManager* resource_manager)
    : resource_manager_(resource_manager) {
    if (!resource_manager_) {
        throw std::runtime_error("AudioPlayer 构造失败: 提供的 ResourceManager 指针为空。");
    }
}

int AudioPlayer::playSound(entt::id_type sound_id, int channel) {

    Mix_Chunk* chunk = resource_manager_->getSound(sound_id); // 通过 ResourceManager 获取资源
    if (!chunk) {
        spdlog::error("AudioPlayer: 无法获取音效 '{}' 播放。", sound_id);
        return -1;
    }

    int played_channel = Mix_PlayChannel(channel, chunk, 0);    // 播放音效
    if (played_channel == -1) {
        spdlog::error("AudioPlayer: 无法播放音效 id: '{}': {}", sound_id, SDL_GetError());
    } else {
         spdlog::trace("AudioPlayer: 播放音效 id: '{}' 在通道 {}。", sound_id, played_channel);
    }
    return played_channel;
}

int AudioPlayer::playSound(entt::hashed_string hashed_path, int channel) {
    Mix_Chunk* chunk = resource_manager_->getSound(hashed_path, hashed_path.data()); // 通过 ResourceManager 获取资源
    if (!chunk) {
        spdlog::error("AudioPlayer: 无法获取音效 id: {}, path: {} 播放。", hashed_path.value(), hashed_path.data());
        return -1;
    }   

    int played_channel = Mix_PlayChannel(channel, chunk, 0);    // 播放音效
    if (played_channel == -1) {
        spdlog::error("AudioPlayer: 无法播放音效 id: {}, path: {}: {}", hashed_path.value(), hashed_path.data(), SDL_GetError());
    } else {
         spdlog::trace("AudioPlayer: 播放音效 id: {}, path: {} 在通道 {}。", hashed_path.value(), hashed_path.data(), played_channel);
    }
    return played_channel;
}

bool AudioPlayer::playMusic(entt::id_type music_id, int loops, int fade_in_ms) {
    if (music_id == current_music_id_) return true;      // 如果当前音乐已经在播放，则不重复播放
    current_music_id_ = music_id;
    Mix_Music* music = resource_manager_->getMusic(music_id); // 通过 ResourceManager 获取资源
    if (!music) {
        spdlog::error("AudioPlayer: 无法获取音乐 '{}' 播放。", music_id);
        return false;
    }

    Mix_HaltMusic();        // 停止之前的音乐

    bool result = false;
    if (fade_in_ms > 0) {
         result = Mix_FadeInMusic(music, loops, fade_in_ms);    // 淡入播放音乐
    } else {
         result = Mix_PlayMusic(music, loops);
    }

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
    Mix_Music* music = resource_manager_->getMusic(hashed_path, hashed_path.data()); // 通过 ResourceManager 获取资源
    if (!music) {
        spdlog::error("AudioPlayer: 无法获取音乐 id: {}, path: {} 播放。", hashed_path.value(), hashed_path.data());
        return false;
    }

    Mix_HaltMusic();        // 停止之前的音乐

    bool result = false;
    if (fade_in_ms > 0) {
         result = Mix_FadeInMusic(music, loops, fade_in_ms);    // 淡入播放音乐
    } else {
         result = Mix_PlayMusic(music, loops);
    }

    if (!result) {
        spdlog::error("AudioPlayer: 无法播放音乐 id: {}, path: {} 播放。error: {}", hashed_path.value(), hashed_path.data(), SDL_GetError());
    } else {
        spdlog::trace("AudioPlayer: 播放音乐 id: {}, path: {}。", hashed_path.value(), hashed_path.data());
    }
    return result;
}

void AudioPlayer::stopMusic(int fade_out_ms) {
    if (fade_out_ms > 0) {
        Mix_FadeOutMusic(fade_out_ms);  // 淡出音乐
    } else {
        Mix_HaltMusic();
    }
     spdlog::trace("AudioPlayer: 停止音乐。");
}

void AudioPlayer::pauseMusic() {
    Mix_PauseMusic();
    spdlog::trace("AudioPlayer: 暂停音乐。");
}

void AudioPlayer::resumeMusic() {
    Mix_ResumeMusic();
    spdlog::trace("AudioPlayer: 恢复音乐。");
}

void AudioPlayer::setSoundVolume(float volume, int channel) {
    // 将浮点音量(0-1)转换为SDL_mixer的音量(0-128)
    int sdl_volume = static_cast<int>(glm::max(0.0f, glm::min(1.0f, volume)) * MIX_MAX_VOLUME);
    Mix_Volume(channel, sdl_volume);
    spdlog::trace("AudioPlayer: 设置通道 {} 的音量为 {:.2f}。", channel, volume);
}

void AudioPlayer::setMusicVolume(float volume) {
    int sdl_volume = static_cast<int>(glm::max(0.0f, glm::min(1.0f, volume)) * MIX_MAX_VOLUME);
    Mix_VolumeMusic(sdl_volume);
    spdlog::trace("AudioPlayer: 设置音乐音量为 {:.2f}。", volume);
}

float AudioPlayer::getMusicVolume() {
    // SDL_mixer的音量(0-128)转为 0～1.0 的浮点数
    return static_cast<float>(Mix_VolumeMusic(-1)) / static_cast<float>(MIX_MAX_VOLUME);
                            /* 参数 -1 表示查询当前音量 */
}

float AudioPlayer::getSoundVolume(int channel) {
    return static_cast<float>(Mix_Volume(channel, -1)) / static_cast<float>(MIX_MAX_VOLUME);
}

} // namespace engine::audio
