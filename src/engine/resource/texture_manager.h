#pragma once
#include <memory>
#include <unordered_map>
#include <string_view>
#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>
#include <entt/core/fwd.hpp>

namespace engine::resource {

/**
 * @brief 管理 SDL_Texture 资源的加载、存储和检索。
 *
 * 在构造时初始化。使用文件路径作为键，确保纹理只加载一次并正确释放。
 * 依赖于一个有效的 SDL_Renderer，构造失败会抛出异常。
 */
class TextureManager final{
    friend class ResourceManager;

private:
    // SDL_Texture 的删除器函数对象，用于智能指针管理
    struct SDLTextureDeleter {
        void operator()(SDL_Texture* texture) const {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
        }
    };

    // 存储文件路径和指向管理纹理的 unique_ptr 的映射。(容器的键不可使用entt::hashed_string)
    std::unordered_map<entt::id_type, std::unique_ptr<SDL_Texture, SDLTextureDeleter>> textures_;

    SDL_Renderer* renderer_ = nullptr; // 指向主渲染器的非拥有指针

public:
    /**
     * @brief 构造函数，执行初始化。
     * @param renderer 指向有效的 SDL_Renderer 上下文的指针。不能为空。
     * @throws std::runtime_error 如果 renderer 为 nullptr 或初始化失败。
     */
    explicit TextureManager(SDL_Renderer* renderer);

    // 当前设计中，我们只需要一个TextureManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

private: // 仅供 ResourceManager 访问的方法

    /**
     * @brief 从文件路径加载纹理
     * @param id 纹理的唯一标识符, 通过entt::hashed_string生成
     * @param file_path 纹理文件的路径
     * @return 加载的纹理的指针
     * @note 如果纹理已经加载，则返回已加载的纹理的指针
     * @note 如果纹理未加载，则从文件路径加载纹理，并返回加载的纹理的指针
     */
    SDL_Texture* loadTexture(entt::id_type id, std::string_view file_path);
    
    /**
     * @brief 从字符串哈希值加载纹理
     * @param str_hs entt::hashed_string类型
     * @return 加载的纹理的指针
     * @note 如果纹理已经加载，则返回已加载的纹理的指针
     * @note 如果纹理未加载，则从字符串对应的文件路径加载纹理，并返回加载的纹理的指针
     */
    SDL_Texture* loadTexture(entt::hashed_string str_hs);
    
    /**
     * @brief 获取纹理
     * @param id 纹理的唯一标识符, 通过entt::hashed_string生成
     * @param file_path 纹理文件的路径
     * @return 加载的纹理的指针
     * @note 如果纹理已经加载，则返回已加载的纹理的指针
     * @note 如果纹理未加载，且提供了file_path，则尝试从文件路径加载纹理，并返回加载的纹理的指针
     * @note 如果纹理未加载，且没有提供file_path，则返回nullptr
     */
    SDL_Texture* getTexture(entt::id_type id, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取纹理
     * @param str_hs entt::hashed_string类型
     * @return 加载的纹理的指针
     * @note 如果纹理已经加载，则返回已加载的纹理的指针
     * @note 如果纹理未加载，则返回nullptr
     */
    SDL_Texture* getTexture(entt::hashed_string str_hs);

    /**
     * @brief 获取纹理的尺寸
     * @param id 纹理的唯一标识符, 通过entt::hashed_string生成
     * @param file_path 纹理文件的路径
     * @return 纹理的尺寸
     * @note 如果纹理未加载，且提供了file_path，则尝试从文件路径加载纹理，并返回加载的纹理的尺寸
     */
    glm::vec2 getTextureSize(entt::id_type id, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取纹理的尺寸
     * @param str_hs entt::hashed_string类型
     * @return 纹理的尺寸
     * @note 如果纹理未加载，则返回glm::vec2(0.0f, 0.0f)
     */
    glm::vec2 getTextureSize(entt::hashed_string str_hs);

    /**
     * @brief 卸载纹理
     * @param id 纹理的唯一标识符, 通过entt::hashed_string生成
     */
    void unloadTexture(entt::id_type id);

    /**
     * @brief 清空所有纹理资源
     */
    void clearTextures();
};

} // namespace engine::resource
