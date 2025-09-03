#pragma once
#include <memory>
#include <unordered_map>
#include <string_view>
#include <entt/core/fwd.hpp>
#include <SDL3_mixer/SDL_mixer.h> // SDL_mixer 主头文件

namespace engine::resource {

/**
 * @brief 管理 SDL_mixer 音效 (Mix_Chunk) 和音乐 (Mix_Music)。
 *
 * 提供音频资源的加载和缓存功能。构造失败时会抛出异常。
 * 仅供 ResourceManager 内部使用。
 */
class AudioManager final{
    friend class ResourceManager;

private:
    // Mix_Chunk 的自定义删除器
    struct SDLMixChunkDeleter {
        void operator()(Mix_Chunk* chunk) const {
            if (chunk) {
                Mix_FreeChunk(chunk);
            }
        }
    };

    // Mix_Music 的自定义删除器
    struct SDLMixMusicDeleter {
        void operator()(Mix_Music* music) const {
            if (music) {
                Mix_FreeMusic(music);
            }
        }
    };

    // 音效存储 (文件路径 -> Mix_Chunk)
    std::unordered_map<entt::id_type, std::unique_ptr<Mix_Chunk, SDLMixChunkDeleter>> sounds_;
    // 音乐存储 (文件路径 -> Mix_Music)
    std::unordered_map<entt::id_type, std::unique_ptr<Mix_Music, SDLMixMusicDeleter>> music_;

public:
    /**
     * @brief 构造函数。初始化 SDL_mixer 并打开音频设备。
     * @throws std::runtime_error 如果 SDL_mixer 初始化或打开音频设备失败。
     */
    AudioManager();

    ~AudioManager();            ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_mixer。

    // 当前设计中，我们只需要一个AudioManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = delete;
    AudioManager& operator=(AudioManager&&) = delete;

private:  // 仅供 ResourceManager 访问的方法
    /**
     * @brief 从文件路径加载音效
     * @param id 音效的唯一标识符, 通过entt::hashed_string生成
     * @param file_path 音效文件的路径
     * @return 加载的音效的指针
     * @note 如果音效已经加载，则返回已加载音效的指针
     * @note 如果音效未加载，则从文件路径加载音效，并返回加载的音效的指针
     */
    Mix_Chunk* loadSound(entt::id_type id, std::string_view file_path);

    /**
     * @brief 从字符串哈希值加载音效
     * @param str_hs entt::hashed_string类型
     * @return 加载的音效的指针
     * @note 如果音效已经加载，则返回已加载音效的指针
     * @note 如果音效未加载，则从哈希字符串对应的文件路径加载音效，并返回加载的音效的指针
     */
    Mix_Chunk* loadSound(entt::hashed_string str_hs);

    /**
     * @brief 从文件路径获取音效
     * @param id 音效的唯一标识符, 通过entt::hashed_string生成
     * @return 加载的音效的指针
     * @note 如果音效已经加载，则返回已加载音效的指针
     * @note 如果音效未加载，则从哈希字符串对应的文件路径加载音效，并返回加载的音效的指针
     */
    Mix_Chunk* getSound(entt::id_type id, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取音效
     * @param str_hs entt::hashed_string类型
     * @return 加载的音效的指针
     * @note 如果音效已经加载，则返回已加载音效的指针
     * @note 如果音效未加载，则从哈希字符串对应的文件路径加载音效，并返回加载的音效的指针
     */
    Mix_Chunk* getSound(entt::hashed_string str_hs);

    /**
     * @brief 卸载指定的音效资源
     * @param id 音效的唯一标识符, 通过entt::hashed_string生成
     */
    void unloadSound(entt::id_type id);

    /**
     * @brief 清空所有音效资源
     */
    void clearSounds();

    /**
     * @brief 从文件路径加载音乐
     * @param id 音乐的唯一标识符, 通过entt::hashed_string生成
     * @param file_path 音乐文件的路径
     * @return 加载的音乐的指针
     * @note 如果音乐已经加载，则返回已加载音乐的指针
     * @note 如果音乐未加载，则从文件路径加载音乐，并返回加载的音乐的指针
     */
    Mix_Music* loadMusic(entt::id_type id, std::string_view file_path);

    /**
     * @brief 从字符串哈希值加载音乐
     * @param str_hs entt::hashed_string类型
     * @return 加载的音乐的指针
     * @note 如果音乐已经加载，则返回已加载音乐的指针
     * @note 如果音乐未加载，则从哈希字符串对应的文件路径加载音乐，并返回加载的音乐的指针
     */
    Mix_Music* loadMusic(entt::hashed_string str_hs);

    /**
     * @brief 从文件路径获取音乐
     * @param id 音乐的唯一标识符, 通过entt::hashed_string生成
     * @return 加载的音乐的指针
     * @note 如果音乐已经加载，则返回已加载音乐的指针
     * @note 如果音乐未加载，则从哈希字符串对应的文件路径加载音乐，并返回加载的音乐的指针
     */
    Mix_Music* getMusic(entt::id_type id, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取音乐
     * @param str_hs entt::hashed_string类型
     * @return 加载的音乐的指针
     * @note 如果音乐已经加载，则返回已加载音乐的指针
     * @note 如果音乐未加载，则从哈希字符串对应的文件路径加载音乐，并返回加载的音乐的指针
     */
    Mix_Music* getMusic(entt::hashed_string str_hs);

    /**
     * @brief 卸载指定的音乐资源
     * @param id 音乐的唯一标识符, 通过entt::hashed_string生成
     */
    void unloadMusic(entt::id_type id);

    /**
     * @brief 清空所有音乐资源
     */
    void clearMusic();

    /**
     * @brief 清空所有音频资源
     */
    void clearAudio();
};

} // namespace engine::resource
