#include "audio_manager.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <entt/core/hashed_string.hpp>

namespace engine::resource {

// 构造函数：初始化SDL_mixer
AudioManager::AudioManager() {
    // 使用所需的格式初始化SDL_mixer（推荐OGG、MP3）
    MIX_InitFlags flags = MIX_INIT_OGG | MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        throw std::runtime_error("AudioManager 错误: Mix_Init 失败: " + std::string(SDL_GetError()));
    }

    // SDL3打开音频设备的方法。默认值：44100 Hz，默认格式，2声道（立体声），2048采样块大小
    if (!Mix_OpenAudio(0, nullptr)) {
        Mix_Quit(); // 如果OpenAudio失败，先清理Mix_Init，再抛出异常
        throw std::runtime_error("AudioManager 错误: Mix_OpenAudio 失败: " + std::string(SDL_GetError()));
    }
    spdlog::trace("AudioManager 构造成功。");
}

AudioManager::~AudioManager()
{
    // 立即停止所有音频播放
    Mix_HaltChannel(-1); // 停止所有音效
    Mix_HaltMusic();     // 停止音乐

    // 清理资源映射（unique_ptrs会调用删除器）
    clearSounds();
    clearMusic();

    // 关闭音频设备
    Mix_CloseAudio();

    // 退出SDL_mixer子系统
    Mix_Quit();
    spdlog::trace("AudioManager 析构成功。");
}

// --- 音效管理 ---
Mix_Chunk* AudioManager::loadSound(entt::id_type id, std::string_view file_path) {
    // 首先检查缓存
    auto it = sounds_.find(id);
    if (it != sounds_.end()) {
        return it->second.get();
    }

    // 加载音效块
    spdlog::debug("加载音效: {}", id);
    Mix_Chunk* raw_chunk = Mix_LoadWAV(file_path.data());
    if (!raw_chunk) {
        spdlog::error("加载音效失败: '{}': {}", id, SDL_GetError());
        return nullptr;
    }

    // 使用unique_ptr存储在缓存中
    sounds_.emplace(id, std::unique_ptr<Mix_Chunk, SDLMixChunkDeleter>(raw_chunk));
    spdlog::debug("成功加载并缓存音效: {}", id);
    return raw_chunk;
}

Mix_Chunk* AudioManager::loadSound(entt::hashed_string str_hs) {
    return loadSound(str_hs.value(), str_hs.data());
}

Mix_Chunk* AudioManager::getSound(entt::id_type id, std::string_view file_path) {
    auto it = sounds_.find(id);
    if (it != sounds_.end()) {
        return it->second.get();
    }
    // 如果未找到，判断是否提供了file_path
    if (file_path.empty()) {
        spdlog::error("音效 '{}' 未找到缓存，且未提供文件路径，返回nullptr。", id);
        return nullptr;
    }

    spdlog::warn("音效 '{}' 未找到缓存，尝试加载。", id);
    return loadSound(id, file_path);
}

Mix_Chunk* AudioManager::getSound(entt::hashed_string str_hs) {
    return getSound(str_hs.value(), str_hs.data());
}

void AudioManager::unloadSound(entt::id_type id) {
    auto it = sounds_.find(id);
    if (it != sounds_.end()) {
        spdlog::debug("卸载音效: {}", id);
        sounds_.erase(it);      // unique_ptr处理Mix_FreeChunk
    } else {
        spdlog::warn("尝试卸载不存在的音效: id = {}", id);
    }
}

void AudioManager::clearSounds() {
    if (!sounds_.empty()) {
        spdlog::debug("正在清除所有 {} 个缓存的音效。", sounds_.size());
        sounds_.clear(); // unique_ptr处理删除
    }
}

// --- 音乐管理 ---
Mix_Music* AudioManager::loadMusic(entt::id_type id, std::string_view file_path) {
    // 首先检查缓存
    auto it = music_.find(id);
    if (it != music_.end()) {
        return it->second.get();
    }

    // 加载音乐
    spdlog::debug("加载音乐: {}", id);
    Mix_Music* raw_music = Mix_LoadMUS(file_path.data());
    if (!raw_music) {
        spdlog::error("加载音乐失败: '{}': {}", id, SDL_GetError());
        return nullptr;
    }

    // 使用unique_ptr存储在缓存中
    music_.emplace(id, std::unique_ptr<Mix_Music, SDLMixMusicDeleter>(raw_music));
    spdlog::debug("成功加载并缓存音乐: {}", id);
    return raw_music;
}

Mix_Music* AudioManager::loadMusic(entt::hashed_string str_hs) {
    return loadMusic(str_hs.value(), str_hs.data());
}

Mix_Music* AudioManager::getMusic(entt::id_type id, std::string_view file_path) {
    auto it = music_.find(id);
    if (it != music_.end()) {
        return it->second.get();
    }
    // 如果未找到，判断是否提供了file_path
    if (file_path.empty()) {
        spdlog::error("音乐 '{}' 未找到缓存，且未提供文件路径，返回nullptr。", id);
        return nullptr;
    }

    spdlog::warn("音乐 '{}' 未找到缓存，尝试加载。", id);
    return loadMusic(id, file_path);
}

Mix_Music* AudioManager::getMusic(entt::hashed_string str_hs) {
    return getMusic(str_hs.value(), str_hs.data());
}

void AudioManager::unloadMusic(entt::id_type id) {
    auto it = music_.find(id);
    if (it != music_.end()) {
        spdlog::debug("卸载音乐: {}", id);
        music_.erase(it); // unique_ptr处理Mix_FreeMusic
    } else {
        spdlog::warn("尝试卸载不存在的音乐: id = {}", id);
    }
}

void AudioManager::clearMusic() {
    if (!music_.empty()) {
        spdlog::debug("正在清除所有 {} 个缓存的音乐曲目。", music_.size());
        music_.clear(); // unique_ptr处理删除
    }
}

void AudioManager::clearAudio()
{
    clearSounds();
    clearMusic();
}

} // namespace engine::resource
