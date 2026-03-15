#pragma once
#include <memory>
#include <unordered_map>
#include <string_view>
#include <entt/core/fwd.hpp>
#include <SDL3_mixer/SDL_mixer.h> // SDL_mixer 主头文件

namespace engine::resource {

/**
 * @brief 管理 SDL_mixer 音效和音乐 (统一为 MIX_Audio 类型)。
 *
 * 提供音频资源的加载和缓存功能。构造失败时会抛出异常。
 * 仅供 ResourceManager 内部使用。
 */
class AudioManager final{
    friend class ResourceManager;

private:
    // MIX_Audio 的自定义删除器（音效和音乐统一类型）
    struct MIXAudioDeleter {
        void operator()(MIX_Audio* audio) const {
            if (audio) {
                MIX_DestroyAudio(audio);
            }
        }
    };

    MIX_Mixer* mixer_{nullptr};  ///< @brief SDL_mixer 混音器实例

    // 音效存储 (id -> MIX_Audio，预解码)
    std::unordered_map<entt::id_type, std::unique_ptr<MIX_Audio, MIXAudioDeleter>> sounds_;
    // 音乐存储 (id -> MIX_Audio，流式解码)
    std::unordered_map<entt::id_type, std::unique_ptr<MIX_Audio, MIXAudioDeleter>> music_;

public:
    /**
     * @brief 构造函数。初始化 SDL_mixer 并创建音频设备混音器。
     * @throws std::runtime_error 如果 SDL_mixer 初始化或创建混音器失败。
     */
    AudioManager();

    ~AudioManager();            ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_mixer。

    // 当前设计中，我们只需要一个AudioManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = delete;
    AudioManager& operator=(AudioManager&&) = delete;

    MIX_Mixer* getMixer() const { return mixer_; }  ///< @brief 获取混音器指针

private:  // 仅供 ResourceManager 访问的方法
    /**
     * @brief 从文件路径加载音效（预解码）
     * @param id 音效的唯一标识符, 通过entt::hashed_string生成
     * @param file_path 音效文件的路径
     * @return 加载的音效的指针
     */
    MIX_Audio* loadSound(entt::id_type id, std::string_view file_path);

    /**
     * @brief 从字符串哈希值加载音效（预解码）
     * @param str_hs entt::hashed_string类型
     * @return 加载的音效的指针
     */
    MIX_Audio* loadSound(entt::hashed_string str_hs);

    /**
     * @brief 获取音效，如果未缓存且提供了路径则尝试加载
     * @param id 音效的唯一标识符
     * @param file_path 音效文件路径（可选）
     * @return 加载的音效的指针
     */
    MIX_Audio* getSound(entt::id_type id, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取音效
     * @param str_hs entt::hashed_string类型
     * @return 加载的音效的指针
     */
    MIX_Audio* getSound(entt::hashed_string str_hs);

    void unloadSound(entt::id_type id);    ///< @brief 卸载指定的音效资源
    void clearSounds();                     ///< @brief 清空所有音效资源

    /**
     * @brief 从文件路径加载音乐（流式解码）
     * @param id 音乐的唯一标识符
     * @param file_path 音乐文件的路径
     * @return 加载的音乐的指针
     */
    MIX_Audio* loadMusic(entt::id_type id, std::string_view file_path);

    /**
     * @brief 从字符串哈希值加载音乐（流式解码）
     * @param str_hs entt::hashed_string类型
     * @return 加载的音乐的指针
     */
    MIX_Audio* loadMusic(entt::hashed_string str_hs);

    /**
     * @brief 获取音乐，如果未缓存且提供了路径则尝试加载
     * @param id 音乐的唯一标识符
     * @param file_path 音乐文件路径（可选）
     * @return 加载的音乐的指针
     */
    MIX_Audio* getMusic(entt::id_type id, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取音乐
     * @param str_hs entt::hashed_string类型
     * @return 加载的音乐的指针
     */
    MIX_Audio* getMusic(entt::hashed_string str_hs);

    void unloadMusic(entt::id_type id);    ///< @brief 卸载指定的音乐资源
    void clearMusic();                      ///< @brief 清空所有音乐资源
    void clearAudio();                      ///< @brief 清空所有音频资源
};

} // namespace engine::resource
