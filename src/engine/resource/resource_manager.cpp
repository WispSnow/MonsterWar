#include "resource_manager.h"
#include "texture_manager.h"
#include "audio_manager.h"
#include "font_manager.h" 
#include <fstream>
#include <filesystem>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h> 
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <entt/core/hashed_string.hpp>
 
namespace engine::resource {

ResourceManager::~ResourceManager() = default;

ResourceManager::ResourceManager(SDL_Renderer* renderer) {
    // --- 初始化各个子系统 --- (如果出现错误会抛出异常，由上层捕获)
    texture_manager_ = std::make_unique<TextureManager>(renderer);
    audio_manager_ = std::make_unique<AudioManager>();
    font_manager_ = std::make_unique<FontManager>();

    spdlog::trace("ResourceManager 构造成功。");
    // RAII: 构造成功即代表资源管理器可以正常工作，无需再初始化，无需检查指针是否为空
}

void ResourceManager::clear() {
    font_manager_->clearFonts();
    audio_manager_->clearSounds();
    texture_manager_->clearTextures();
    spdlog::trace("ResourceManager 中的资源通过 clear() 清空。");
}

void ResourceManager::loadResources(std::string_view file_path) {
    std::filesystem::path path(file_path);
    if (!std::filesystem::exists(path)) {
        spdlog::warn("资源映射文件不存在: {}", file_path);
        return;
    }
    std::ifstream file(path);
    nlohmann::json json;
    file >> json;
    try {
        if (json.contains("sound")) {
            for (const auto& [key, value] : json["sound"].items()) {
                loadSound(entt::hashed_string(key.c_str()), value.get<std::string>());
            }
        }
        if (json.contains("music")) {
            for (const auto& [key, value] : json["music"].items()) {
                loadMusic(entt::hashed_string(key.c_str()), value.get<std::string>());
            }
        }
        if (json.contains("texture")) {
            for (const auto& [key, value] : json["texture"].items()) {
                loadTexture(entt::hashed_string(key.c_str()), value.get<std::string>());
            }
        }
        if (json.contains("font")) {
            for (const auto& [key, value] : json["font"].items()) {
                loadFont(entt::hashed_string(key.c_str()), value.get<int>(), value.get<std::string>());
            }
        }   
    } catch (const nlohmann::json::exception& e) {
        spdlog::error("加载资源文件失败: {}", e.what());
    }
}

// --- 纹理接口实现 ---
SDL_Texture* ResourceManager::loadTexture(entt::id_type id, std::string_view file_path) {
    // 构造函数已经确保了 texture_manager_ 不为空，因此不需要再进行if检查，以免性能浪费
    return texture_manager_->loadTexture(id, file_path);
}

SDL_Texture* ResourceManager::loadTexture(entt::hashed_string str_hs) {
    return texture_manager_->loadTexture(str_hs);
}

SDL_Texture* ResourceManager::getTexture(entt::id_type id, std::string_view file_path) {
    return texture_manager_->getTexture(id, file_path);
}

SDL_Texture* ResourceManager::getTexture(entt::hashed_string str_hs) {
    return texture_manager_->getTexture(str_hs);
}

glm::vec2 ResourceManager::getTextureSize(entt::id_type id, std::string_view file_path) {
    return texture_manager_->getTextureSize(id, file_path);
}

glm::vec2 ResourceManager::getTextureSize(entt::hashed_string str_hs) {
    return texture_manager_->getTextureSize(str_hs);
}

void ResourceManager::unloadTexture(entt::id_type id) {
    texture_manager_->unloadTexture(id);
}

void ResourceManager::clearTextures() {
    texture_manager_->clearTextures();
}

// --- 音频接口实现 ---
Mix_Chunk* ResourceManager::loadSound(entt::id_type id, std::string_view file_path) {
    return audio_manager_->loadSound(id, file_path);
}

Mix_Chunk* ResourceManager::loadSound(entt::hashed_string str_hs) {
    return audio_manager_->loadSound(str_hs);
}

Mix_Chunk* ResourceManager::getSound(entt::id_type id, std::string_view file_path) {
    return audio_manager_->getSound(id, file_path);
}

Mix_Chunk* ResourceManager::getSound(entt::hashed_string str_hs) {
    return audio_manager_->getSound(str_hs);
}

void ResourceManager::unloadSound(entt::id_type id) {
    audio_manager_->unloadSound(id);
}

void ResourceManager::clearSounds() {
    audio_manager_->clearSounds();
}

Mix_Music* ResourceManager::loadMusic(entt::id_type id, std::string_view file_path) {
    return audio_manager_->loadMusic(id, file_path);
}

Mix_Music* ResourceManager::loadMusic(entt::hashed_string str_hs) {
    return audio_manager_->loadMusic(str_hs);
}

Mix_Music* ResourceManager::getMusic(entt::id_type id, std::string_view file_path) {
    return audio_manager_->getMusic(id, file_path);
}

Mix_Music* ResourceManager::getMusic(entt::hashed_string str_hs) {
    return audio_manager_->getMusic(str_hs);
}

void ResourceManager::unloadMusic(entt::id_type id) {
    audio_manager_->unloadMusic(id);
}

void ResourceManager::clearMusic() {
    audio_manager_->clearMusic();
}

// --- 字体接口实现 ---
TTF_Font* ResourceManager::loadFont(entt::id_type id, int point_size, std::string_view file_path) {
    return font_manager_->loadFont(id, point_size, file_path);
}

TTF_Font* ResourceManager::loadFont(entt::hashed_string str_hs, int point_size) {
    return font_manager_->loadFont(str_hs, point_size);
}

TTF_Font* ResourceManager::getFont(entt::id_type id, int point_size, std::string_view file_path) {
    return font_manager_->getFont(id, point_size, file_path);
}

TTF_Font* ResourceManager::getFont(entt::hashed_string str_hs, int point_size) {
    return font_manager_->getFont(str_hs, point_size);
}

void ResourceManager::unloadFont(entt::id_type id, int point_size) {
    font_manager_->unloadFont(id, point_size);
}

void ResourceManager::clearFonts() {
    font_manager_->clearFonts();
}

} // namespace engine::resource
