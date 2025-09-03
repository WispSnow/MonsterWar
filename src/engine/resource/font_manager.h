#pragma once
#include <memory>
#include <unordered_map>
#include <utility>
#include <string_view>
#include <entt/core/fwd.hpp>
#include <SDL3_ttf/SDL_ttf.h>

namespace engine::resource {

// 定义字体键类型（路径 + 大小）
using FontKey = std::pair<entt::id_type, int>;

/**
 * @brief FontKey 的自定义哈希函数，适用于 std::unordered_map。
 *        使用标准库推荐的哈希合并方式，避免简单异或带来的哈希冲突。
 */
struct FontKeyHash {
    std::size_t operator()(const FontKey& key) const noexcept {
        // 采用C++20标准库的hash_combine实现思路
        std::size_t h1 = std::hash<entt::id_type>{}(key.first);
        std::size_t h2 = std::hash<int>{}(key.second);
        // 推荐的哈希合并方式，参考boost::hash_combine
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

/**
 * @brief 管理 SDL_ttf 字体资源（TTF_Font）。
 *
 * 提供字体的加载和缓存功能，通过文件路径和点大小来标识。
 * 构造失败会抛出异常。仅供 ResourceManager 内部使用。
 */
class FontManager final{
    friend class ResourceManager;

private:
    // TTF_Font 的自定义删除器
    struct SDLFontDeleter {
        void operator()(TTF_Font* font) const {
            if (font) {
                TTF_CloseFont(font);
            }
        }
    };

    // 字体存储（FontKey -> TTF_Font）。  
    // unordered_map 的键需要能转换为哈希值，对于基础数据类型，系统会自动转换
    // 但是对于对于自定义类型（系统无法自动转化），则需要提供自定义哈希函数（第三个模版参数）
    std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHash> fonts_;

public:
    /**
     * @brief 构造函数。初始化 SDL_ttf。
     * @throws std::runtime_error 如果 SDL_ttf 初始化失败。
     */
    FontManager();
    
    ~FontManager();            ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_ttf。

    // 当前设计中，我们只需要一个FontManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;

private: // 仅由 ResourceManager（和内部）访问的方法
    /**
     * @brief 从文件路径加载指定点大小的字体
     * @param id 字体的唯一标识符, 通过entt::hashed_string生成
     * @param point_size 字体的点大小
     * @param file_path 字体文件的路径
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，则从文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* loadFont(entt::id_type id, int point_size, std::string_view file_path);

    /**
     * @brief 从字符串哈希值加载指定点大小的字体
     * @param str_hs entt::hashed_string类型
     * @param point_size 字体的点大小
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，则从哈希字符串对应的文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* loadFont(entt::hashed_string str_hs, int point_size);

    /**
     * @brief 尝试获取已加载字体的指针，如果未加载则尝试加载
     * @param id 字体的唯一标识符, 通过entt::hashed_string生成
     * @param point_size 字体的点大小
     * @param file_path 字体文件的路径
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，且提供了file_path，则尝试从文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* getFont(entt::id_type id, int point_size, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取字体
     * @param str_hs entt::hashed_string类型
     * @param point_size 字体的点大小
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，则从哈希字符串对应的文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* getFont(entt::hashed_string str_hs, int point_size);

    /**
     * @brief 卸载特定字体（通过路径哈希值和大小标识）
     * @param id 字体的唯一标识符, 通过entt::hashed_string生成
     * @param point_size 字体的点大小
     */
    void unloadFont(entt::id_type id, int point_size);
    
    /**
     * @brief 清空所有缓存的字体
     */
    void clearFonts();
};

} // namespace engine::resource
